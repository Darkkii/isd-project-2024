#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include "network/tcp/TcpConnection.hpp"
#include <lwip/altcp.h>

#include <vector>

namespace Network::Tcp
{

class TcpServer
{
  public:
    TcpServer() = default;
    virtual err_t accept(altcp_pcb *clientPcb, err_t err) = 0;
    virtual err_t respond(TcpConnection &connection, pbuf *p, err_t err) const = 0;

  protected:
    std::vector<TcpConnection> m_Connections{};
};

} // namespace Network::Tcp

#endif /* TCPSERVER_HPP */
