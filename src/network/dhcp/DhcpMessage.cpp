#include "DhcpMessage.hpp"

#include "network/NetworkCommon.hpp"
#include "network/dhcp/DhcpCommon.hpp"
#include "network/dhcp/DhcpLease.hpp"
#include <lwip/ip4_addr.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <vector>

namespace Network::Dhcp
{

constexpr uint8_t DHCP_RESPONSE = 0x02;

DhcpMessage::DhcpMessage(netbuf *netBuffer)
{
    uint16_t offset = 0;

    deserialize(netBuffer, m_Op, offset);
    deserialize(netBuffer, m_Htype, offset);
    deserialize(netBuffer, m_Hlen, offset);
    deserialize(netBuffer, m_Hops, offset);
    deserialize(netBuffer, m_Xid, offset);
    deserialize(netBuffer, m_Secs, offset);
    deserialize(netBuffer, m_Flags, offset);
    deserialize(netBuffer, m_Ciaddr, offset);
    deserialize(netBuffer, m_Yiaddr, offset);
    deserialize(netBuffer, m_Siaddr, offset);
    deserialize(netBuffer, m_Giaddr, offset);
    deserialize(netBuffer, m_Chaddr, offset);
    deserialize(netBuffer, m_Sname, offset);
    deserialize(netBuffer, m_File, offset);
    deserialize(netBuffer, m_MagicCookie, offset);
    deserialize(netBuffer, m_Options, offset);
}

void DhcpMessage::setAsResponse()
{
    m_Op = DHCP_RESPONSE;

    // Clear the options vector and set starting magic cookie
    m_Options.clear();
    m_Options.emplace_back(DHCP_OPT_END);
}

void DhcpMessage::setIpAddresses(ip_addr_t *serverIp, uint8_t clientOctet)
{
    m_Siaddr = {ip4_addr_get_byte(serverIp, 0),
                ip4_addr_get_byte(serverIp, 1),
                ip4_addr_get_byte(serverIp, 2),
                ip4_addr_get_byte(serverIp, 3)};

    m_Yiaddr = {ip4_addr_get_byte(serverIp, 0),
                ip4_addr_get_byte(serverIp, 1),
                ip4_addr_get_byte(serverIp, 2),
                clientOctet};
}

void DhcpMessage::writeOption(DhcpOptions option, uint8_t data)
{
    writeOption(option, std::vector<uint8_t>{data});
}

void DhcpMessage::writeOption(DhcpOptions option, uint16_t data)
{
    writeOption(option,
                std::vector<uint8_t>{static_cast<uint8_t>(data),
                                     static_cast<uint8_t>(data >> 8)});
}

void DhcpMessage::writeOption(DhcpOptions option, uint32_t data)
{
    writeOption(option,
                std::vector<uint8_t>{static_cast<uint8_t>(data),
                                     static_cast<uint8_t>(data >> 8),
                                     static_cast<uint8_t>(data >> 16),
                                     static_cast<uint8_t>(data >> 24)});
}

void DhcpMessage::writeOption(DhcpOptions option, std::vector<uint8_t> data)
{
    constexpr uint16_t MAX_OPTION_SIZE = 312;

    if (m_Op == DHCP_RESPONSE && data.size() < std::numeric_limits<uint8_t>::max()
        && m_Options.size() + data.size() < MAX_OPTION_SIZE)
    {
        if (m_Options.back() == DHCP_OPT_END) { m_Options.pop_back(); }
        m_Options.push_back(option);
        m_Options.push_back(data.size());
        std::move(data.begin(), data.end(), std::back_inserter(m_Options));
        m_Options.push_back(DHCP_OPT_END);
    }
}

std::array<uint8_t, MAC_LEN> DhcpMessage::getClientMac() const
{
    std::array<uint8_t, MAC_LEN> clientMac;
    std::memcpy(clientMac.data(), m_Chaddr.data(), clientMac.size());
    return clientMac;
}

std::vector<uint8_t> DhcpMessage::getOption(DhcpOptions option) const
{
    std::vector<uint8_t> result{};

    // DHCP options use type–length–value encoding.
    // To find the next option in the chain, read the byte containing the length of the value field,
    // and then jump forward 2 bytes (type and length) + the length of the value field.
    auto it = m_Options.cbegin();
    while (*it != option && it < m_Options.cend()) { it += 2 + *(it + 1); }
    if (*it == option) { result.assign(it + 2, it + 2 + *(it + 1)); }

    return result;
}

std::vector<uint8_t> DhcpMessage::serialize() const
{
    std::vector<uint8_t> result = {m_Op,
                                   m_Htype,
                                   m_Hlen,
                                   m_Hops,
                                   static_cast<uint8_t>(m_Xid),
                                   static_cast<uint8_t>(m_Xid >> 8),
                                   static_cast<uint8_t>(m_Xid >> 16),
                                   static_cast<uint8_t>(m_Xid >> 24),
                                   static_cast<uint8_t>(m_Secs),
                                   static_cast<uint8_t>(m_Secs >> 8),
                                   static_cast<uint8_t>(m_Flags),
                                   static_cast<uint8_t>(m_Flags >> 8)};
    result.insert(result.end(), m_Ciaddr.begin(), m_Ciaddr.end());
    result.insert(result.end(), m_Yiaddr.begin(), m_Yiaddr.end());
    result.insert(result.end(), m_Siaddr.begin(), m_Siaddr.end());
    result.insert(result.end(), m_Giaddr.begin(), m_Giaddr.end());
    result.insert(result.end(), m_Chaddr.begin(), m_Chaddr.end());
    result.insert(result.end(), m_Sname.begin(), m_Sname.end());
    result.insert(result.end(), m_File.begin(), m_File.end());
    result.insert(result.end(), m_MagicCookie.begin(), m_MagicCookie.end());
    result.insert(result.end(), m_Options.begin(), m_Options.end());

    return result;
}

} // namespace Network::Dhcp