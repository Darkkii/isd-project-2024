//
// Created by Carina on 08.12.2024.
//

#ifndef ISD_PROJECT_SENSDUST_H
#define ISD_PROJECT_SENSDUST_H

#include <cstdint>
#include <string>

class SensDust
{
    uint16_t sensorNum;
    uint16_t pm1_0Concentration;
    uint16_t pm2_5Concentration;
    uint16_t pm10_0Concentration;
    uint16_t pm1_0ConcentrationAtmospheric;
    uint16_t pm2_5ConcentrationAtmospheric;
    uint16_t pm10_0ConcentrationAtmospheric;
    uint16_t numPart0_3;
    uint16_t numPart0_5;
    uint16_t numPart1_0;
    uint16_t numPart2_5;
    uint16_t numPart5_0;
    uint16_t numPart10;

  public:
    explicit SensDust(const uint8_t * data);
    std::string toString() const;
    bool valid;
};

#endif // ISD_PROJECT_SENSDUST_H
