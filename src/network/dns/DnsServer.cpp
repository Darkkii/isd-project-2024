#include "DnsServer.hpp"

#include "DnsCommon.hpp"
#include "network/udp/UdpCallback.hpp"
#include <lwip/err.h>
#include <lwip/ip4_addr.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>

namespace Network::Dns
{

DnsServer::DnsServer(const std::shared_ptr<std::string> serverIp)
{
    constexpr uint16_t PORT_DNS_SERVER = 53;

    ip4addr_aton(serverIp->c_str(), ip_2_ip4(&m_Ip));

    m_UdpPcb = udp_new();
    udp_recv(m_UdpPcb, Udp::udpReceive, this);

    ip_addr_t addr;
    ip4_addr_set_zero(&addr);

    udp_bind(m_UdpPcb, &addr, PORT_DNS_SERVER);

    printf("DNS server listening on port %d\n", PORT_DNS_SERVER);
}

DnsServer::~DnsServer()
{
    if (m_UdpPcb != nullptr)
    {
        udp_remove(m_UdpPcb);
        m_UdpPcb = nullptr;
    }
}

int DnsServer::process(pbuf *p, const ip_addr_t *src_addr, u16_t src_port, netif *nif)
{
    constexpr size_t DNS_HEADER_SIZE = DNS_MESSAGE_LEN_MAX - DNS_RECORDS_LEN_MAX;

    (void)nif;

    DnsMessage message;
    size_t len = pbuf_copy_partial(p, &message, sizeof(message), 0);
    uint16_t flags = lwip_ntohs(message.flags);
    uint16_t qCount = lwip_ntohs(message.question_count);

    if (len < DNS_HEADER_SIZE)
    {
        pbuf_free(p);
        return -1;
    }

    printf("len %d\n", len);
    printf("dns flags 0x%x\n", flags);
    printf("dns question count 0x%x\n", qCount);

    // Check QR indicates a query
    if (((flags >> 15) & 0x1) != 0)
    {
        printf("Ignoring non-query\n");
        pbuf_free(p);
        return -1;
    }

    // Check for standard query
    if (((flags >> 11) & 0xf) != 0)
    {
        printf("Ignoring non-standard query\n");
        pbuf_free(p);
        return -1;
    }

    // Check question count
    if (qCount < 1)
    {
        printf("Invalid question count\n");
        pbuf_free(p);
        return -1;
    }

    printf("question: ");
    auto *recordsStart = std::cbegin(message.records);
    auto *recordsEnd = std::cend(message.records);
    auto *currentRecord = recordsStart;

    while (currentRecord < recordsEnd)
    {
        if (*currentRecord == 0)
        {
            currentRecord++;
            break;
        }
        else
        {
            if (currentRecord > recordsStart) { printf("."); }
            int label_len = *currentRecord++;
            if (label_len > 63)
            {
                printf("[Error %d] - Invalid label.\n", -1);
                pbuf_free(p);
                return -1;
            }
            printf("%.*s", label_len, currentRecord);
            currentRecord += label_len;
        }
    }

    printf("\n");

    // Check question length
    if (std::distance(recordsStart, currentRecord) > 255)
    {
        printf("[Error %d] - Invalid question length\n", -1);
        pbuf_free(p);
        return -1;
    }

    // Skip QNAME and QTYPE
    currentRecord += 4;

    // Generate answer
    uint8_t index = std::distance(recordsStart, currentRecord);
    message.records[index++] = 0xc0; // pointer
    message.records[index++] = DNS_HEADER_SIZE;

    message.records[index++] = 0;
    message.records[index++] = 1; // host address

    message.records[index++] = 0;
    message.records[index++] = 1; // Internet class

    message.records[index++] = 0;
    message.records[index++] = 0;
    message.records[index++] = 0;
    message.records[index++] = 60; // ttl 60s

    message.records[index++] = 0;
    message.records[index++] = 4;                                      // length
    std::memcpy(&message.records[index], &ip4_addr_get_u32(&m_Ip), 4); // use our address
    index += 4;

    message.flags = lwip_htons(0x1 << 15 | // QR = response
                               0x1 << 10 | // AA = authoritative
                               0x1 << 7);  // RA = authenticated
    message.question_count = lwip_htons(1);
    message.answer_record_count = lwip_htons(1);
    message.authority_record_count = 0;
    message.additional_record_count = 0;

    // Send the reply
    printf("Sending %d byte reply to %s:%d\n",
           sizeof(message) - (recordsEnd - &message.records[index]),
           ipaddr_ntoa(src_addr),
           src_port);

    send(&message, sizeof(message) - (recordsEnd - &message.records[index]), src_addr, src_port);

    pbuf_free(p);

    return ERR_OK;
}

int DnsServer::send(const void *buf, size_t len, const ip_addr_t *dest, uint16_t port)
{
    if (len > 0xffff) { len = 0xffff; }

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (p == nullptr)
    {
        printf("DNS: Failed to send message out of memory\n");
        return -ENOMEM;
    }

    memcpy(p->payload, buf, len);
    err_t err = udp_sendto(m_UdpPcb, p, dest, port);

    pbuf_free(p);

    if (err != ERR_OK)
    {
        printf("DNS: Failed to send message %d\n", err);
        return err;
    }

    return len;
}

} // namespace Network::Dns