#ifndef UDPSERVER_HPP
#define UDPSERVER_HPP

#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>

namespace Network::Udp
{

class UdpServer
{
  public:
    UdpServer() = default;
    virtual int process(pbuf *p, const ip_addr_t *src_addr, u16_t src_port, netif *nif) = 0;
};

} // namespace Network::Udp

#endif /* UDPSERVER_HPP */
