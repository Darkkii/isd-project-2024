#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "FreeRTOS.h" // IWYU pragma: keep
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"
#include "rtos/semaphore/Mutex.hpp"

#include <cstdint>
#include <mutex>
#include <queue>
#include <utility>

namespace Rtos::Queue
{

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

        if (m_Storage.size() < N) { m_Storage.push(std::move(element)); }
        return xQueueSend(m_Handle, std::addressof(m_Storage.front()), 0);
    }

    T receive() { return receive(0); }

    T receive(TickType_t ticksToWait)
    {
        std::lock_guard<Semaphore::Mutex> lock(m_Access);

        T *element;
        xQueueReceive(m_Handle, element, ticksToWait);

        return *element;
    }

    T peek() { return peek(0); }

    T peek(uint32_t msWaitTime)
    {
        std::lock_guard<Semaphore::Mutex> lock(m_Access);

        T *element;
        xQueuePeek(m_Handle, element, pdMS_TO_TICKS(msWaitTime));
        return *element;
    }

    void reset()
    {
        std::lock_guard<Semaphore::Mutex> lock(m_Access);
        xQueueReset(m_Handle);
    }

  private:
    Semaphore::Mutex m_Access;
    std::queue<T> m_Storage{};
    xQueueHandle m_Handle{xQueueCreate(N, sizeof(T *))};
    UBaseType_t m_Length{N};
};

} // namespace Rtos::Queue

#endif /* QUEUE_HPP */
