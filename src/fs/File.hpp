#ifndef FILE_HPP
#define FILE_HPP

#include <cstdint>
#include <string>

namespace Fs
{

enum Filename
{
    INDEX_HTML,
    SCRIPT_JS
};

class File
{
  public:
    explicit File(Filename filename);
    const std::string &read();
    void rewind();
    [[nodiscard]] constexpr bool eof() const { return m_Eof; };
    [[nodiscard]] constexpr uint32_t size() const { return m_Size; }

  private:
    uint8_t *m_Start;
    uint8_t *m_End;
    uint8_t *m_ReadHead;
    const uint32_t m_Size;
    bool m_Eof{false};
    std::string m_ReadBuffer;
    static uint8_t *indexStart;
    static uint8_t *indexEnd;
    static uint8_t *scriptStart;
    static uint8_t *scriptEnd;
};

} // namespace Fs

#endif /* FILE_HPP */
