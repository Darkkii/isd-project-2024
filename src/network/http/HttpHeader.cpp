#include "network/http/HttpHeader.hpp"

namespace Network::Http
{
HttpHeader::HttpHeader(uint16_t status, uint32_t length, std::string contentType) :
    m_Header{"HTTP/1.1 " + std::to_string(status) + " OK\nContent-Length: "
             + std::to_string(length) + "\nContent-Type: " + contentType
             + "; charset=utf-8\nConnection: close\n\n"}
{}

HttpHeader::HttpHeader(const std::string &ip) :
    m_Header{"HTTP/1.1 302 Redirect\nLocation: http://" + ip + "/\n\n"}
{}

const std::string &HttpHeader::str() const { return m_Header; }

} // namespace Network::Http