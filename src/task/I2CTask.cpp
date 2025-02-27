//
// Created by Carina Schleiwies on 05.12.2024.
//

#include "I2CTask.h"

#include "Display-lib/SSD1306_OLED.hpp"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "projdefs.h"
#include "sensor/MS430.h"
#include "sensor/RTCModule.h"
#include "sensor/RTClib.h"
#include "sensor/SPS30.h"
// #include "sensors/SensDust.h"
#include <hardware/i2c.h>
// #include <hardware/structs/io_qspi.h>
#include <pico/error.h>

#include <chrono>
#include <cstdio>
#include <utility>

#define FULLSCREEN (128 * (32 / 8))

extern volatile uint8_t newDataOnMS430;

namespace Task
{

I2CTask::I2CTask(std::shared_ptr<I2c::PicoI2C> i2cDevice,
                 std::shared_ptr<Uart::PicoOsUart> uartDevice,
                 std::shared_ptr<Sensor::SensorData> sensorData) :
    BaseTask{"I2CTask", 1024, this, MED}
{
    this->i2cDevice = std::move(i2cDevice);
    this->uartDevice = std::move(uartDevice);
    this->sensorData = std::move(sensorData);
}

void I2CTask::run()
{
    //    int rc = cyw43_arch_init();
    //    if (rc == PICO_OK)
    //    {
    uartDevice->send("\r\nI2C Bus Scan\r\n");
    uartDevice
        ->send("\r\n   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\r\n\n");
    char addrData[10];
    for (int addr = 0; addr < (1 << 7); ++addr)
    {
        if (addr % 16 == 0)
        {
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
            if (addr == 0x71)
            {
                uartDevice->send("Sensor MS430 not connected");
            }
        }
        uartDevice->send(ret <= 0 ? "." : "@");
        uartDevice->send(addr % 16 == 15 ? "\r\n" : "  ");
    }
    uartDevice->send("Done.\r\n");

    auto display = SSD1306(128, 32, i2cDevice);
    uint8_t screenBuffer[FULLSCREEN];
    if (!display.OLEDSetBufferPtr(128, 32, screenBuffer, sizeof(screenBuffer)))
        return;
    display.OLEDclearBuffer();
    display.OLEDbegin();
    display.OLEDFillScreen(0x15, 0); // F0splash screen bars, optional just for effect
    vTaskDelay(pdMS_TO_TICKS(1000));
    display.OLEDStopScroll();
    display.setTextColor(WHITE);
    display.setCursor(10, 10);
    display.print("I2C working");
    display.OLEDupdate();

    vTaskDelay(10);

    //        vTaskDelay(pdMS_TO_TICKS(10000));
    //
    //
    ////        if (!display.OLEDSetBufferPtr(128, 64, screenBuffer, sizeof(screenBuffer))) return;
    ////
    ////        display.OLEDclearBuffer();
    ////        //display.OLEDupdate();
    ////        //vTaskDelay(1000);
    ////        //display.OLEDclearBuffer();
    ////        display.setTextColor(WHITE);
    ////        display.setCursor(32, 0);
    ////        display.setTextSize(1);
    ////        display.print("abcdef");
    ////        //display.setCursor(0, 32);
    ////        //display.print("12345678901234");
    ////        //            vTaskDelay(pdMS_TO_TICKS(1000));
    ////        display.OLEDupdate();
    ////        //            vTaskDelay(pdMS_TO_TICKS(1000));

    // Display
    if (!display.OLEDSetBufferPtr(128, 32, screenBuffer, sizeof(screenBuffer)))
        return;
    display.OLEDclearBuffer();
    // display.OLEDupdate();
    // vTaskDelay(1000);
    // display.OLEDclearBuffer();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("WiFi: ISD_SENSOR_DATA\n\nIP: 192.168.0.1\n");
    // display.setCursor(0, 32);
    display.OLEDupdate();
    vTaskDelay(pdMS_TO_TICKS(100));

    auto rtc = RTCModule(i2cDevice, uartDevice);
    //        rtc.setDateTime(DateTime(2025,02,27,9,58,00)); //FIXME: Find way to always have accurate time
    std::string str = "DDD, DD MMM YYYY hh:mm:ss\r\n";
    char dateFormat[str.length() + 1];
    strcpy(dateFormat, str.c_str());
    std::string str2 = "DD.MM.YY hh:mm:ss\r\n";
    char dateFormat2[str2.length() + 1];
    strcpy(dateFormat2, str2.c_str());
    DateTime dateTime = rtc.getDateTime();

    vTaskDelay(10);

    // Setup MS430
    auto ms430 = MS430(i2cDevice);
    uartDevice->send(ms430.reset() != 0 ? "Write error Reset for Combi-Sensor\n" : "");
    uartDevice->send(ms430.configCycleMode() != 0
                         ? "Write error configCycleMode for Combi-Sensor\n"
                         : "");
    vTaskDelay(pdMS_TO_TICKS(10));
    uartDevice->send(ms430.startCycleMode() != 0
                         ? "Write error CycleMode for Combi-Sensor\n"
                         : "");
    vTaskDelay(pdMS_TO_TICKS(10));
    newDataOnMS430 = 0;

    // SPS30
    // start Measurement
    // Pointer: 0x0010, 0x05 -> int, 0x00, checksum von 0x05 und 0x00
    uint8_t sps30Data[2] = {0x03, 0x00};
    uint8_t sps30StartMeasurement[5] = {0x00, 0x10, 0x03, 0x00, calcCrc(sps30Data)};
    uint ret = i2cDevice->write(0x69, &sps30StartMeasurement[0], 5);
    uartDevice->send(ret != 5 ? "Write error for SPS30\n" : "");
    vTaskDelay(pdMS_TO_TICKS(25));
    uint8_t sps30Meas[60];
    // Read Data: Pointer: 0x0300
    sps30Data[0] = 0x03;
    sps30Data[1] = 0x00;

    vTaskDelay(10);

    while (true)
    {
        // SensDust
        //             uint8_t value = 0x88;
        //             uint ret = i2cDevice->write(0x40, &value, 1);
        //             uartDevice->send(ret != 1 ? "Write error for SENS
        //             DUST\r\n" : ""); vTaskDelay(pdMS_TO_TICKS(100)); uint8_t
        //             sensDustData[30]; ret = i2cDevice->read(0x40,
        //             &sensDustData[0], 29); uartDevice->send(ret != 29 ? "Read
        //             error for SENS DUST\r\n" : ""); SensDust sensDustDecoded
        //             = SensDust(sensDustData); if(sensDustDecoded.valid){
        //                 uartDevice->send(sensDustDecoded.toString());
        //             }else{
        //                 uartDevice->send("Data error for SENS DUST\r\n");
        //             }

        //            0x71 Combi-Sensor
        if (newDataOnMS430 == 1)
        {
            uartDevice->send(ms430.updateEnvironmentData() != 0
                                 ? "Data read error for Combi-Sensor\n"
                                 : "");
            uartDevice->send(ms430.updateAirQualityData() != 0
                                 ? "Data read error1 for Combi-Sensor\n"
                                 : "");
            uartDevice->send(ms430.updateLightData() != 0
                                 ? "Data read error2 for Combi-Sensor\n"
                                 : "");
            uartDevice->send(ms430.updateSoundData() != 0
                                 ? "Data read error3 for Combi-Sensor\n"
                                 : "");
            uartDevice->send(ms430.toString());
            sensorData->setTemperature(ms430.temp);
            sensorData->setHumidity(ms430.hum);
            sensorData->setPressure(ms430.pressure / 100);
            sensorData->setIllumination(ms430.illuminance);
            sensorData->setNoise(ms430.dbA);
            sensorData->setVoc(ms430.voc);
            // uartDevice->send("Done Reading MS430\r\n");
            newDataOnMS430 = 0;
        }

        vTaskDelay(10);

        //            RTC-Modul
        auto rtc2 = RTCModule(i2cDevice, uartDevice);
        DateTime dateTime2 = rtc2.getDateTime();
        uartDevice->send(dateTime2.toString(dateFormat2));
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Bonus SPS30 I2c -> 0x69
        //            uint8_t sps30Rdy[3];
        //            //Read data ready flag
        //            sps30Data[0] = 0x02;
        //            sps30Data[1] = 0x02;
        //            //Pointer: 0x0300, 2 data, checksum ... int insgesamt: 30
        //            ret =  i2cDevice->write(0x69, &sps30Data[0], 2);
        //            uartDevice->send(ret != 2 ? "Write error0 for SPS30\n" : "");
        //            ret =  i2cDevice->read(0x69, &sps30Rdy[0], 3);
        //            uartDevice->send(ret != 3 ? "Read error0 for SPS30\n" : "");
        //            uint8_t check = calcCrc(sps30Rdy);
        //            uartDevice->send(check != sps30Rdy[2] ? "Read error1 for SPS30\n" : "");

        // Read Measured Values

        // Pointer: 0x0300, 2 data, checksum ... int insgesamt: 30
        ret = i2cDevice->write(0x69, &sps30Data[0], 2);
        uartDevice->send(ret != 2 ? "Write error1 for SPS30\n" : "");
        ret = i2cDevice->read(0x69, &sps30Meas[0], 60);
        uartDevice->send(ret != 60 ? "Read error1 for SPS30\n" : "");

        auto sps30Decoded = SPS30<float>(&sps30Meas[0]);
        uartDevice->send("SPS30 data\r\n");
        uartDevice->send(sps30Decoded.toString());
        if (sps30Decoded.partSize > 0.001 && sps30Decoded.mass1_0 > 0.001
            && sps30Decoded.mass2_5 > 0.001 && sps30Decoded.mass4_0 > 0.001
            && sps30Decoded.mass10_0 > 0.001)
        {
            sensorData->setParticles(sps30Decoded.mass1_0,
                                     sps30Decoded.mass2_5,
                                     sps30Decoded.mass4_0,
                                     sps30Decoded.mass10_0,
                                     sps30Decoded.number1_0,
                                     sps30Decoded.number2_5,
                                     sps30Decoded.number4_0,
                                     sps30Decoded.number10_0,
                                     sps30Decoded.partSize);
        }
    }
}

} // namespace Task
