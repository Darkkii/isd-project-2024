#ifndef DNSSERVERTASK_HPP
#define DNSSERVERTASK_HPP

#include "network/NetworkGroup.hpp"
#include "task/BaseTask.hpp"
#include <lwip/api.h>

#include <memory>

namespace Task
{

class DnsServerTask : public BaseTask
{
  public:
    DnsServerTask(const std::shared_ptr<std::string> serverIp,
                  std::shared_ptr<Network::NetworkGroup> networkGroup);
    void run() override;

  private:
    netconn *m_Connection{nullptr};
    ip_addr_t m_ServerIp{};
    std::shared_ptr<Network::NetworkGroup> m_NetworkGroup;
};

} // namespace Task

#endif /* DNSSERVERTASK_HPP */
