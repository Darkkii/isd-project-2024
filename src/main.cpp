#include "FreeRTOS.h" // IWYU pragma: keep
#include "event_groups.h"
#include "i2c/PicoI2C.hpp"
#include "network/NetworkGroup.hpp"
#include "pico/stdlib.h" // IWYU pragma: keep
#include "queue.h"
#include "sensor/MS430.h"
#include "sensor/SensorData.hpp"
#include "task.h"
#include "task/AccessPointTask.hpp"
#include "task/DhcpServerTask.hpp"
#include "task/DnsServerTask.hpp"
#include "task/HttpServerTask.hpp"
#include "task/I2CTask.h"
#include "task/MHZTask.h"
#include "task/MHZTaskParams.h"
#include "uart/PicoOsUart.hpp"
#include <hardware/structs/timer.h>
#include <pico/stdio.h>

#include <cstdio>
#include <memory>

volatile uint8_t newDataOnMS430 = 0;

void newMS430Data(uint gpio, uint32_t events)
{
    if (gpio == RDY_PIN) { newDataOnMS430 = 1; }
}

extern "C"
{
    uint32_t read_runtime_ctr(void) { return timer_hw->timerawl; }

    // "Syscall stub" to silence linker warning from some versions of arm-none-eabi-gcc
    int getentropy(void *buffer, size_t length) { return -ENOSYS; }
}

int main()
{
    const auto ssid = std::make_shared<std::string>("ISD_SENSOR_DATA");
    const auto serverIp = std::make_shared<std::string>("192.168.0.1"); // Server components support /24 networks only.
    const auto netmask = std::make_shared<std::string>("255.255.255.0");

    stdio_init_all();
    printf("\nBoot\n");

    int rdy = RDY_PIN;

    gpio_init(rdy);
    gpio_pull_up(rdy);
    gpio_set_function(rdy, GPIO_FUNC_NULL);
    gpio_set_irq_enabled_with_callback(RDY_PIN, GPIO_IRQ_EDGE_FALL, true, newMS430Data);

    // Create shared resources
    auto picoI2c0 = std::make_shared<I2c::PicoI2C>(I2c::BUS_0);
    auto picoI2c1 = std::make_shared<I2c::PicoI2C>(I2c::BUS_1);

    // 1) UART for general logging, e.g. pins 0/1 at 115200
    auto logUart = std::make_shared<Uart::PicoOsUart>(0, 0, 1, 115200);

    // 2) UART for the MHZ19C sensor, pins 4/5 at 9600
    auto sensorUart = std::make_shared<Uart::PicoOsUart>(1, 4, 5, 9600, 1);

    // Event groups
    auto networkGroup = std::make_shared<Network::NetworkGroup>();

    // Create shared data storage
    auto sensorData = std::make_shared<Sensor::SensorData>();

    // Create task objects
    auto apTask = new Task::AccessPointTask(ssid, serverIp, networkGroup);
    auto dhcpTask = new Task::DhcpServerTask(serverIp, netmask, networkGroup);
    auto dnsTask = new Task::DnsServerTask(serverIp, networkGroup);
    auto httpTask = new Task::HttpServerTask(serverIp, sensorData, networkGroup);

    // 3) Create I2CTask using logUart for logs
    auto i2cTask = new Task::I2CTask(picoI2c1, logUart, sensorData);

    // 4) Create MHZTask parameter struct
    auto mhzParams = new MHZTaskParams{
        .sensorUart = sensorUart,
        .logUart = logUart,
        .sensorData = sensorData,
    };

    // 5) Create the MHZTask
    xTaskCreate(MHZTask,                      // The task function
                "MHZTask",                    // Task name
                2048,                         // Stack size
                mhzParams,                    // Parameter (our struct)
                tskIDLE_PRIORITY + Task::LOW, // Priority
                nullptr                       // No handle needed
    );

    // Start scheduler
    vTaskStartScheduler();

    while (true) {};

    // Delete task objects, can silence some warnings about unused variables
    delete apTask;
    delete dhcpTask;
    delete dnsTask;
    delete httpTask;
    delete i2cTask;
    delete mhzParams;

    return 0;
}
