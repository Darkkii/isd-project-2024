#ifndef DHCPLEASE_HPP
#define DHCPLEASE_HPP

#include <pico/time.h>
#include <pico/types.h>

#include <array>
#include <cstdint>

namespace Network::Dhcp
{

constexpr uint8_t MAC_LEN = 6;

class DhcpLease
{
  public:
    constexpr DhcpLease() = default;
    DhcpLease(std::array<uint8_t, MAC_LEN> macAddress, uint32_t expiry);
    [[nodiscard]] const std::array<uint8_t, MAC_LEN> mac() const;
    [[nodiscard]] const bool hasExpired() const;

  private:
    std::array<uint8_t, MAC_LEN> m_Mac{0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    absolute_time_t m_Expiry{};
};

} // namespace Network::Dhcp

#endif /* DHCPLEASE_HPP */
