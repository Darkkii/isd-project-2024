#ifndef TCPCALLBACK_HPP
#define TCPCALLBACK_HPP

#include <lwip/altcp.h>

namespace Network::Tcp
{

err_t tcpAccept(void *arg, altcp_pcb *clientPcb, err_t err);
err_t tcpReceive(void *arg, altcp_pcb *clientPcb, pbuf *p, err_t err);
err_t tcpSent(void *arg, altcp_pcb *clientPcb, uint16_t sent);
err_t tcpPoll(void *arg, altcp_pcb *clientPcb);
void tcpError(void *arg, err_t err);

} // namespace Network::Tcp

#endif /* TCPCALLBACK_HPP */
