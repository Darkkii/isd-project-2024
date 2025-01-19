//
// Created by Carina on 21.12.2024.
//

#ifndef ISD_PROJECT_MS430_H
#define ISD_PROJECT_MS430_H

#include "i2c/PicoI2C.hpp"
#include "uart/PicoOsUart.hpp"

#include <cstdint>
#include <memory>
#include <string>


#define RDY_PIN 20

class MS430
{
  private:
    std::shared_ptr<I2c::PicoI2C> i2cDevice;

    float temp;
    uint32_t pressure;
    float hum;
    uint32_t gas_sens_r;
    float aqi;
    float co2;
    float voc;
    uint8_t aqi_accuracy;
    float illuminance;
    uint16_t white_light_level;
    float dbA;
    float db[6];
    float peak_sound_amplitude;
    bool sound_stable;

  public:
    explicit MS430(std::shared_ptr<I2c::PicoI2C> i2cDevice);
    [[nodiscard]] std::string toString() const;
    void waitReady();
    int reset();
    int configCycleMode();
    int startCycleMode();
    int updateEnvironmentData();
    int updateAirQualityData();
    int updateLightData();
    int updateSoundData();
};

#endif // ISD_PROJECT_MS430_H
