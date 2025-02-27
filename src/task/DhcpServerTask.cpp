#include "DhcpServerTask.hpp"

#include "debug/Print.hpp"
#include "network/NetworkGroup.hpp"
#include "network/dhcp/DhcpCommon.hpp"
#include "network/dhcp/DhcpLease.hpp"
#include "network/dhcp/DhcpMessage.hpp"
#include <lwip/api.h>
#include <lwip/err.h>
#include <lwip/ip4_addr.h>
#include <lwip/netbuf.h>
#include <lwip/pbuf.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <pico.h>
#include <string>
#include <utility>
#include <vector>

using namespace Network::Dhcp;

namespace Task
{

bool ipAvailable(const std::array<uint8_t, MAC_LEN> &leaseMac,
                 const std::array<uint8_t, MAC_LEN> &requestMac);
uint8_t getMessageType(DhcpMessage &message);
uint8_t getRequestedIp(DhcpMessage &message);

DhcpServerTask::DhcpServerTask(const std::shared_ptr<std::string> serverIp,
                               const std::shared_ptr<std::string> netmask,
                               std::shared_ptr<Network::NetworkGroup> networkGroup) :
    BaseTask{"DhcpServerTask", 512, this, MED},
    m_NetworkGroup{std::move(networkGroup)}
{
    ip4addr_aton(serverIp->c_str(), ip_2_ip4(&m_ServerIp));
    ip4addr_aton(netmask->c_str(), ip_2_ip4(&m_Netmask));
}

void DhcpServerTask::run()
{
    constexpr uint16_t DHCP_SERVER_PORT = 67;
    constexpr uint16_t DHCP_CLIENT_PORT = 68;
    constexpr uint16_t DHCP_MIN_SIZE = 240 + 3;
    constexpr uint8_t DHCP_MIN_IP = 50;
    constexpr uint8_t DHCP_MAX_IP = DHCP_MIN_IP + 100;
    constexpr uint32_t DHCP_LEASE_TIME = 60 * 60; // 1 hour lease time

    DhcpMessage message;
    uint8_t messageType = 0;
    uint8_t requestIp = 0;

    err_t err = ERR_OK;
    netbuf *receiveBuffer;

    // We wait until AP task is ready before starting DHCP operations
    m_NetworkGroup->wait(Network::AP);

    m_Connection = netconn_new(NETCONN_UDP);

    if (m_Connection != nullptr)
    {
        netconn_bind(m_Connection, IP_ANY_TYPE, DHCP_SERVER_PORT);
    }

    Debug::printInfo("DHCP", "Listening on port %u", DHCP_SERVER_PORT);
    m_NetworkGroup->set(Network::DHCP);

    while (true)
    {
        err = netconn_recv(m_Connection, &receiveBuffer);

        if (err == ERR_OK)
        {
            if (receiveBuffer->p->tot_len < DHCP_MIN_SIZE) { err = ERR_VAL; }
            else
            {
                message = DhcpMessage(receiveBuffer);
                messageType = getMessageType(message);

                if (messageType == 0) { err = ERR_VAL; }
                else
                {
                    requestIp = getRequestedIp(message);
                    requestIp = requestIp >= DHCP_MIN_IP ? requestIp : 0;
                    requestIp = requestIp <= DHCP_MAX_IP ? requestIp : 0;
                }
            }

            netbuf_free(receiveBuffer);
            netbuf_delete(receiveBuffer);
        }

        if (err == ERR_OK)
        {
            switch (messageType)
            {
                case DHCPDISCOVER:
                    if (requestIp == 0
                        || (m_Leases.count(requestIp) == 1
                            && !ipAvailable(m_Leases.at(requestIp).mac(),
                                            message.getClientMac())))
                    {
                        requestIp = 0;

                        for (uint8_t ip = DHCP_MIN_IP;
                             ip < DHCP_MAX_IP && requestIp == 0;
                             ++ip)
                        {
                            if (m_Leases.count(ip) == 0)
                            {
                                // Lease available
                                requestIp = ip;
                            }
                            else if (m_Leases.at(ip).hasExpired())
                            {
                                // IP expired
                                m_Leases.erase(ip);
                                requestIp = ip;
                            }
                        }

                        if (requestIp == 0)
                        {
                            // No IPs available
                            err = ERR_MEM;
                        }
                    }

                    message.setAsResponse();

                    if (err == ERR_OK)
                    {
                        message.setIpAddresses(&m_ServerIp, requestIp);
                        message.writeOption(DHCP_OPT_MSG_TYPE, DHCPOFFER);
                    }
                    break;

                case DHCPREQUEST:
                    if (requestIp == 0) { err = ERR_ARG; }

                    if (err == ERR_OK
                        && (m_Leases.count(requestIp) == 0
                            || ipAvailable(m_Leases.at(requestIp).mac(),
                                           message.getClientMac())))
                    {
                        // IP available, accept DHCP request
                        m_Leases.insert_or_assign(requestIp,
                                                  DhcpLease(message.getClientMac(),
                                                            DHCP_LEASE_TIME));
                    }
                    else { err = ERR_MEM; }

                    message.setAsResponse();

                    if (err == ERR_OK)
                    {
                        message.setIpAddresses(&m_ServerIp, requestIp);
                        message.writeOption(DHCP_OPT_MSG_TYPE, DHCPACK);
                        auto mac = message.getClientMac();

                        Debug::printInfo("DHCP",
                                         "Client connected: "
                                         "MAC=%X:%X:%X:%X:%X:%X, "
                                         "IP=%u.%u.%u.%u",
                                         mac.at(0),
                                         mac.at(1),
                                         mac.at(2),
                                         mac.at(3),
                                         mac.at(4),
                                         mac.at(5),
                                         ip4_addr_get_byte(&m_ServerIp, 0),
                                         ip4_addr_get_byte(&m_ServerIp, 1),
                                         ip4_addr_get_byte(&m_ServerIp, 2),
                                         requestIp);
                    }
                    break;

                default:
                    err = ERR_ARG;
                    break;
            }

            if (err == ERR_OK)
            {
                message.writeOption(DHCP_OPT_SERVER_ID, ip4_addr_get_u32(&m_ServerIp));
                message.writeOption(DHCP_OPT_SUBNET_MASK, ip4_addr_get_u32(&m_Netmask));
                message.writeOption(DHCP_OPT_ROUTER, ip4_addr_get_u32(&m_ServerIp));
                message.writeOption(DHCP_OPT_DNS, ip4_addr_get_u32(&m_ServerIp));
                message.writeOption(DHCP_OPT_IP_LEASE_TIME, DHCP_LEASE_TIME);
            }
            else
            {
                Debug::printError("DHCP", "Invalid DHCP message.", err);
                message.writeOption(DHCP_OPT_MSG_TYPE, DHCPNAK);
            }

            auto buf = message.serialize();
            netbuf *sendBuffer = netbuf_new();

            netbuf_ref(sendBuffer, buf.data(), buf.size());
            netconn_sendto(m_Connection, sendBuffer, IP4_ADDR_BROADCAST, DHCP_CLIENT_PORT);
            netbuf_delete(sendBuffer);
        }
    }
}

bool ipAvailable(const std::array<uint8_t, MAC_LEN> &leaseMac,
                 const std::array<uint8_t, MAC_LEN> &requestMac)
{
    std::array<uint8_t, MAC_LEN> empty{};
    bool available = std::equal(leaseMac.cbegin(), leaseMac.cend(), empty.cbegin());
    if (!available)
    {
        available = std::equal(leaseMac.cbegin(), leaseMac.cend(), requestMac.cbegin());
    }
    return available;
}

uint8_t getMessageType(DhcpMessage &message)
{
    uint8_t messageType = 0;
    auto optionVec = message.getOption(DHCP_OPT_MSG_TYPE);

    if (!optionVec.empty()) { messageType = optionVec.at(0); }

    return messageType;
}

uint8_t getRequestedIp(DhcpMessage &message)
{
    uint8_t ip = 0;

    auto optionVec = message.getOption(DHCP_OPT_REQUESTED_IP);

    if (optionVec.size() == 4) { ip = optionVec.at(3); }

    return ip;
}

} // namespace Task