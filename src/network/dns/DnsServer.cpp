#include "DnsServer.hpp"

#include "DnsCommon.hpp"
#include <lwip/err.h>
#include <lwip/ip4_addr.h>

// #include <array>
#include <cstdint>
#include <cstdio>
#include <iterator>

namespace Network::Dns
{

DnsServer::DnsServer(const std::string &serverIp)
{
    m_Udp = udp_new();
    ip4addr_aton(serverIp.c_str(), ip_2_ip4(&m_Ip));

    udp_recv(m_Udp, udpReceive, this);

    udp_bind(m_Udp, IP4_ADDR_ANY, PORT_DNS_SERVER);

    printf("DNS server listening on port %d\n", PORT_DNS_SERVER);
}

DnsServer::~DnsServer()

{
    if (m_Udp != nullptr)
    {
        udp_remove(m_Udp);
        m_Udp = nullptr;
    }
}

int DnsServer::process(pbuf *p, const ip_addr_t *src_addr, u16_t src_port, netif *nif)
{
    constexpr size_t DNS_HEADER_SIZE = DNS_MESSAGE_LEN_MAX - DNS_RECORDS_LEN_MAX;

    printf("dns_server_process %u\n", p->tot_len);

    (void)nif;

    // std::array<uint8_t, MAX_DNS_MSG_SIZE> message;
    DnsMessage message;
    // auto *header = (dns_header_t *)message.data();
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

    // Print the question
    printf("question: ");
    // const uint8_t *question_ptr_start = message.data() +
    // sizeof(dns_header_t); const uint8_t *question_ptr_end = message.data() +
    // message.size(); const uint8_t *question_ptr = question_ptr_start;
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
                printf("Invalid label\n");
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
        printf("Invalid question length\n");
        pbuf_free(p);
        return -1;
    }

    // Skip QNAME and QTYPE
    currentRecord += 4;

    // Generate answer
    // uint8_t *answer_ptr = message.data() + (question_ptr - message.data());
    // uint8_t *answer = message.records + (question - message.records);

    uint8_t index = std::distance(recordsStart, currentRecord);
    message.records[index++] = 0xc0; // pointer
    message.records[index++] = DNS_HEADER_SIZE;
    // *answer++ = recordsStart - message.records; // pointer to question

    message.records[index++] = 0;
    message.records[index++] = 1; // host address

    message.records[index++] = 0;
    message.records[index++] = 1; // Internet class

    message.records[index++] = 0;
    message.records[index++] = 0;
    message.records[index++] = 0;
    message.records[index++] = 60; // ttl 60s

    message.records[index++] = 0;
    message.records[index++] = 4;                                 // length
    memcpy(&message.records[index], &ip4_addr_get_u32(&m_Ip), 4); // use our address
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
    udpSend(m_Udp,
            nullptr,
            &message,
            sizeof(message) - (recordsEnd - &message.records[index]),
            ip4_addr_get_u32(src_addr),
            src_port);

    pbuf_free(p);

    return ERR_OK;
}

} // namespace Network::Dns