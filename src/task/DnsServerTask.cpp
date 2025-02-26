#include "DnsServerTask.hpp"

#include "debug/Print.hpp"
#include "network/NetworkGroup.hpp"
#include "network/dns/DnsMessage.hpp"
#include <lwip/api.h>
#include <lwip/err.h>
#include <lwip/ip4_addr.h>
#include <lwip/netbuf.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <utility>
#include <vector>

using namespace Network::Dns;

namespace Task
{

uint8_t getQuestionLength(DnsMessage &message);

DnsServerTask::DnsServerTask(const std::shared_ptr<std::string> serverIp,
                             std::shared_ptr<Network::NetworkGroup> networkGroup) :
    BaseTask{"DnsServerTask", 512, this, MED},
    m_NetworkGroup{std::move(networkGroup)}
{
    ip4addr_aton(serverIp->c_str(), ip_2_ip4(&m_ServerIp));
}

void DnsServerTask::run()
{
    constexpr uint16_t DNS_SERVER_PORT = 53;
    constexpr uint16_t DNS_MESSAGE_LENGTH_MAX = 512; // Max DNS message length

    err_t err = ERR_OK;
    DnsMessage message;
    netbuf *receiveBuffer;
    ip_addr_t clientIp;
    uint16_t clientPort;

    // We wait until AP and DHCP tasks are ready before starting DNS operations
    m_NetworkGroup->wait(Network::AP_DHCP);

    m_Connection = netconn_new(NETCONN_UDP);

    if (m_Connection != nullptr)
    {
        netconn_bind(m_Connection, IP_ANY_TYPE, DNS_SERVER_PORT);
    }

    Debug::printInfo("DNS", "Listening on port %u", DNS_SERVER_PORT);
    m_NetworkGroup->set(Network::DNS);

    while (true)
    {
        err = netconn_recv(m_Connection, &receiveBuffer);

        if (err == ERR_OK)
        {
            if (receiveBuffer->p->tot_len <= DNS_MESSAGE_LENGTH_MAX)
            {
                message = DnsMessage{receiveBuffer};
                clientIp = receiveBuffer->addr;
                clientPort = receiveBuffer->port;
            }
            else { err = ERR_VAL; }

            if (!message.isStandardQuery()) { err = ERR_VAL; }
            if (message.getQCount() < 1) { err = ERR_VAL; }

            netbuf_free(receiveBuffer);
            netbuf_delete(receiveBuffer);
        }

        if (err == ERR_OK)
        {
            uint8_t qLength = getQuestionLength(message);

            if (qLength > 0)
            {
                message.setAnswer(&m_ServerIp, qLength);
                auto buf = message.serialize();
                netbuf *sendBuffer = netbuf_new();

                netbuf_ref(sendBuffer, buf.data(), buf.size());
                netconn_sendto(m_Connection, sendBuffer, &clientIp, clientPort);
                netbuf_delete(sendBuffer);
            }
        }
    }
}

uint8_t getQuestionLength(DnsMessage &message)
{
    auto &ref = message.getRecords();

    // Calculate length of QNAME
    uint8_t length = 1 + std::distance(ref.begin(), std::find(ref.begin(), ref.end(), 0));

    // Check QNAME length
    if (length > 0 && length <= 63)
    {
        length += 4; // add QTYPE and QCLASS field lengths (2 bytes each)
    }
    else { length = 0; }

    return length;
}

} // namespace Task