#include "DhcpLease.hpp"

#include <hardware/timer.h>
#include <pico/time.h>

namespace Network::Dhcp
{

DhcpLease::DhcpLease(std::array<uint8_t, MAC_LEN> macAddress, uint32_t expiry) :
    m_Mac{macAddress},
    m_Expiry{make_timeout_time_ms(1000 * expiry)}
{}

const std::array<uint8_t, MAC_LEN> DhcpLease::mac() const { return m_Mac; };

const bool DhcpLease::hasExpired() const { return time_reached(m_Expiry); };

} // namespace Network::Dhcp
