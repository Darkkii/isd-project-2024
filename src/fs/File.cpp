#include "File.hpp"

#include <cstdint>

// Variables for start and end of files included in the binary
extern uint8_t fs_index_start, fs_index_end, fs_script_start, fs_script_end;

namespace Fs
{

// Pointers to start and end of index.html
uint8_t *File::indexStart = &fs_index_start;
uint8_t *File::indexEnd = &fs_index_end;

// Pointers to start and end of script.js
uint8_t *File::scriptStart = &fs_script_start;
uint8_t *File::scriptEnd = &fs_script_end;

File::File(Filename filename) :
    m_Start{filename ? scriptStart : indexStart},
    m_End{filename ? scriptEnd : indexEnd},
    m_ReadHead{m_Start},
    m_Size{static_cast<uint32_t>(&m_End - &m_Start)}
{}

const std::string &File::read()
{
    constexpr uint16_t READ_LENGTH = 256; // Read in up to 256 byte chunks to minimize memory use

    if (!m_Eof)
    {
        uint8_t *readEnd = m_ReadHead + READ_LENGTH < m_End ? m_ReadHead + READ_LENGTH
                                                            : m_End;

        m_ReadBuffer.assign(m_ReadHead, readEnd);

        m_ReadHead = readEnd;
        if (m_ReadHead >= m_End) { m_Eof = true; }
    }
    else { m_ReadBuffer.clear(); }

    return m_ReadBuffer;
};

void File::rewind()
{
    m_ReadHead = m_Start;
    m_ReadBuffer.clear();
    m_Eof = false;
};
} // namespace Fs