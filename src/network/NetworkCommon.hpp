#ifndef NETWORKCOMMON_HPP
#define NETWORKCOMMON_HPP

#include <lwip/netbuf.h>
#include <type_traits>

#include <array>
#include <cstdint>
#include <vector>

namespace Network
{

template <typename>
struct isArray : std::false_type
{};

template <typename T, std::size_t N>
struct isArray<std::array<T, N>> : std::true_type
{};

template <typename>
struct isVector : std::false_type
{};

template <typename T>
struct isVector<std::vector<T>> : std::true_type
{};

template <typename T>
void deserialize(netbuf *netBuffer, T &value, uint16_t &offset)
{
    if constexpr (std::is_arithmetic_v<T>)
    {
        netbuf_copy_partial(netBuffer, std::addressof(value), sizeof(value), offset);
        offset += sizeof(value);
    }
    else if constexpr (isArray<T>::value)
    {
        netbuf_copy_partial(netBuffer, value.data(), value.size(), offset);
        offset += value.size();
    }
    else if constexpr (isVector<T>::value)
    {
        value.resize(netBuffer->p->tot_len - offset);
        netbuf_copy_partial(netBuffer, value.data(), value.size(), offset);
        offset += value.size();
    }
};

} // namespace Network

#endif /* NETWORKCOMMON_HPP */
