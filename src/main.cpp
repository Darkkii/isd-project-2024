#include "FreeRTOS.h" // IWYU pragma: keep
#include "event_groups.h"
#include "i2c/PicoI2C.hpp"
#include "network/NetworkGroup.hpp"
#include "queue.h"
#include "sensor/SensorData.hpp"
#include "task.h"
#include "task/AccessPointTask.hpp"
#include "task/DhcpServerTask.hpp"
#include "task/DnsServerTask.hpp"
#include "task/HttpServerTask.hpp"
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
    const auto ssid = std::make_shared<std::string>("ISD_SENSOR_DATA");
    const auto serverIp = std::make_shared<std::string>("192.168.0.1"); // Server components support /24 networks only.
    const auto netmask = std::make_shared<std::string>("255.255.255.0");

    stdio_init_all();
    printf("\nBoot\n");

    // Create shared resources
    auto picoI2c0 = std::make_shared<I2c::PicoI2C>(I2c::BUS_0);
    auto picoI2c1 = std::make_shared<I2c::PicoI2C>(I2c::BUS_1);
    auto picoUart0 = std::make_shared<Uart::PicoOsUart>(0, 0, 1, 9600);
    auto picoUart1 = std::make_shared<Uart::PicoOsUart>(1, 4, 5, 115200);

    // Event groups
    auto networkGroup = std::make_shared<Network::NetworkGroup>();

    // Create shared data storage
    auto sensorData = std::make_shared<Sensor::SensorData>();

    // Create task objects
    auto apTask = new Task::AccessPointTask(ssid, serverIp, networkGroup);
    auto dhcpTask = new Task::DhcpServerTask(serverIp, netmask, networkGroup);
    auto dnsTask = new Task::DnsServerTask(serverIp, networkGroup);
    auto httpTask = new Task::HttpServerTask(serverIp, sensorData, networkGroup);

    // Start scheduler
    vTaskStartScheduler();

    while (true) {};

    // Delete task objects, can silence some warnings about unused variables
    delete apTask;
    delete dhcpTask;
    delete dnsTask;
    delete httpTask;

    return 0;
}
