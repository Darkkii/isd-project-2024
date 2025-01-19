//
// Created by Carina Schleiwies on 05.12.2024.
//

#include "I2CTask.h"

#include "Display-lib/SSD1306_OLED.hpp"
#include "pico/binary_info.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "projdefs.h"
#include "sensors/MS430.h"
#include "sensors/RTCModule.h"
#include "sensors/RTClib.h"
#include "sensors/SPS30.h"
#include "sensors/SensDust.h"
#include <hardware/i2c.h>
#include <hardware/structs/io_qspi.h>
#include <pico/error.h>

#include <chrono>
#include <cstdio>
#include <utility>

#define FULLSCREEN (128 * (32/8))

volatile uint8_t newDataOnMS430 = 0;

void newMS430Data(uint gpio, uint32_t events){
    if(gpio == RDY_PIN) { newDataOnMS430 = 1; }
}

namespace Task
{

I2CTask::I2CTask(std::shared_ptr<I2c::PicoI2C> i2cDevice, std::shared_ptr<Uart::PicoOsUart> uartDevice) : BaseTask{"I2CTask", 1024, this, HIGH} {
    this->i2cDevice = std::move(i2cDevice);
    this->uartDevice = std::move(uartDevice);
    int rdy = RDY_PIN;

    gpio_init(rdy);
    gpio_pull_up(rdy);
    gpio_set_function(rdy, GPIO_FUNC_NULL);
    gpio_set_irq_enabled_with_callback(RDY_PIN, GPIO_IRQ_EDGE_FALL, true, newMS430Data);
}


void I2CTask::run()
{
    int rc = cyw43_arch_init();
    if (rc == PICO_OK)
    {
        uartDevice->send("\r\nI2C Bus Scan\r\n");
        uartDevice->send("\r\n   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n\n");
        char addrData[10];
        for (int addr = 0; addr < (1 << 7); ++addr) {
            if (addr % 16 == 0) {
                sprintf(addrData, "%02x ", addr);
                uartDevice->send(addrData);
            }

            uint ret;
            uint8_t rxdata;
            ret = i2cDevice->read(addr, &rxdata, 1);
            if (ret == 0)
            {
                if (addr == 0x3c) { uartDevice->send("Display not connected"); }
                // if (addr == 0x40) { uartDevice->send("Sensor SENS DUST not connected"); }
                if (addr == 0x68) { uartDevice->send("RTC not connected"); }
                if (addr == 0x69) { uartDevice->send("SPS30 not connected"); }
                if (addr == 0x71) { uartDevice->send("Sensor MS430 not connected"); }
            }
            uartDevice->send(ret <= 0 ? "." : "@");
            uartDevice->send(addr % 16 == 15 ? "\r\n" : "  ");
        }
        uartDevice->send("Done.\r\n");


        auto display = SSD1306(128,32, i2cDevice);
        display.OLEDbegin();
        display.OLEDFillScreen(0xF0, 0); // splash screen bars, optional just for effect
        vTaskDelay(pdMS_TO_TICKS(1000));
        uint8_t  screenBuffer[FULLSCREEN];
        if (!display.OLEDSetBufferPtr(128, 32, screenBuffer, sizeof(screenBuffer))) return;
        display.OLEDclearBuffer();
        display.setTextColor(WHITE);
        display.setCursor(10, 10);
        display.print("I2C TEST");
        display.OLEDupdate();


        auto rtc = RTCModule(i2cDevice, uartDevice);
      //  rtc.setDateTime(DateTime(2024,12,22,2,7,20)); //FIXME: Find way to always have accurate time
        std::string str = "DDD, DD MMM YYYY hh:mm:ss\r\n";
        char dateFormat[ str.length() + 1];
        strcpy(dateFormat, str.c_str());
        std::string str2 = "DD.MM.YY hh:mm:ss\r\n";
        char dateFormat2[str2.length() + 1];
        strcpy(dateFormat2, str2.c_str());


        //Setup MS430
        auto ms430 = MS430(i2cDevice);
        uartDevice->send(ms430.reset() != 0 ? "Write error Reset for Combi-Sensor\n" : "");
        uartDevice->send(ms430.configCycleMode() != 0 ? "Write error configCycleMode for Combi-Sensor\n" : "");
        vTaskDelay(pdMS_TO_TICKS(10));
        uartDevice->send(ms430.startCycleMode() != 0 ? "Write error CycleMode for Combi-Sensor\n" : "");
        vTaskDelay(pdMS_TO_TICKS(10));
        newDataOnMS430 = 0;

        //SPS30
        //start Measurement
        //Pointer: 0x0010, 0x05 -> int, 0x00, checksum von 0x05 und 0x00
        uint8_t sps30Data[2] = {0x05, 0x00};
        uint8_t sps30StartMeasurement[5] = {0x00, 0x10, 0x05, 0x00, calcCrc(sps30Data)};
        uint ret =  i2cDevice->write(0x69, &sps30StartMeasurement[0], 5);
        uartDevice->send(ret != 5 ? "Write error for SPS30\n" : "");
        vTaskDelay(pdMS_TO_TICKS(25));
        uint8_t sps30Meas[30];

        while (true)
        {
            //SensDust
            //            uint8_t value = 0x88;
            //            uint ret = i2cDevice->write(0x40, &value, 1);
            //            uartDevice->send(ret != 1 ? "Write error for SENS DUST\r\n" : "");
            //            vTaskDelay(pdMS_TO_TICKS(100));
            //            uint8_t sensDustData[30];
            //            ret = i2cDevice->read(0x40, &sensDustData[0], 29);
            //            uartDevice->send(ret != 29 ? "Read error for SENS DUST\r\n" : "");
            //            SensDust sensDustDecoded = SensDust(sensDustData);
            //            if(sensDustDecoded.valid){
            //                uartDevice->send(sensDustDecoded.toString());
            //            }else{
            //                uartDevice->send("Data error for SENS DUST\r\n");
            //            }


            //0x71 Combi-Sensor
            if(newDataOnMS430 == 1)
            {
                uartDevice->send(ms430.updateEnvironmentData() != 0 ? "Data read error for Combi-Sensor\n" : "");
                uartDevice->send(ms430.updateAirQualityData() != 0 ? "Data read error1 for Combi-Sensor\n" : "");
                uartDevice->send(ms430.updateLightData() != 0      ? "Data read error2 for Combi-Sensor\n" : "");
                uartDevice->send(ms430.updateSoundData() != 0      ? "Data read error3 for Combi-Sensor\n" : "");
                uartDevice->send(ms430.toString());
                uartDevice->send("Done Reading MS430\r\n");
                newDataOnMS430 = 0;
            }
            //RTC-Modul
            DateTime dateTime =  rtc.getDateTime();
            uartDevice->send(dateTime.toString(dateFormat));
            vTaskDelay(pdMS_TO_TICKS(1000));

            //Bonus SPS30 I2c -> 0x69
//            uint8_t sps30Rdy[3];
//            //Read data ready flag
//            sps30Data[0] = 0x02;
//            sps30Data[1] = 0x02;
            //Pointer: 0x0300, 2 data, checksum ... int insgesamt: 30
//            ret =  i2cDevice->write(0x69, &sps30Data[0], 2);
//            uartDevice->send(ret != 2 ? "Write error0 for SPS30\n" : "");
//            ret =  i2cDevice->read(0x69, &sps30Rdy[0], 3);
//            uartDevice->send(ret != 3 ? "Read error0 for SPS30\n" : "");
//            uint8_t check = calcCrc(sps30Rdy);
//            uartDevice->send(check != sps30Rdy[2] ? "Read error1 for SPS30\n" : "");

            //Read Measured Values
            sps30Data[0] = 0x03;
            sps30Data[1] = 0x00;
            //Pointer: 0x0300, 2 data, checksum ... int insgesamt: 30
            ret =  i2cDevice->write(0x69, &sps30Data[0], 2);
            uartDevice->send(ret != 2 ? "Write error1 for SPS30\n" : "");
            ret =  i2cDevice->read(0x69,  &sps30Meas[0], 30);
            uartDevice->send(ret != 30 ? "Read error1 for SPS30\n" : "");

            auto sps30Decoded = SPS30<uint16_t>(&sps30Meas[0]);
            uartDevice->send("SPS30 data\r\n");
            uartDevice->send(sps30Decoded.toString());


            //Display
            if (!display.OLEDSetBufferPtr(128, 32, screenBuffer, sizeof(screenBuffer))) return;
            display.OLEDclearBuffer();
            display.setTextColor(WHITE);
            display.setCursor(0, 0);
            display.setTextSize(1);
           // display.print(dateTime.toString(dateFormat2));
           // display.setCursor(10, 13);
            display.print(sps30Decoded.toString());
            display.OLEDupdate();

        }
    }
}

} // namespace Task
