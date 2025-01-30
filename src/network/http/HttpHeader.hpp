#ifndef HTTPHEADER_HPP
#define HTTPHEADER_HPP

#include <cstdint>
#include <string>

namespace Network::Http
{

class HttpHeader
{
  public:
    HttpHeader(uint16_t status, uint32_t length);
    HttpHeader(const std::string &ip);
    [[nodiscard]] const std::string &str() const;

  private:
    std::string m_Header;
};

} // namespace Network::Http

#endif /* HTTPHEADER_HPP */
