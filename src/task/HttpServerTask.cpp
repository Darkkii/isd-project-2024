#include "HttpServerTask.hpp"

#include "debug/Print.hpp"
#include "fs/File.hpp"
#include "network/NetworkGroup.hpp"
#include "network/http/HttpHeader.hpp"
#include "projdefs.h"
#include <hardware/timer.h>
#include <lwip/api.h>
#include <lwip/err.h>
#include <lwip/netbuf.h>
#include <lwip/tcpip.h>
#include <pico/time.h>

#include <cstdint>
#include <cstdio>
#include <utility>

namespace Task
{

constexpr uint16_t KILOBYTE = 1024;

err_t write(netconn *client, const void *dataptr, size_t size, u8_t apiflags, size_t *bytes_written);

HttpServerTask::HttpServerTask(const std::shared_ptr<std::string> serverIp,
                               std::shared_ptr<Network::NetworkGroup> networkGroup) :
    BaseTask{"HttpServerTask", 768, this, MED},
    m_ServerIp{std::move(serverIp)},
    m_NetworkGroup{std::move(networkGroup)}
{}

void HttpServerTask::run()
{
    constexpr uint16_t TCP_PORT = 80;

    err_t err = ERR_OK;
    netconn *clientConnection;
    netbuf *netBuffer;
    std::string request;

    // We wait until AP, DHCP and DNS tasks are ready before starting HTTP operations
    m_NetworkGroup->wait(Network::AP_DHCP);

    request.resize(KILOBYTE); // Allocate 1kB for requests

    m_ServerConnection = netconn_new(NETCONN_TCP);

    if (m_ServerConnection != nullptr)
    {
        netconn_bind(m_ServerConnection, IP_ANY_TYPE, TCP_PORT);
        netconn_listen(m_ServerConnection);
    }

    Debug::printInfo("HTTP", "Listening on port %u", TCP_PORT);
    m_NetworkGroup->set(Network::HTTP);

    while (true)
    {
        // TODO: add timeout and fetch sensor data from queue?
        err = netconn_accept(m_ServerConnection, &clientConnection);

        if (err == ERR_OK)
        {
            err = netconn_recv(clientConnection, &netBuffer);

            if (err == ERR_OK)
            {
                // Copy the request into the buffer, only copy up to 1kB (-1 for null terminator)
                uint16_t requestSize = netBuffer->p->tot_len < KILOBYTE
                                           ? netBuffer->p->tot_len
                                           : KILOBYTE - 1;
                request.resize(requestSize);
                netbuf_copy_partial(netBuffer, request.data(), request.length(), 0);

                // We only handle GET requests
                if (request.find("GET") != std::string::npos)
                {
                    err = handleRequest(clientConnection, request);
                    if (err != ERR_OK)
                    {
                        Debug::printError("HTTP", "Unable to handle request", err);
                    }
                }

                netbuf_free(netBuffer);
                netbuf_delete(netBuffer);
            }

            netconn_close(clientConnection);
            netconn_delete(clientConnection);
        }
    }
}

err_t HttpServerTask::handleRequest(netconn *client, const std::string &request) const
{
    err_t err = ERR_OK;

    size_t start = request.find_first_of('/');
    size_t end = request.find_first_of("? ", start);
    std::string path{request, start, end - start};
    std::string header;

    Fs::File indexHtml{Fs::INDEX_HTML};
    Fs::File scriptJs{Fs::SCRIPT_JS};

    Debug::printInfo("HTTP", "Received GET request.");

    if (path == "/")
    {
        header.assign(Network::Http::HttpHeader(200, indexHtml.size(), "text/html")
                          .str());
        err = sendResponse(client, header, &indexHtml);
    }
    else if (path == "/script.js")
    {
        header.assign(Network::Http::HttpHeader(200, scriptJs.size(), "text/javascript")
                          .str());
        err = sendResponse(client, header, &scriptJs);
    }
    else if (path == "/data.json")
    {
        // TODO: handle data path
        std::string data{R"([
  {
    "name": {
      "ger": "Temperatur",
      "fin": "Lämpötila",
      "en": "Temperature"
    },
    "value": "20.6",
    "unit": "°C"
  },
  {
    "name": {
      "ger": "Luftfeuchtigkeit",
      "fin": "Ilmankosteus",
      "en": "Humidity"
    },
    "value": "45",
    "unit": "%"
  },
  {
    "name": {
      "ger": "Luftdruck",
      "fin": "Ilmanpaine",
      "en": "Pressure"
    },
    "value": "1013",
    "unit": "hPa"
  }
])"};
        header.assign(Network::Http::HttpHeader(200, data.size(), "application/json")
                          .str());
        err = sendResponse(client, header, data);
    }
    else if (path == "/favicon.ico")
    {
        header.assign(Network::Http::HttpHeader(200, 0, "text/plain").str());
    }
    else
    {
        // Redirect all other paths to root
        header.assign(Network::Http::HttpHeader(*m_ServerIp).str());
        err = sendResponse(client, header);

        Debug::printInfo("HTTP", "Redirecting client to %s/", m_ServerIp->c_str());
    }

    Debug::printInfo("HTTP", "Sent response for path: %s", path.c_str());

    return err;
}

err_t HttpServerTask::sendResponse(netconn *client, std::string &header, Fs::File *file) const
{
    err_t err = ERR_OK;
    size_t totalSent = 0;
    size_t sent = 0;

    err = write(client, header.c_str(), header.length(), NETCONN_COPY, &sent);

    if (err == ERR_OK && file != nullptr)
    {
        size_t dataToSend;

        while (err == ERR_OK && totalSent < file->size())
        {
            // Send the file in up to 1kB chunks to the TCP to prevent errors
            dataToSend = (file->size() - totalSent) < KILOBYTE
                             ? (file->size() - totalSent)
                             : KILOBYTE;

            err = write(client, file->begin() + totalSent, dataToSend, NETCONN_COPY, &sent);

            while (dataToSend != sent) { vTaskDelay(pdMS_TO_TICKS(1)); }

            totalSent += sent;
        }
    }

    return err;
}

err_t HttpServerTask::sendResponse(netconn *client, std::string &header, std::string &data) const
{
    err_t err = ERR_OK;
    size_t totalSent = 0;
    size_t sent = 0;

    err = write(client, header.c_str(), header.length(), NETCONN_COPY, &sent);

    if (err == ERR_OK)
    {
        size_t dataToSend;

        while (err == ERR_OK && totalSent < data.size())
        {
            // Send the file in up to 1kB chunks to the TCP to prevent errors
            dataToSend = (data.size() - totalSent) < KILOBYTE
                             ? (data.size() - totalSent)
                             : KILOBYTE;

            err = write(client, data.data() + totalSent, dataToSend, NETCONN_COPY, &sent);

            while (dataToSend != sent) { vTaskDelay(pdMS_TO_TICKS(1)); }

            totalSent += sent;
        }
    }

    return err;
}

err_t write(netconn *client, const void *dataptr, size_t size, u8_t apiflags, size_t *bytes_written)
{
    err_t err = ERR_INPROGRESS;

    for (uint8_t i = 0; err != ERR_OK && i < 3; ++i)
    {
        err = netconn_write_partly(client, dataptr, size, apiflags, bytes_written);
    }

    if (err != ERR_OK)
    {
        Debug::printError("HTTP", "Failed to write data for TCP sending.", err);
    }

    return err;
}

} // namespace Task