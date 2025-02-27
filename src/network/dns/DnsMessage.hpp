#ifndef DNSMESSAGE_HPP
#define DNSMESSAGE_HPP

#include <lwip/ip_addr.h>
#include <lwip/netbuf.h>

#include <cstdint>
#include <vector>

namespace Network::Dns
{

class DnsMessage
{
  public:
    DnsMessage() = default;
    DnsMessage(netbuf *buffer);
    bool isStandardQuery();
    [[nodiscard]] uint16_t getQCount();
    const std::vector<uint8_t> &getRecords();
    void setAnswer(ip_addr_t *ip, uint8_t qLength);
    [[nodiscard]] std::vector<uint8_t> serialize() const;

  private:
    uint16_t m_Id{};
    uint16_t m_Flags{};
    uint16_t m_Questions{};
    uint16_t m_Answers{};
    uint16_t m_AuthorityRRs{};
    uint16_t m_AdditionalRRs{};
    std::vector<uint8_t> m_Records{};
};

} // namespace Network::Dns

#endif /* DNSMESSAGE_HPP */
