#include "DhcpServer.hpp"

#include "cyw43_config.h"
#include "network/dhcp/DhcpCommon.hpp"
#include "network/udp/UdpCallback.hpp"
#include <lwip/err.h>
#include <lwip/ip4_addr.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>

namespace Network::Dhcp
{

bool ipAvailable(const std::array<uint8_t, MAC_LEN> &mac,
                 const std::array<uint8_t, MAC_LEN> &chaddr);
static void writeOption(uint8_t **options, uint8_t cmd, uint8_t val);
static void writeOption(uint8_t **options, uint8_t cmd, uint32_t val);
static void writeOption(uint8_t **options, uint8_t cmd, size_t n, const void *data);
static uint8_t *findOption(uint8_t *options, uint8_t cmd);

DhcpServer::DhcpServer(const std::shared_ptr<std::string> serverIp, uint8_t leaseMax) :
    m_LeaseMax{leaseMax}
{
    constexpr uint16_t DHCP_SERVER_PORT = 67;
    constexpr uint8_t DHCPS_LEASE_LIMIT = 254 - DHCPS_BASE_IP;

    m_LeaseMax = m_LeaseMax > DHCPS_LEASE_LIMIT ? DHCPS_LEASE_LIMIT : m_LeaseMax;

    ip4addr_aton(serverIp->c_str(), ip_2_ip4(&m_Ip));
    IP4_ADDR(ip_2_ip4(&m_Netmask), 255, 255, 255, 0);

    m_Leases.resize(m_LeaseMax);

    m_UdpPcb = udp_new();
    udp_recv(m_UdpPcb, Udp::udpReceive, this);
    udp_bind(m_UdpPcb, IP_ANY_TYPE, DHCP_SERVER_PORT);
}

DhcpServer::~DhcpServer()
{
    if (m_UdpPcb != nullptr)
    {
        udp_remove(m_UdpPcb);
        m_UdpPcb = nullptr;
    }
}

int DhcpServer::process(pbuf *p, const ip_addr_t *src_addr, u16_t src_port, netif *nif)
{
    constexpr uint16_t DHCP_CLIENT_PORT = 68;
    constexpr uint8_t DHCP_RESPONSE = 2;
    constexpr uint32_t DEFAULT_LEASE_TIME_S = 60 * 60; // 1 hour lease time
    constexpr uint16_t DHCP_MIN_SIZE = 240 + 3;

    (void)src_addr;
    (void)src_port;

    const auto serverIp = &ip4_addr_get_u32(&m_Ip);
    const auto netmask = &ip4_addr_get_u32(&m_Netmask);

    std::array<uint8_t, MAC_LEN> chaddr;
    uint8_t *options = nullptr;
    const uint8_t *msgtype = nullptr;
    const uint8_t *optRequestIp = nullptr;

    DhcpPayload payload;
    int err = ERR_OK;
    uint8_t yi = 0;
    std::array<uint8_t, MAC_LEN> emptyLease{0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (p->tot_len < DHCP_MIN_SIZE) { return -1; }

    size_t len = pbuf_copy_partial(p, &payload, sizeof(payload), 0);

    if (len < DHCP_MIN_SIZE) { return -1; }

    std::copy_n(std::cbegin(payload.chaddr), MAC_LEN, chaddr.begin());

    payload.op = DHCP_RESPONSE;
    std::memcpy(&payload.yiaddr, serverIp, 4);
    std::memcpy(&payload.siaddr, serverIp, 4);
    options = (uint8_t *)&payload.options;
    options += 4; // assume magic cookie: 99, 130, 83, 99
    msgtype = findOption(options, DHCP_OPT_MSG_TYPE);

    if (msgtype == nullptr)
    {
        printf("DHCPS: Invalid or missing DHCP message type.");
        return -1;
    }

    optRequestIp = findOption(options, DHCP_OPT_REQUESTED_IP);

    if (optRequestIp != nullptr)
    {
        yi = optRequestIp[5] - DHCPS_BASE_IP < 0 ? 0 : optRequestIp[5] - DHCPS_BASE_IP;
        yi = yi > m_LeaseMax ? 0 : yi;
    }

    switch (msgtype[2])
    {
        case DHCPDISCOVER:
            if (!ipAvailable(m_Leases.at(yi).mac, chaddr)
                && !ipAvailable(m_Leases.at(yi).mac, emptyLease))
            {
                bool validLease = false;

                for (auto it = m_Leases.begin(); it != m_Leases.cend() && !validLease;
                     ++it)
                {
                    if (ipAvailable(it->mac, chaddr)) { validLease = true; }
                    else if ((int32_t)((it->expiry << 16 | 0xFFFF) - cyw43_hal_ticks_ms())
                             < 0)
                    {
                        // IP expired
                        it->mac.fill(0);
                        validLease = true;
                    }
                    if (!validLease) { ++yi; }
                }

                if (!validLease)
                {
                    // No IPs available
                    err = -1;
                }
            }

            if (err == ERR_OK)
            {
                payload.yiaddr[3] = DHCPS_BASE_IP + yi;
                writeOption(&options, DHCP_OPT_MSG_TYPE, DHCPOFFER);
            }
            break;

        case DHCPREQUEST:
            if (optRequestIp == nullptr
                || std::memcmp(optRequestIp + 2, serverIp, 3) != 0)
            {
                err = -1;
            }

            yi = optRequestIp[5] - DHCPS_BASE_IP;

            if (yi >= m_LeaseMax && yi > 0) { err = -1; }

            if (err == ERR_OK && ipAvailable(m_Leases.at(yi).mac, chaddr))
            {
                // MAC match, accept DHCP request
            }
            else if (err == ERR_OK && ipAvailable(m_Leases.at(yi).mac, emptyLease))
            {
                // IP available, accept DHCP request
                m_Leases.at(yi).mac = chaddr;
            }
            else { err = -1; }

            if (err == ERR_OK && payload.yiaddr[3] != 0)
            {
                m_Leases.at(yi).expiry = (cyw43_hal_ticks_ms()
                                          + DEFAULT_LEASE_TIME_S * 1000)
                                         >> 16;
                payload.yiaddr[3] = DHCPS_BASE_IP + yi;

                writeOption(&options, DHCP_OPT_MSG_TYPE, DHCPACK);

                printf("DHCPS: client connected: "
                       "MAC=%02x:%02x:%02x:%02x:%02x:%02x "
                       "IP=%u.%u.%u.%u\n",
                       payload.chaddr[0],
                       payload.chaddr[1],
                       payload.chaddr[2],
                       payload.chaddr[3],
                       payload.chaddr[4],
                       payload.chaddr[5],
                       payload.yiaddr[0],
                       payload.yiaddr[1],
                       payload.yiaddr[2],
                       payload.yiaddr[3]);
            }
            break;

        default:
            err = -1;
            break;
    }

    if (err == ERR_OK)
    {
        writeOption(&options, DHCP_OPT_SERVER_ID, sizeof(uint32_t), serverIp);
        writeOption(&options, DHCP_OPT_SUBNET_MASK, sizeof(uint32_t), netmask);
        writeOption(&options, DHCP_OPT_ROUTER, sizeof(uint32_t), serverIp);
        writeOption(&options, DHCP_OPT_DNS, sizeof(uint32_t), serverIp);
        writeOption(&options, DHCP_OPT_IP_LEASE_TIME, DEFAULT_LEASE_TIME_S);
    }
    else { writeOption(&options, DHCP_OPT_MSG_TYPE, DHCPNAK); }

    *options++ = DHCP_OPT_END;

    send(nif, &payload, options - (uint8_t *)&payload, IPADDR_BROADCAST, DHCP_CLIENT_PORT);

    pbuf_free(p);

    return err;
}

int DhcpServer::send(netif *nif, const void *buf, size_t len, uint32_t ip, uint16_t port)
{
    ip_addr_t dest;
    err_t err;
    pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

    if (len > 0xffff) { len = 0xffff; }

    if (p == nullptr) { return -ENOMEM; }

    std::memcpy(p->payload, buf, len);

    IP4_ADDR(ip_2_ip4(&dest), ip >> 24 & 0xff, ip >> 16 & 0xff, ip >> 8 & 0xff, ip & 0xff);

    if (nif != nullptr) { err = udp_sendto_if(m_UdpPcb, p, &dest, port, nif); }
    else { err = udp_sendto(m_UdpPcb, p, &dest, port); }

    pbuf_free(p);

    if (err != ERR_OK) { return err; }

    return len;
}

bool ipAvailable(const std::array<uint8_t, MAC_LEN> &mac,
                 const std::array<uint8_t, MAC_LEN> &chaddr)
{
    return std::equal(mac.cbegin(), mac.cend(), chaddr.cbegin());
}

static void writeOption(uint8_t **options, uint8_t cmd, uint8_t val)
{
    uint8_t *o = *options;
    *o++ = cmd;
    *o++ = sizeof(val);
    *o++ = val;
    *options = o;
}

static void writeOption(uint8_t **options, uint8_t cmd, uint32_t val)
{
    uint8_t *o = *options;
    *o++ = cmd;
    *o++ = sizeof(val);
    *o++ = val >> 24;
    *o++ = val >> 16;
    *o++ = val >> 8;
    *o++ = val;
    *options = o;
}

static void writeOption(uint8_t **options, uint8_t cmd, size_t n, const void *data)
{
    uint8_t *o = *options;
    *o++ = cmd;
    *o++ = n;
    std::memcpy(o, data, n);
    *options = o + n;
}

static uint8_t *findOption(uint8_t *options, uint8_t cmd)
{
    constexpr uint16_t OPTION_MAX = 312 - 4; // Option field size (312 bytes) - magic cookie (4 bytes)

    for (uint16_t i = 0; i < OPTION_MAX && options[i] != DHCP_OPT_END;)
    {
        if (options[i] == cmd) { return &options[i]; }
        i += 2 + options[i + 1];
    }
    return nullptr;
}

} // namespace Network::Dhcp