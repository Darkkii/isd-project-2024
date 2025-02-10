#ifndef DHCPSERVERTASK_HPP
#define DHCPSERVERTASK_HPP

#include "FreeRTOS.h" // IWYU pragma: keep
#include "event_groups.h"
#include "network/dhcp/DhcpLease.hpp"
#include "task/BaseTask.hpp"
#include <lwip/api.h>
#include <lwip/err.h>

#include <map>
#include <memory>

namespace Task
{

// Small DHCP server, supports only /24 networks, up to 100 leases starting from x.x.x.50
class DhcpServerTask : public BaseTask
{
  public:
    DhcpServerTask(const std::shared_ptr<std::string> serverIp,
                   const std::shared_ptr<std::string> netmask);
    void run() override;

  private:
    netconn *dhcpConnection{nullptr};
    ip_addr_t m_ServerIp{};
    ip_addr_t m_Netmask{};
    EventGroupHandle_t m_EventGroup;
    std::map<uint8_t, Network::Dhcp::DhcpLease> m_Leases;
};

} // namespace Task

#endif /* DHCPSERVERTASK_HPP */
