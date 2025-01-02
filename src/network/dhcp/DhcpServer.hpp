#ifndef DHCPSERVER_HPP
#define DHCPSERVER_HPP

#include <lwip/ip_addr.h>
#include <lwip/udp.h>

#include <cstdint>
#include <string>
#include <vector>

// Small DHCP server, supports only /24 networks, leases start from x.x.x.50 onward
namespace Network::Dhcp
{

class DhcpServer
{
  public:
    DhcpServer(const std::string &network, uint8_t leaseMax);
    ~DhcpServer();
    [[nodiscard]] constexpr const udp_pcb *getUdp() const;
    int handleRequest(struct DhcpPayload &payload, struct netif *nif);

  private:
    ip_addr_t m_Ip{};
    ip_addr_t m_Netmask{};
    uint8_t m_LeaseMax{24};
    udp_pcb *m_Udp{};
    std::vector<struct DhcpLease> m_Leases;
};

} // namespace Network::Dhcp

#endif /* DHCPSERVER_HPP */
