#ifndef EVENTGROUP_HPP
#define EVENTGROUP_HPP

#include "FreeRTOS.h" // IWYU pragma: keep
#include "event_groups.h"

namespace Rtos
{

class EventGroup
{
  public:
    EventGroup() = default;
    EventGroup(const EventGroup &) = delete;
    ~EventGroup();
    void set(EventBits_t bits);
    void wait(EventBits_t bits, bool clearOnExit, bool waitForAll, uint32_t msWait);
    void clear(EventBits_t bits);

  private:
    EventGroupHandle_t m_Handle{xEventGroupCreate()};
};

} // namespace Rtos

#endif /* EVENTGROUP_HPP */
