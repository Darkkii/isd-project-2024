#ifndef MUTEX_HPP
#define MUTEX_HPP

#include "FreeRTOS.h" // IWYU pragma: keep
#include "semphr.h"

namespace Rtos::Semaphore
{
class Mutex
{
  public:
    Mutex();
    ~Mutex();
    void lock();
    bool try_lock();
    void unlock();

  private:
    SemaphoreHandle_t m_Semaphore;
};

} // namespace Rtos::Semaphore

#endif /* MUTEX_HPP */
