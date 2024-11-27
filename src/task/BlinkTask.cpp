#include "BlinkTask.hpp"

#include "pico/cyw43_arch.h"
#include "projdefs.h"
#include <pico/error.h>

namespace Task
{

BlinkTask::BlinkTask() : BaseTask{"BlinkTask", 256, this, MED} {}

void BlinkTask::run()
{
    int rc = cyw43_arch_init();

    while (true)
    {
        if (rc == PICO_OK)
        {
            // Blinks LED on the board, 100ms on, 2400 ms off
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN,
                                !cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN));
            vTaskDelay(pdMS_TO_TICKS(cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN) ? 100 : 2400));
        }
    }
}

} // namespace Task
