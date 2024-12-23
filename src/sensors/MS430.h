//
// Created by Carina on 21.12.2024.
//

#ifndef ISD_PROJECT_MS430_H
#define ISD_PROJECT_MS430_H

#include <cstdint>
#include <string>

class MS430
{
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
    explicit MS430(const uint8_t * airData, const uint8_t * airQualityData, const uint8_t * lightData, const uint8_t * soundData);
    std::string toString() const;
};

#endif // ISD_PROJECT_MS430_H
