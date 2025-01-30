#ifndef NETWORKCOMMON_HPP
#define NETWORKCOMMON_HPP

#include "FreeRTOS.h" // IWYU pragma: keep
#include "event_groups.h"

namespace Network
{

enum NetworkEventBits : EventBits_t
{
    WIFI = 0x1,
    DHCP = 0x2,
    DNS = 0x4,
    HTTP = 0x8
};

constexpr EventBits_t NetworkAllEventBits = 0xF;

} // namespace Network

#endif /* NETWORKCOMMON_HPP */
