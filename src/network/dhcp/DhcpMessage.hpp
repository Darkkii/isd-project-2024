#ifndef DHCPMESSAGE_HPP
#define DHCPMESSAGE_HPP

#include "network/dhcp/DhcpCommon.hpp"
#include "network/dhcp/DhcpLease.hpp"
#include <lwip/ip_addr.h>
#include <lwip/netbuf.h>

#include <array>
#include <cstdint>
#include <vector>

namespace Network::Dhcp
{

class DhcpMessage
{
  public:
    DhcpMessage() = default;
    DhcpMessage(netbuf *netBuffer);
    void setAsResponse();
    void setIpAddresses(ip_addr_t *serverIp, uint8_t clientOctet);
    void writeOption(DhcpOptions option, uint8_t data);
    void writeOption(DhcpOptions option, uint16_t data);
    void writeOption(DhcpOptions option, uint32_t data);
    void writeOption(DhcpOptions option, std::vector<uint8_t> data);
    [[nodiscard]] std::array<uint8_t, MAC_LEN> getClientMac() const;
    [[nodiscard]] std::vector<uint8_t> getOption(DhcpOptions option) const;
    [[nodiscard]] std::vector<uint8_t> serialize() const;

  private:
    // Overview of the variables used: https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol
    uint8_t m_Op{};
    uint8_t m_Htype{};
    uint8_t m_Hlen{};
    uint8_t m_Hops{};
    uint32_t m_Xid{};
    uint16_t m_Secs{};
    uint16_t m_Flags{};
    std::array<uint8_t, 4> m_Ciaddr{};
    std::array<uint8_t, 4> m_Yiaddr{};
    std::array<uint8_t, 4> m_Siaddr{};
    std::array<uint8_t, 4> m_Giaddr{};
    std::array<uint8_t, 16> m_Chaddr{};
    std::array<uint8_t, 64> m_Sname{};
    std::array<uint8_t, 128> m_File{};
    std::array<uint8_t, 4> m_MagicCookie{};
    std::vector<uint8_t> m_Options{};
};

} // namespace Network::Dhcp

#endif /* DHCPMESSAGE_HPP */
