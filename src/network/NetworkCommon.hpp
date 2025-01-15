#ifndef NETWORKCOMMON_HPP
#define NETWORKCOMMON_HPP

#include <lwip/ip_addr.h>
#include <lwip/udp.h>

#include <cstdint>
#include <cstring>

namespace Network
{
int udpSend(udp_pcb *udp, netif *nif, const void *buf, size_t len, uint32_t ip, uint16_t port);
void udpReceive(void *arg, udp_pcb *upcb, pbuf *p, const ip_addr_t *src_addr, u16_t src_port);

class UdpServer
{
  public:
    UdpServer() = default;
    virtual int process(pbuf *p, const ip_addr_t *src_addr, u16_t src_port, netif *nif) = 0;
};

} // namespace Network

#endif /* NETWORKCOMMON_HPP */
