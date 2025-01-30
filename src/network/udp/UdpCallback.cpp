#include "UdpCallback.hpp"

#include "network/udp/UdpServer.hpp"

#include <cerrno>
#include <cstring>

namespace Network::Udp
{

void udpReceive(void *arg, udp_pcb *upcb, pbuf *p, const ip_addr_t *src_addr, u16_t src_port)
{
    auto *server = static_cast<UdpServer *>(arg);
    (void)upcb;

    netif *nif = ip_current_input_netif();

    server->process(p, src_addr, src_port, nif);
}

} // namespace Network::Udp