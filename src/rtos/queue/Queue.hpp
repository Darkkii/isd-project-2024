#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "FreeRTOS.h" // IWYU pragma: keep
#include "portmacro.h"
#include "projdefs.h"
#include "queue.h"

#include <cstdint>

/*
 *   *** WARNING ***
 *   This is a quick and dirty implementation of a C++ wrapper around the FreeRTOS queue.
 *   Care should be taken with the types used with this template as FreeRTOS
 *   queues might not understand all C++ object correctly.
 *   While pointers can be used to work around this issue, but care should be
 *   taken to make sure the pointers remain valid while they remain in the queue.
 */

namespace Rtos::Queue
{

template <typename T, UBaseType_t N>
class Queue
{
  public:
    Queue<T, N>();
    ~Queue<T, N>()
    {
        if (m_Handle != nullptr)
        {
            vQueueDelete(m_Handle);
            m_Handle = nullptr;
        }
    }

    bool send(T *element) { return xQueueSend(m_Handle, element, 0); }
    T receive() { return receive(0); }
    T receive(TickType_t ticksToWait)
    {
        T element;
        xQueueReceive(m_Handle, &element, ticksToWait);
    }
    T peek() { return peek(0); }
    T peek(uint32_t msWaitTime)
    {
        T element;
        xQueuePeek(m_Handle, &element, pdMS_TO_TICKS(msWaitTime));
    }

    void reset() { xQueueReset(m_Handle); }

  private:
    xQueueHandle m_Handle{xQueueCreate(N, sizeof(T))};
    UBaseType_t m_Length{N};
};

} // namespace Rtos::Queue

#endif /* QUEUE_HPP */
