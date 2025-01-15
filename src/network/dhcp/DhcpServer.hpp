#ifndef DHCPSERVER_HPP
#define DHCPSERVER_HPP

#include "network/NetworkCommon.hpp"
#include <lwip/ip_addr.h>
#include <lwip/udp.h>

#include <cstdint>
#include <string>
#include <vector>

// Small DHCP server, supports only /24 networks, leases start from x.x.x.50 onward
namespace Network::Dhcp
{

class DhcpServer : public UdpServer
{
  public:
    DhcpServer(const std::string &serverIp, uint8_t leaseMax);
    ~DhcpServer();
    [[nodiscard]] constexpr const udp_pcb *getUdp() const;
    int process(pbuf *p, const ip_addr_t *src_addr, u16_t src_port, netif *nif) override;

  private:
    ip_addr_t m_Ip{};
    ip_addr_t m_Netmask{};
    uint8_t m_LeaseMax{24};
    udp_pcb *m_Udp{};
    std::vector<struct DhcpLease> m_Leases;
};

} // namespace Network::Dhcp

#endif /* DHCPSERVER_HPP */
