#include "HttpServer.hpp"

#include "fs/File.hpp"
#include "network/http/HttpHeader.hpp"
#include "network/tcp/TcpCallback.hpp"
#include "network/tcp/TcpConnection.hpp"
#include <lwip/altcp.h>
#include <lwip/altcp_tcp.h>
#include <lwip/err.h>
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <lwip/tcp.h>
#include <pico/error.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

namespace Network::Http
{
HttpServer::HttpServer(const std::shared_ptr<std::string> serverIp)
{
    constexpr uint8_t TCP_PORT = 80;

    ip4addr_aton(serverIp->c_str(), ip_2_ip4(&m_Ip));

    printf("HTTP server on port %u\n", TCP_PORT);

    struct altcp_pcb *pcb = altcp_tcp_new_ip_type(IPADDR_TYPE_ANY);

    if (pcb != nullptr)
    {
        altcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
        m_TcpPcb = altcp_listen_with_backlog(pcb, 1);

        if (m_TcpPcb == nullptr)
        {
            printf("Failed to listen.");
            if (pcb != nullptr) { altcp_close(pcb); }
        }
        else
        {
            altcp_arg(m_TcpPcb, this);
            altcp_accept(m_TcpPcb, Tcp::tcpAccept);

            printf("Try connecting to '%s'\n", "ISD_SENSOR_DATA");
        }
    }
}

HttpServer::~HttpServer()
{
    if (m_TcpPcb != nullptr)
    {
        altcp_arg(m_TcpPcb, nullptr);
        if (altcp_close(m_TcpPcb) != ERR_OK) { altcp_abort(m_TcpPcb); };
        m_TcpPcb = nullptr;
    }
}

err_t HttpServer::accept(altcp_pcb *clientPcb, err_t err)
{
    if (err == ERR_OK && clientPcb != nullptr)
    {
        removeDisconnected();
        m_Connections.emplace_back(&(*this), clientPcb);
        printf("Client connected.\n");
    }
    else { printf("[Error %d] Failed to accept connection.\n", err); }

    return err;
}

err_t HttpServer::respond(Tcp::TcpConnection &connection, pbuf *p, err_t err) const
{
    std::string request;

    // Copy the request into the buffer
    request.resize(p->tot_len);
    pbuf_copy_partial(p, request.data(), request.length(), 0);

    // We only handle GET requests
    if (request.find("GET") != std::string::npos)
    {
        err = handleRequest(connection, request);
    }

    return ERR_OK;
}

err_t HttpServer::handleRequest(Tcp::TcpConnection &connection,
                                const std::string &request) const
{
    size_t start = request.find_first_of('/');
    size_t end = request.find_first_of("? ", start);
    std::string path{request, start, end - start};
    std::string response;

    if (path == "/")
    {
        Fs::File index(Fs::INDEX_HTML);
        response.assign(HttpHeader(200, index.size()).get());
        connection.send(response, index);
    }
    else if (path == "/script.js")
    {
        Fs::File script(Fs::SCRIPT_JS);
        response.assign(HttpHeader(200, script.size()).get());
        connection.send(response, script);
    }
    // else if (path == "/data.json") {} // TODO: handle data path
    else
    {
        // Redirect all other paths to root
        std::string ip{ipaddr_ntoa(&m_Ip)};
        response.assign(HttpHeader(ip).get());
        connection.send(response);
        printf("Redirecting client to %s/\n", ip.c_str());
    }

    printf("Response for path: %s\n", path.c_str());

    return ERR_OK;
}

void HttpServer::removeDisconnected()
{
    // Remove finished connections from vector
    m_Connections.erase(std::remove_if(m_Connections.begin(),
                                       m_Connections.end(),
                                       [](auto &conn) -> bool
                                       { return conn.isComplete(); }),
                        m_Connections.end());
}

} // namespace Network::Http