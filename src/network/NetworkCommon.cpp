#include "NetworkCommon.hpp"

#include <cctype>
#include <cerrno>
#include <cstdio>

namespace Network
{

int udpSend(udp_pcb *udp, netif *nif, const void *buf, size_t len, uint32_t ip, uint16_t port)
{
    ip_addr_t dest;
    err_t err;
    pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

    if (len > 0xffff) { len = 0xffff; }

    if (p == nullptr) { return -ENOMEM; }

    std::memcpy(p->payload, buf, len);

    IP4_ADDR(ip_2_ip4(&dest), ip >> 24 & 0xff, ip >> 16 & 0xff, ip >> 8 & 0xff, ip & 0xff);

    if (nif != nullptr) { err = udp_sendto_if(udp, p, &dest, port, nif); }
    else { err = udp_sendto(udp, p, &dest, port); }

    pbuf_free(p);

    if (err != ERR_OK) { return err; }

    return len;
}

void udpReceive(void *arg, udp_pcb *upcb, pbuf *p, const ip_addr_t *src_addr, u16_t src_port)
{
    auto *server = static_cast<UdpServer *>(arg);
    (void)upcb;

    netif *nif = ip_current_input_netif();

    server->process(p, src_addr, src_port, nif);

    pbuf_free(p);
}

} // namespace Network