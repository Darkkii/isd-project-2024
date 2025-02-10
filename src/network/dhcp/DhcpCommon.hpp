#ifndef DHCPCOMMON_HPP
#define DHCPCOMMON_HPP

#include <cstdint>

namespace Network::Dhcp
{

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

} // namespace Network::Dhcp

#endif /* DHCPCOMMON_HPP */
