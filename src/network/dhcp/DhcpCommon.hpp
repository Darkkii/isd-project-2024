#ifndef DHCPCOMMON_HPP
#define DHCPCOMMON_HPP

#include <array>
#include <cstdint>

namespace Network::Dhcp
{

constexpr uint8_t MAC_LEN = 6;
constexpr uint8_t DHCPS_BASE_IP = 50;

enum DhcpMessageType : uint8_t
{
    DHCPDISCOVER = 1,
    DHCPOFFER,
    DHCPREQUEST,
    DHCPDECLINE,
    DHCPACK,
    DHCPNAK,
    DHCPRELEASE,
    DHCPINFORM
};

enum DhcpOptions : uint8_t
{
    DHCP_OPT_PAD = 0,
    DHCP_OPT_SUBNET_MASK,
    DHCP_OPT_ROUTER = 3,
    DHCP_OPT_DNS = 6,
    DHCP_OPT_HOST_NAME = 12,
    DHCP_OPT_REQUESTED_IP = 50,
    DHCP_OPT_IP_LEASE_TIME,
    DHCP_OPT_MSG_TYPE = 53,
    DHCP_OPT_SERVER_ID,
    DHCP_OPT_PARAM_REQUEST_LIST,
    DHCP_OPT_MAX_MSG_SIZE = 57,
    DHCP_OPT_VENDOR_CLASS_ID = 60,
    DHCP_OPT_CLIENT_ID,
    DHCP_OPT_END = 255
};

struct DhcpLease
{
    std::array<uint8_t, MAC_LEN> mac{};
    uint16_t expiry{};
};

extern "C"
{
    struct DhcpPayload
    {
        uint8_t op{};    // message opcode
        uint8_t htype{}; // hardware address type
        uint8_t hlen{};  // hardware address length
        uint8_t hops{};
        uint32_t xid{};  // transaction id, chosen by client
        uint16_t secs{}; // client seconds elapsed
        uint16_t flags{};
        uint8_t ciaddr[4]{};  // client IP address
        uint8_t yiaddr[4]{};  // your IP address
        uint8_t siaddr[4]{};  // next server IP address
        uint8_t giaddr[4]{};  // relay agent IP address
        uint8_t chaddr[16]{}; // client hardware address
        uint8_t sname[64]{};  // server host name, bootp legacy
        uint8_t file[128]{};  // boot file name, bootp legacy
        uint8_t options[312]{}; // optional parameters, variable, starts with magic
    };
};

} // namespace Network::Dhcp

#endif /* DHCPCOMMON_HPP */
