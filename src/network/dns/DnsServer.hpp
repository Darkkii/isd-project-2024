#ifndef DNSSERVER_HPP
#define DNSSERVER_HPP

#include "lwip/ip_addr.h"
#include "network/NetworkCommon.hpp"
#include <lwip/udp.h>

#include <string>

namespace Network::Dns
{

class DnsServer : public UdpServer
{
  public:
    DnsServer(const std::string &serverIp);
    ~DnsServer();
    int process(pbuf *p, const ip_addr_t *src_addr, u16_t src_port, netif *nif) override;

  private:
    ip_addr_t m_Ip{};
    udp_pcb *m_Udp{};
};

} // namespace Network::Dns

#endif /* DNSSERVER_HPP */
