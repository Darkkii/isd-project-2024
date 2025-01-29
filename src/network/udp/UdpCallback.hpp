#ifndef UDPCALLBACK_HPP
#define UDPCALLBACK_HPP

#include <lwip/udp.h>

namespace Network::Udp
{

void udpReceive(void *arg, udp_pcb *upcb, pbuf *p, const ip_addr_t *src_addr, u16_t src_port);

} // namespace Network::Udp

#endif /* UDPCALLBACK_HPP */
