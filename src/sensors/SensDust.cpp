//
// Created by Carina on 08.12.2024.
//

#include "SensDust.h"

SensDust::SensDust(uint8_t *data) {
    sensorNum = (data[2] << 8) | data[3];
    pm1_0Concentration = (data[4] << 8) | data[5];
    pm2_5Concentration = (data[6] << 8) | data[7];
    pm10_0Concentration = (data[8] << 8) | data[9];
    pm1_0ConcentrationAtmospheric = (data[10] << 8) | data[11];
    pm2_5ConcentrationAtmospheric = (data[12] << 8) | data[13];
    pm10_0ConcentrationAtmospheric = (data[14] << 8) | data[15];
    numPart0_3 = (data[16] << 8) | data[17];
    numPart0_5 = (data[18] << 8) | data[19];
    numPart1_0 = (data[20] << 8) | data[21];
    numPart2_5 = (data[22] << 8) | data[23];
    numPart5_0 = (data[24] << 8) | data[25];
    numPart10 = (data[26] << 8) | data[27];
    valid = false;
    int8_t checksum = 0;
    for(int i = 0; i<28; i++){
        checksum += data[i];
    }
    if(checksum == data[28]){
        valid = true;
    }
}

std::string SensDust::toString() {
    char result[500];
    sprintf(result, "SensorNr. %u\n Concentration PM1.0 %u, PM2.5 %u, PM10.0 %u \n "
                    "Atmospheric environment Concentration: PM1.0 %u, PM2.5 %u, PM10.0 %u \n"
                    "Number of Particles: 0.3 um: %u, 0.5 um: %u, 1.0 um: %u, 2.5 um: %u, 5.0 um: %u, 10 um: %u\n",
            sensorNum, pm1_0Concentration, pm2_5Concentration, pm10_0Concentration,
            pm1_0ConcentrationAtmospheric, pm2_5ConcentrationAtmospheric, pm10_0ConcentrationAtmospheric,
            numPart0_3, numPart0_5, numPart1_0, numPart2_5, numPart5_0, numPart10);
    return result;
}
