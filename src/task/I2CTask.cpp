//
// Created by Carina Schleiwies on 05.12.2024.
//

#include "I2CTask.h"

#include "pico/binary_info.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "projdefs.h"
#include "sensors/MS430.h"
#include "sensors/RTClib.h"
#include "sensors/SensDust.h"
#include <hardware/i2c.h>
#include <hardware/structs/io_qspi.h>
#include <pico/error.h>

#include <chrono>
#include <cstdio>
#include <utility>

#define RDY_PIN 19

#define DS3231_ADDRESS 0x68   ///< I2C address for DS3231
#define DS3231_TIME 0x00      ///< Time register
#define DS3231_ALARM1 0x07    ///< Alarm 1 register
#define DS3231_ALARM2 0x0B    ///< Alarm 2 register
#define DS3231_CONTROL 0x0E   ///< Control register
#define DS3231_STATUSREG 0x0F ///< Status register
#define DS3231_TEMPERATUREREG                                                  \
  0x11 ///< Temperature register (high byte - low byte is at 0x12), 10-bit
///< temperature value

namespace Task
{

I2CTask::I2CTask(std::shared_ptr<I2c::PicoI2C> i2cDevice, std::shared_ptr<Uart::PicoOsUart> uartDevice) : BaseTask{"I2CTask", 1024, this, HIGH} {
    this->i2cDevice = std::move(i2cDevice);
    this->uartDevice = std::move(uartDevice);
}


uint8_t bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }
uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }
/*!
     @brief  Convert the day of the week to a representation suitable for
             storing in the DS3231: from 1 (Monday) to 7 (Sunday).
     @param  d Day of the week as represented by the library:
             from 0 (Sunday) to 6 (Saturday).
     @return the converted value
 */
uint8_t dowToDS3231(uint8_t d) { return d == 0 ? 7 : d; }

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

            int ret;
            uint8_t rxdata;
            ret = i2cDevice->read(addr, &rxdata, 1);
            if (ret == 0)
            {
               // if (addr == 0x08) { uartDevice->send("Sensor Gas SENS V2 not connected"); }
                if (addr == 0x3c) { uartDevice->send("Display not connected"); }
                // if (addr == 0x40) { uartDevice->send("Sensor SENS DUST not connected"); }
                if (addr == 0x68) { uartDevice->send("RTC not connected"); }
                if (addr == 0x71) { uartDevice->send("Sensor MS430 not connected"); }
            }
            uartDevice->send(ret <= 0 ? "." : "@");
            uartDevice->send(addr % 16 == 15 ? "\r\n" : "  ");
        }
        uartDevice->send("Done.\r\n");


//        DateTime dt = DateTime(2024,12,22,2,7,20); //FIXME: Find way to always have accurate time
//        uint8_t buffer[8] = {DS3231_TIME,
//                             bin2bcd(dt.second()),
//                             bin2bcd(dt.minute()),
//                             bin2bcd(dt.hour()),
//                             bin2bcd(dowToDS3231(dt.dayOfTheWeek())),
//                             bin2bcd(dt.day()),
//                             bin2bcd(dt.month()),
//                             bin2bcd(dt.year() - 2000U)};
//        i2cDevice->write(0x68, buffer, 8);
//        uint8_t reg = DS3231_STATUSREG;
//        uint8_t statreg[2] = {DS3231_STATUSREG, 0};
//        i2cDevice->transaction(0x68, &reg, 1, &statreg[1], 1);
//        statreg[1] &= ~0x80; // flip OSF bit
//        i2cDevice->write(0x68, &statreg[0], 1);

        int rdy = RDY_PIN;

        gpio_init(rdy);
        gpio_pull_up(rdy);
        gpio_set_function(rdy, GPIO_FUNC_NULL);

        char data[20];


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


            //TODO: 0x71 Combi-Sensor -> Daten nacheinander abfragen und Klasse für die Werte erstellen, Nicht getestet, da READY Signal dauerhaft auf HIGH steht und dadurch keine Kommunikation stattfinden kann
//            uint8_t value = 0xE1;
//            bool rdy2 = gpio_get(rdy);
//            while(rdy2==true){
//                rdy2 = gpio_get(rdy);
//                vTaskDelay(pdMS_TO_TICKS(10));
//            }
//            int ret =  i2cDevice->write(0x71, &value, 1);
//            uartDevice->send(ret != 1 ? "Write error for Combi-Sensor\n" : "");
//            rdy2 = gpio_get(rdy);
//            while(rdy2==true){
//                rdy2 = gpio_get(rdy);
//                vTaskDelay(pdMS_TO_TICKS(10));
//            }
//            // vTaskDelay(pdMS_TO_TICKS(505));
//            uint8_t tempData[12];
//            uint8_t airQualityData[10];
//            uint8_t lightData[5];
//            uint8_t soundData[18];
//            uint8_t value2 = 0x10;
//
//            ret = i2cDevice->transaction(0x71, &value2,1, &tempData[0],12);
//
//            uartDevice->send(ret != 13 ? "Data read error for Combi-Sensor\n" : "");
//            rdy2 = gpio_get(rdy);
//            while(rdy2==true){
//                rdy2 = gpio_get(rdy);
//                vTaskDelay(pdMS_TO_TICKS(10));
//            }
//            value2 = 0x11;
////            ret = i2cDevice->transaction(0x71, &value2,1, &airQualityData[0],10);
////
////            uartDevice->send(ret != 11 ? "Data read error1 for Combi-Sensor\n" : "");
////            rdy2 = gpio_get(rdy);
////            while(rdy2==true){
////                rdy2 = gpio_get(rdy);
////                vTaskDelay(pdMS_TO_TICKS(10));
////            }
//            value2 = 0x12;
//            ret = i2cDevice->transaction(0x71, &value2,1, &lightData[0],5);
//            uartDevice->send(ret != 6 ? "Data read error2 for Combi-Sensor\n" : "");
//
//            rdy2 = gpio_get(rdy);
//            while(rdy2==true){
//                rdy2 = gpio_get(rdy);
//                vTaskDelay(pdMS_TO_TICKS(10));
//            }
//            value2 = 0x13;
//            ret = i2cDevice->transaction(0x71, &value2,1, &soundData[0],18);
//            uartDevice->send(ret != 19 ? "Data read error3 for Combi-Sensor\n" : "");
//            vTaskDelay(pdMS_TO_TICKS(500));
//
//            MS430 ms430Data = MS430(&tempData[0], &airQualityData[0], &lightData[0], &soundData[0]);
//            uartDevice->send(ms430Data.toString());
//            uartDevice->send("Done Reading MS430\r\n");
            //RTC-Modul

            uint8_t buffer[7];
            buffer[0] = 0;
            int ret = i2cDevice->transaction(0x68, &buffer[0], 1, &buffer[0],7);
            uartDevice->send(ret != 8 ? "Data read error for RTC\n" : "");

            DateTime dateTime = DateTime(bcd2bin(buffer[6]) + 2000U, bcd2bin(buffer[5] & 0x7F),
                                         bcd2bin(buffer[4]), bcd2bin(buffer[2]), bcd2bin(buffer[1]),
                                         bcd2bin(buffer[0] & 0x7F));
            std::string str = "DDD, DD MMM YYYY hh:mm:ss\r\n";
            int n = str.length();
            char dateFormat[n + 1];
            strcpy(dateFormat, str.c_str());

            uartDevice->send(dateTime.toString(dateFormat));
            vTaskDelay(pdMS_TO_TICKS(1000));
            //TODO: Bonus SPS30 I2c ausprobieren

            //TODO: Bonus: Display

        }
    }
}

} // namespace Task
