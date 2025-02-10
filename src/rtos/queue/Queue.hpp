#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "FreeRTOS.h" // IWYU pragma: keep
#include "portmacro.h"
#include "queue.h"
#include "rtos/semaphore/Mutex.hpp"

#include <mutex>
#include <queue>
#include <utility>

namespace Rtos::Queue
{

// Wrapper for the FreeRTOS queue. Only blocking receive/peek are supported.
template <typename T, UBaseType_t N>
class Queue
{
  public:
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
        if (m_Storage.size() < N) { m_Storage.emplace(std::move(element)); }
        return xQueueSend(m_Handle, nullptr, 0);
    }

    T receive()
    {
        std::lock_guard<Semaphore::Mutex> lock(m_Access);
        xQueueReceive(m_Handle, nullptr, portMAX_DELAY);
        T element = m_Storage.front();
        m_Storage.pop();
        return element;
    }

    T peek()
    {
        std::lock_guard<Semaphore::Mutex> lock(m_Access);
        xQueuePeek(m_Handle, nullptr, portMAX_DELAY);
        T element = m_Storage.front();
        m_Storage.pop();
        return element;
    }

    void reset()
    {
        std::lock_guard<Semaphore::Mutex> lock(m_Access);
        xQueueReset(m_Handle);
    }

  private:
    Semaphore::Mutex m_Access;
    std::queue<T> m_Storage{};
    xQueueHandle m_Handle{xQueueCreate(N, 0)};
    UBaseType_t m_Length{N};
};

} // namespace Rtos::Queue

#endif /* QUEUE_HPP */
