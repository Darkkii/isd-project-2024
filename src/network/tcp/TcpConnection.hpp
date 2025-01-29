#ifndef TCPCONNECTION_HPP
#define TCPCONNECTION_HPP

#include "fs/File.hpp"
#include <lwip/altcp.h>
#include <pico/types.h>

#include <cstdint>
#include <memory>
#include <string>

namespace Network::Tcp
{

class TcpServer;

class TcpConnection
{
  public:
    TcpConnection(TcpServer *server, altcp_pcb *pcb);
    ~TcpConnection();
    err_t send(const std::string &data);
    err_t send(const std::string &data, Fs::File file);
    err_t receive(pbuf *p, err_t err);
    err_t sent(uint16_t sent);
    err_t poll();
    void error(err_t err);
    [[nodiscard]] constexpr bool isComplete() const { return m_Complete; }

  private:
    TcpServer *m_Server;
    altcp_pcb *m_TcpPcb;
    std::string m_DataBuffer{};
    std::shared_ptr<Fs::File> m_File{nullptr}; // TODO: ???
    uint16_t m_DataQueued{0};
    uint32_t m_DataSent{0};
    bool m_Complete{false};
    err_t write();
    err_t write(uint16_t startPosition);
    err_t disconnect(err_t close_err);
};

} // namespace Network::Tcp

#endif /* TCPCONNECTION_HPP */
