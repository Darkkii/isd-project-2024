//
// Created by Keijo Länsikunnas on 10.9.2024.
//

#ifndef PICOI2C_HPP
#define PICOI2C_HPP
#include "FreeRTOS.h" // IWYU pragma: keep
#include "hardware/i2c.h"
#include "semaphore/Mutex.hpp"
#include "semphr.h"
#include "task.h"

namespace I2c
{

enum i2cBus : uint
{
    BUS_0,
    BUS_1
};

class PicoI2C
{
  public:
    explicit PicoI2C(int bus_nr, uint speed = 100000);
    PicoI2C(const PicoI2C &) = delete;
    uint write(uint8_t addr, const uint8_t *buffer, uint length);
    uint read(uint8_t addr, uint8_t *buffer, uint length);
    uint transaction(uint8_t addr,
                     const uint8_t *wbuffer,
                     uint wlength,
                     uint8_t *rbuffer,
                     uint rlength);

  private:
    i2c_inst *i2c;
    int irqn;
    TaskHandle_t task_to_notify;
    Semaphore::Mutex access;
    const uint8_t *wbuf;
    uint wctr;
    uint8_t *rbuf;
    uint rctr;
    uint rcnt;
    void tx_fill_fifo();
    void rx_fill_fifo();

    void isr();
    static void i2c0_irq();
    static void i2c1_irq();
    static PicoI2C *i2c0_instance;
    static PicoI2C *i2c1_instance;
};

} // namespace I2c

#endif /* PICOI2C_HPP */
