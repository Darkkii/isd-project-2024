#include "FreeRTOS.h" // IWYU pragma: keep
#include "i2c/PicoI2C.hpp"
#include "queue.h"
#include "task.h"
#include "task/NetworkTask.hpp"
#include "uart/PicoOsUart.hpp"
#include <hardware/structs/timer.h>
#include <pico/stdio.h>

#include <cstdio>
#include <memory>

extern "C"
{
    uint32_t read_runtime_ctr(void) { return timer_hw->timerawl; }

    // "Syscall stub" to silence linker warning from some versions of arm-none-eabi-gcc
    int getentropy(void *buffer, size_t length) { return -ENOSYS; }
}

int main()
{
    const std::string network = "192.168.0.0"; // Server components support /24 networks only. Server address is x.x.x.1.

    stdio_init_all();
    printf("\nBoot\n");

    // Create shared resources
    auto picoI2c0 = std::make_shared<I2c::PicoI2C>(I2c::BUS_0);
    auto picoI2c1 = std::make_shared<I2c::PicoI2C>(I2c::BUS_1);
    auto picoUart0 = std::make_shared<Uart::PicoOsUart>(0, 0, 1, 9600);
    auto picoUart1 = std::make_shared<Uart::PicoOsUart>(1, 4, 5, 115200);

    // Create queues

    // Create task objects
    auto networkTask = new Task::NetworkTask(network);

    // Start scheduler
    vTaskStartScheduler();

    while (true) {};

    // Delete task objects, can silence some warnings about unused variables
    delete networkTask;

    return 0;
}
