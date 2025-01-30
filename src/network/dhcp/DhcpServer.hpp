#ifndef DHCPSERVER_HPP
#define DHCPSERVER_HPP

#include "network/udp/UdpServer.hpp"
#include <lwip/ip_addr.h>
#include <lwip/udp.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Small DHCP server, supports only /24 networks, leases start from x.x.x.50 onward
namespace Network::Dhcp
{

class DhcpServer : public Udp::UdpServer
{
  public:
    DhcpServer(const std::shared_ptr<std::string> serverIp, uint8_t leaseMax);
    DhcpServer(const DhcpServer &) = delete;
    ~DhcpServer();
    int process(pbuf *p, const ip_addr_t *src_addr, u16_t src_port, netif *nif) override;

  private:
    ip_addr_t m_Ip{};
    ip_addr_t m_Netmask{};
    uint8_t m_LeaseMax{24};
    udp_pcb *m_UdpPcb{};
    std::vector<struct DhcpLease> m_Leases;
    int send(netif *nif, const void *buf, size_t len, uint32_t ip, uint16_t port);
};

} // namespace Network::Dhcp

#endif /* DHCPSERVER_HPP */
