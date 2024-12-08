//
// Created by Carina Schleiwies on 05.12.2024.
//

#ifndef ISD_PROJECT_I2CTASK_H
#define ISD_PROJECT_I2CTASK_H

#include "BaseTask.hpp"
#include "i2c/PicoI2C.hpp"
#include "uart/PicoOsUart.hpp"

#include <memory>

namespace Task
{
class I2CTask : public BaseTask
{
  public:
    I2CTask(std::shared_ptr<I2c::PicoI2C> i2cDevice, std::shared_ptr<Uart::PicoOsUart> uartDevice);
    void run() override;

  private:
    std::shared_ptr<I2c::PicoI2C> i2cDevice;
    std::shared_ptr<Uart::PicoOsUart> uartDevice;
};
} // namespace Task


#endif // ISD_PROJECT_I2CTASK_H
