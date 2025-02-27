#include "EventGroup.hpp"

#include <cstdint>

namespace Rtos
{

EventGroup::~EventGroup()
{
    if (m_Handle != nullptr)
    {
        vEventGroupDelete(m_Handle);
        m_Handle = nullptr;
    }
}

void EventGroup::set(EventBits_t bits)
{
    if (m_Handle != nullptr) { xEventGroupSetBits(m_Handle, bits); }
}

void EventGroup::wait(EventBits_t bits, bool clearOnExit, bool waitForAll, uint32_t msWait)
{
    if (m_Handle != nullptr)
    {
        xEventGroupWaitBits(m_Handle, bits, clearOnExit, waitForAll, msWait);
    }
}

void EventGroup::clear(EventBits_t bits)
{
    if (m_Handle != nullptr) { xEventGroupClearBits(m_Handle, bits); }
}

} // namespace Rtos