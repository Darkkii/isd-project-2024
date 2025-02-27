//
// Created by Carina on 19.01.2025.
//

#include "RTCModule.h"

#define DS3231_ADDRESS 0x68   ///< I2C address for DS3231
#define DS3231_TIME 0x00      ///< Time register
#define DS3231_ALARM1 0x07    ///< Alarm 1 register
#define DS3231_ALARM2 0x0B    ///< Alarm 2 register
#define DS3231_CONTROL 0x0E   ///< Control register
#define DS3231_STATUSREG 0x0F ///< Status register
#define DS3231_TEMPERATUREREG                                                  \
  0x11 ///< Temperature register (high byte - low byte is at 0x12), 10-bit
///< temperature value

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

RTCModule::RTCModule(std::shared_ptr<I2c::PicoI2C> i2cDevice,
                     std::shared_ptr<Uart::PicoOsUart> uartDevice)
{
    this->i2cDevice = std::move(i2cDevice);
    this->uartDevice = std::move(uartDevice);
}


DateTime RTCModule::getDateTime() {
    uint8_t buffer[7];
    buffer[0] = 0;
    uint ret = i2cDevice->transaction(0x68, &buffer[0], 1, &buffer[0],7);
    uartDevice->send(ret != 8 ? "Data read error for RTC\n" : "");

    DateTime dateTime = DateTime(bcd2bin(buffer[6]) + 2000U, bcd2bin(buffer[5] & 0x7F),
                                 bcd2bin(buffer[4]), bcd2bin(buffer[2]), bcd2bin(buffer[1]),
                                 bcd2bin(buffer[0] & 0x7F));
    return dateTime;
}

void RTCModule::setDateTime(const DateTime& dt) {
    uint8_t buffer[8] = {DS3231_TIME,
                         bin2bcd(dt.second()),
                         bin2bcd(dt.minute()),
                         bin2bcd(dt.hour()),
                         bin2bcd(dowToDS3231(dt.dayOfTheWeek())),
                         bin2bcd(dt.day()),
                         bin2bcd(dt.month()),
                         bin2bcd(dt.year() - 2000U)};
    i2cDevice->write(0x68, buffer, 8);
    uint8_t reg = DS3231_STATUSREG;
    uint8_t statreg[2] = {DS3231_STATUSREG, 0};
    i2cDevice->transaction(0x68, &reg, 1, &statreg[1], 1);
    statreg[1] &= ~0x80; // flip OSF bit
    i2cDevice->write(0x68, &statreg[0], 1);
}