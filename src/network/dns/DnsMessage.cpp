#include "DnsMessage.hpp"

#include "network/NetworkCommon.hpp"
#include <lwip/def.h>
#include <lwip/ip4_addr.h>
#include <lwip/netbuf.h>

#include <cstdio>

namespace Network::Dns
{

DnsMessage::DnsMessage(netbuf *buffer)
{
    uint16_t offset = 0;

    deserialize(buffer, m_Id, offset);
    deserialize(buffer, m_Flags, offset);
    deserialize(buffer, m_Questions, offset);
    deserialize(buffer, m_Answers, offset);
    deserialize(buffer, m_AuthorityRRs, offset);
    deserialize(buffer, m_AdditionalRRs, offset);
    deserialize(buffer, m_Records, offset);
}

bool DnsMessage::isStandardQuery()
{
    bool result = ((m_Flags >> 15) & 0x1) == 0;
    if (result) { result = ((m_Flags >> 11) & 0xf) == 0; }
    return result;
}

uint16_t DnsMessage::getQCount() { return m_Questions; }

const std::vector<uint8_t> &DnsMessage::getRecords() { return m_Records; }

void DnsMessage::setAnswer(ip_addr_t *ip, uint8_t qLength)
{
    constexpr uint8_t DNS_HEADER_LENGTH = 12;

    uint16_t flags = lwip_ntohs(m_Flags);

    flags |= 1 << 15;   // Set as response
    flags |= 1 << 10;   // Set as authoritative answer
    flags &= ~(1 << 9); // No truncation needed
    flags |= 1 << 7;    // Set recursion as available

    m_Flags = lwip_htons(flags);
    m_Answers = lwip_htons(1);
    m_AuthorityRRs = 0;
    m_AdditionalRRs = 0;

    m_Records.erase(m_Records.begin() + qLength, m_Records.end());

    // Generate answer
    // NAME
    m_Records.emplace_back(0xc0); // Pointer to question
    m_Records.emplace_back(DNS_HEADER_LENGTH);
    // TYPE
    m_Records.emplace_back(0);
    m_Records.emplace_back(1);
    // CLASS
    m_Records.emplace_back(0);
    m_Records.emplace_back(1);
    // TTL
    m_Records.emplace_back(0);
    m_Records.emplace_back(0);
    m_Records.emplace_back(0);
    m_Records.emplace_back(60);
    // RDATA LENGTH
    m_Records.emplace_back(0);
    m_Records.emplace_back(4);
    // RDATA (IP address)
    m_Records.emplace_back(ip4_addr_get_byte(ip, 0));
    m_Records.emplace_back(ip4_addr_get_byte(ip, 1));
    m_Records.emplace_back(ip4_addr_get_byte(ip, 2));
    m_Records.emplace_back(ip4_addr_get_byte(ip, 3));
}

std::vector<uint8_t> DnsMessage::serialize() const
{
    std::vector<uint8_t> result = {static_cast<uint8_t>(m_Id),
                                   static_cast<uint8_t>(m_Id >> 8),
                                   static_cast<uint8_t>(m_Flags),
                                   static_cast<uint8_t>(m_Flags >> 8),
                                   static_cast<uint8_t>(m_Questions),
                                   static_cast<uint8_t>(m_Questions >> 8),
                                   static_cast<uint8_t>(m_Answers),
                                   static_cast<uint8_t>(m_Answers >> 8),
                                   static_cast<uint8_t>(m_AuthorityRRs),
                                   static_cast<uint8_t>(m_AuthorityRRs >> 8),
                                   static_cast<uint8_t>(m_AdditionalRRs),
                                   static_cast<uint8_t>(m_AdditionalRRs >> 8)};
    result.insert(result.end(), m_Records.begin(), m_Records.end());

    return result;
}

} // namespace Network::Dns