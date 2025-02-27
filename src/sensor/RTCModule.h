//
// Created by Carina on 19.01.2025.
//

#ifndef ISD_PROJECT_RTCMODULE_H
#define ISD_PROJECT_RTCMODULE_H

#include "RTClib.h"
#include "i2c/PicoI2C.hpp"
#include "uart/PicoOsUart.hpp"

#include <memory>

class RTCModule
{
  public:
    RTCModule(std::shared_ptr<I2c::PicoI2C> i2cDevice, std::shared_ptr<Uart::PicoOsUart> uartDevice);
    DateTime getDateTime();
    void setDateTime(const DateTime& dt);

  private:
    std::shared_ptr<I2c::PicoI2C> i2cDevice;
    std::shared_ptr<Uart::PicoOsUart> uartDevice;
};

#endif // ISD_PROJECT_RTCMODULE_H
