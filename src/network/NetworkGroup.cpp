#include "NetworkGroup.hpp"

#include "portmacro.h"

namespace Network
{

void NetworkGroup::set(NetworkBits bit) { Rtos::EventGroup::set(bit); }

void NetworkGroup::wait(NetworkBits bits)
{
    Rtos::EventGroup::wait(bits, false, true, portMAX_DELAY);
}

} // namespace Network