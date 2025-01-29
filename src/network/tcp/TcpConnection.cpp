#include "TcpConnection.hpp"

#include "FreeRTOS.h" // IWYU pragma: keep
#include "fs/File.hpp"
#include "network/tcp/TcpCallback.hpp"
#include "network/tcp/TcpServer.hpp"
#include "task.h"
#include <hardware/timer.h>
#include <lwip/altcp.h>
#include <lwip/err.h>
#include <pico/time.h>

#include <cstdint>
#include <cstdio>
#include <memory>

namespace Network::Tcp
{

TcpConnection::TcpConnection(TcpServer *server, altcp_pcb *pcb) :
    m_Server{server},
    m_TcpPcb{pcb}
{
    altcp_arg(m_TcpPcb, this);
    altcp_sent(m_TcpPcb, Tcp::tcpSent);
    altcp_recv(m_TcpPcb, Tcp::tcpReceive);
    altcp_poll(m_TcpPcb, Tcp::tcpPoll, 10);
    altcp_err(m_TcpPcb, Tcp::tcpError);
}

TcpConnection::~TcpConnection()
{
    if (m_TcpPcb != nullptr) { disconnect(ERR_OK); }
}

err_t TcpConnection::send(const std::string &data)
{
    // Copy data to buffer and write to TCP send queue
    m_DataBuffer = data;
    m_DataQueued = data.length();
    return write();
}

err_t TcpConnection::send(const std::string &data, Fs::File file)
{
    // Create a file pointer and copy first part of data for sending
    m_File = std::make_shared<Fs::File>(file);
    m_DataBuffer = data + m_File->read();
    m_DataQueued = data.length() + m_File->size();
    return write();
}

err_t TcpConnection::receive(pbuf *p, err_t err)
{
    if (!p) { return disconnect(ERR_OK); }

    if (p->tot_len > 0)
    {
        printf("TCP data received: %d bytes\n", p->tot_len);
        err = m_Server->respond(*this, p, err);
        altcp_recved(m_TcpPcb, p->tot_len);
    }

    pbuf_free(p);

    return err;
}

err_t TcpConnection::sent(uint16_t sent)
{
    m_DataSent += sent;

    if (m_DataSent >= m_DataQueued) { disconnect(ERR_OK); }
    else
    {
        m_DataBuffer.erase(0, sent);
        if (m_File != nullptr) { m_DataBuffer.append(m_File->read()); }
        write();
    }

    return ERR_OK;
}

err_t TcpConnection::poll()
{
    if (isComplete()) { disconnect(ERR_OK); }
    return ERR_OK;
}

void TcpConnection::error(err_t err)
{
    if (err != ERR_ABRT)
    {
        printf("[Error %d] - TCP client error.\n", err);
        disconnect(err);
    }
}

err_t TcpConnection::write()
{
    // Calculate the amount of data that can fit in the send buffer
    // uint16_t dataLength = (m_DataBuffer.length()) < (altcp_sndbuf(m_TcpPcb))
    //                           ? m_DataBuffer.length()
    //                           : altcp_sndbuf(m_TcpPcb) - 1;

    err_t err = altcp_write(m_TcpPcb, m_DataBuffer.data(), m_DataBuffer.length(), 0);

    if (err != ERR_OK)
    {
        printf("[Error %d] - Failed to write data for sending\n", err);
        err = disconnect(err);
    }

    return err;
}
err_t TcpConnection::disconnect(err_t close_err)
{
    altcp_arg(m_TcpPcb, nullptr);
    altcp_poll(m_TcpPcb, nullptr, 0);
    altcp_sent(m_TcpPcb, nullptr);
    altcp_recv(m_TcpPcb, nullptr);
    altcp_err(m_TcpPcb, nullptr);
    err_t err = altcp_close(m_TcpPcb);

    if (err != ERR_OK)
    {
        printf("[Error %d] - Close failed,  calling abort.\n", err);
        altcp_abort(m_TcpPcb);
        close_err = ERR_ABRT;
    }

    m_Complete = true;
    m_TcpPcb = nullptr;

    printf("Connection closed\n");
    return close_err;
}

} // namespace Network::Tcp