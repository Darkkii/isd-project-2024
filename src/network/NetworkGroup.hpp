#ifndef NETWORKGROUP_HPP
#define NETWORKGROUP_HPP

#include "rtos/EventGroup.hpp"

namespace Network
{

enum NetworkBits : EventBits_t
{
    AP = 0x1,
    DHCP = 0x2,
    DNS = 0x4,
    HTTP = 0x8,
    AP_DHCP = AP + DHCP,
    AP_DHCP_DNS = AP_DHCP + DNS,
    AP_DHCP_DNS_HTTP = AP_DHCP_DNS + HTTP
};

class NetworkGroup final : public Rtos::EventGroup
{
  public:
    void set(NetworkBits bits);
    void wait(NetworkBits bits);

  private:
};

} // namespace Network

#endif /* NETWORKGROUP_HPP */
