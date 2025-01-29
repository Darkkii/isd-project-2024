#include "TcpCallback.hpp"

#include "TcpConnection.hpp"
#include "TcpServer.hpp"

namespace Network::Tcp
{

err_t tcpAccept(void *arg, altcp_pcb *clientPcb, err_t err)
{
    auto *server = static_cast<TcpServer *>(arg);
    return server->accept(clientPcb, err);
}

err_t tcpReceive(void *arg, altcp_pcb *clientPcb, pbuf *p, err_t err)
{
    auto *connection = static_cast<Tcp::TcpConnection *>(arg);
    return connection->receive(p, err);
}

err_t tcpSent(void *arg, altcp_pcb *clientPcb, uint16_t sent)
{
    auto *connection = static_cast<Tcp::TcpConnection *>(arg);
    return connection->sent(sent);
}

err_t tcpPoll(void *arg, struct altcp_pcb *clientPcb)
{
    auto *connection = static_cast<Tcp::TcpConnection *>(arg);
    return connection->poll();
}

void tcpError(void *arg, err_t err)
{
    auto *connection = static_cast<Tcp::TcpConnection *>(arg);
    return connection->error(err);
}

} // namespace Network::Tcp