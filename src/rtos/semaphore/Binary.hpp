#ifndef BINARY_HPP
#define BINARY_HPP

#include "FreeRTOS.h" // IWYU pragma: keep
#include "semphr.h"

namespace Rtos::Semaphore
{

class Binary
{
  public:
    Binary();
    virtual ~Binary();
    void take();
    void give();

  private:
    SemaphoreHandle_t m_Semaphore;
};

} // namespace Rtos::Semaphore

#endif /* BINARY_HPP */
