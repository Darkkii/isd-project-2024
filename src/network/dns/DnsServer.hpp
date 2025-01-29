#ifndef DNSSERVER_HPP
#define DNSSERVER_HPP

#include "network/udp/UdpServer.hpp"
#include <lwip/ip_addr.h>
#include <lwip/udp.h>

#include <memory>
#include <string>

namespace Network::Dns
{

class DnsServer : public Udp::UdpServer
{
  public:
    DnsServer(const std::shared_ptr<std::string> serverIp);
    DnsServer(const DnsServer &) = delete;
    ~DnsServer();
    int process(pbuf *p, const ip_addr_t *src_addr, u16_t src_port, netif *nif) override;

  private:
    ip_addr_t m_Ip{};
    udp_pcb *m_UdpPcb{};
    int send(const void *buf, size_t len, const ip_addr_t *dest, uint16_t port);
};

} // namespace Network::Dns

#endif /* DNSSERVER_HPP */
