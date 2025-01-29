#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "network/tcp/TcpServer.hpp"
#include <lwip/altcp.h>
#include <lwip/ip_addr.h>

#include <memory>
#include <string>

namespace Network::Http
{

class HttpServer : public Tcp::TcpServer
{
  public:
    HttpServer(const std::shared_ptr<std::string> serverIp);
    HttpServer(const HttpServer &) = delete;
    ~HttpServer();
    err_t accept(altcp_pcb *clientPcb, err_t err) override;
    err_t respond(Tcp::TcpConnection &connection, pbuf *p, err_t err) const override;

  private:
    ip_addr_t m_Ip;
    ip_addr_t m_Netmask;
    altcp_pcb *m_TcpPcb;
    [[nodiscard]] err_t handleRequest(Tcp::TcpConnection &connection,
                                      const std::string &request) const;
    void removeDisconnected();
};

} // namespace Network::Http

#endif /* HTTPSERVER_HPP */
