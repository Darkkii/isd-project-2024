#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "FreeRTOS.h" // IWYU pragma: keep
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "rtos/semaphore/Mutex.hpp"

#include <cstdint>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace Rtos
{

// Wrapper for the FreeRTOS queue supporting C++ objects. Multiple producers, single consumer.
template <typename T, size_t N>
class Queue
{
  public:
    Queue() = default;
    Queue(const Queue &) = delete;
    ~Queue()
    {
        if (m_Handle != nullptr)
        {
            vQueueDelete(m_Handle);
            m_Handle = nullptr;
        }
    }

    bool send(T element)
    {
        std::lock_guard<Semaphore::Mutex> lock(m_Access);
        bool success = false;
        if (m_Storage.size() < N && xQueueSend(m_Handle, nullptr, 0))
        {
            m_Storage.emplace(std::move(element));
            success = true;
        }
        return success;
    }

    T receive()
    {
        xQueueReceive(m_Handle, nullptr, portMAX_DELAY);
        std::lock_guard<Semaphore::Mutex> lock(m_Access);
        T element = m_Storage.front();
        m_Storage.pop();
        return element;
    }

    auto receive(uint32_t msTimeout)
    {
        std::optional<T> element = std::nullopt;
        if (xQueueReceive(m_Handle, nullptr, pdMS_TO_TICKS(msTimeout)))
        {
            std::lock_guard<Semaphore::Mutex> lock(m_Access);
            element.emplace(std::move(m_Storage.front()));
            m_Storage.pop();
        }
        return element;
    }

    T &peek()
    {
        xQueuePeek(m_Handle, nullptr, portMAX_DELAY);
        return m_Storage.front();
    }

    auto peek(uint32_t msTimeout)
    {
        return xQueuePeek(m_Handle, nullptr, pdMS_TO_TICKS(msTimeout))
                   ? std::optional<T>{m_Storage.front()}
                   : std::nullopt;
    }

    auto size() { return m_Storage.size(); }

    auto maxSize() { return N; }

    void reset()
    {
        std::lock_guard<Semaphore::Mutex> lock(m_Access);
        while (!m_Storage.empty()) { m_Storage.pop(); }
        xQueueReset(m_Handle);
    }

  private:
    xQueueHandle m_Handle{xQueueCreate(N, 0)};
    Semaphore::Mutex m_Access;
    std::queue<T> m_Storage{};
};

} // namespace Rtos

#endif /* QUEUE_HPP */
