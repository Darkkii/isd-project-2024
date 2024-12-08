//
// Created by Carina Schleiwies on 05.12.2024.
//

#include "I2CTask.h"

#include "pico/binary_info.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "projdefs.h"
#include "sensors/SensDust.h"
#include <hardware/i2c.h>
#include <pico/error.h>

#include <cstdio>
#include <utility>

namespace Task
{

I2CTask::I2CTask(std::shared_ptr<I2c::PicoI2C> i2cDevice, std::shared_ptr<Uart::PicoOsUart> uartDevice) : BaseTask{"I2CTask", 256, this, HIGH} {
    this->i2cDevice = std::move(i2cDevice);
    this->uartDevice = std::move(uartDevice);
}

void I2CTask::run()
{
    int rc = cyw43_arch_init();

    uartDevice->send("\nI2C Bus Scan\n");
    uartDevice->send("\n   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n\n");
    char addrData[10];
    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            sprintf(addrData, "%02x ", addr);
            uartDevice->send(addrData);
        }

        int ret;
        uint8_t rxdata;
        ret = i2cDevice->read(addr, &rxdata, 1);
        if (ret == 0)
        {
            if (addr == 0x08) { uartDevice->send("Sensor Gas SENS V2 not connected"); }
            if (addr == 0x3c) { uartDevice->send("Display not connected"); }
            if (addr == 0x40) { uartDevice->send("Sensor SENS DUST not connected"); }
            if (addr == 0x71) { uartDevice->send("Sensor MS430 not connected"); }
        }
        uartDevice->send(ret <= 0 ? "." : "@");
        uartDevice->send(addr % 16 == 15 ? "\n" : "  ");
    }
    uartDevice->send("Done.\n");

    while (true)
    {
        if (rc == PICO_OK)
        {
            //SensDust
            uint8_t value = 0x88;
            uint ret = i2cDevice->write(0x40, &value, 1);
            uartDevice->send(ret != 1 ? "Write error for SENS DUST\n" : "");
            vTaskDelay(pdMS_TO_TICKS(100));
            uint8_t sensDustData[30];
            ret = i2cDevice->read(0x40, &sensDustData[0], 29);
            uartDevice->send(ret != 29 ? "Read error for SENS DUST\n" : "");
            SensDust sensDustDecoded = SensDust(sensDustData);
            if(sensDustDecoded.valid){
                uartDevice->send(sensDustDecoded.toString());
            }else{
                uartDevice->send("Data error for SENS DUST\n");
            }


            //TODO: 0x71 Combi-Sensor
            //TODO: 0x08 Gas Sens V2

            //TODO: Bonus: Display

        }
    }
}

} // namespace Task
