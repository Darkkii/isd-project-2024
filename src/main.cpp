#include "FreeRTOS.h" // IWYU pragma: keep
#include "i2c/PicoI2C.hpp"
#include "queue.h"
#include "task.h"
#include "task/BlinkTask.hpp"
#include "task/I2CTask.h"
#include "uart/PicoOsUart.hpp"
#include <hardware/structs/timer.h>
#include <pico/stdio.h>
#include "sensors/MHZ19CSensor.h"
#include "task/MHZTask.h"
#include "task/MHZTaskParams.h"

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
    stdio_init_all();
    printf("\nBoot\n");

    // Create shared resources
    auto picoI2c0 = std::make_shared<I2c::PicoI2C>(I2c::BUS_0);
    auto picoI2c1 = std::make_shared<I2c::PicoI2C>(I2c::BUS_1);

    // 1) UART for general logging, e.g. pins 0/1 at 115200
    auto logUart = std::make_shared<Uart::PicoOsUart>(0, 0, 1, 115200);

    // 2) UART for the MHZ19C sensor, pins 4/5 at 9600
    auto sensorUart = std::make_shared<Uart::PicoOsUart>(1, 4, 5, 9600, 1);

    // 3) Create I2CTask using logUart for logs
    auto i2cTask = new Task::I2CTask(picoI2c1, logUart);

    // 4) Create MHZTask parameter struct
    auto mhzParams = new MHZTaskParams{
        .sensorUart = sensorUart,
        .logUart    = logUart
    };

    // 5) Create the MHZTask
    xTaskCreate(
        MHZTask,                // The task function
        "MHZTask",              // Task name
        2048,                   // Stack size
        mhzParams,             // Parameter (our struct)
        tskIDLE_PRIORITY + 1,   // Priority
        nullptr                 // No handle needed
    );



   // auto blinkTask = new Task::BlinkTask();


    // Start scheduler
    vTaskStartScheduler();




    while (true) {};

    // Delete task objects, can silence some warnings about unused variables
    //delete blinkTask;
    delete i2cTask;
    delete mhzParams;

    return 0;
}
