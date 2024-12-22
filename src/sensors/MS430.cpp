//
// Created by Carina on 21.12.2024.
//

#include "MS430.h"

MS430::MS430(uint8_t *airData, uint8_t *airQualityData, uint8_t *lightData, uint8_t *soundData)
{
    temp = airData[0] + (airData[1]/10);
    pressure = (airData[2] << 24) | (airData[3] << 16) | (airData[4] << 8) | airData[5];
    hum = airData[7];
    hum = hum/10;
    hum += (float) airData[6];
    gas_sens_r = (airData[8] << 24) | (airData[9] << 16) | (airData[10] << 8) | airData[11];
    aqi = ((airQualityData[0] << 8) | airQualityData[1]) + airQualityData[2]/10;
    co2 = ((airQualityData[3] << 8) | airQualityData[4]) + airQualityData[5]/10;
    voc = ((airQualityData[6] << 8) | airQualityData[7]) + airQualityData[8]/100;
    aqi_accuracy = airQualityData[9];
    illuminance = ((lightData[0] << 8) | lightData[1]) + lightData[2]/100;
    white_light_level = (lightData[3] << 8) | lightData[4];
    dbA = soundData[0] + (soundData[1]/10);
    for(int i = 0; i<6; i++){
        db[i] = soundData[2+i] + (soundData[8+i]/10);
    }
    peak_sound_amplitude = ((soundData[14] << 8) | soundData[15]) + soundData[16]/100;
    sound_stable = soundData[17];
}

std::string MS430::toString() {  char result[500];
    sprintf(result, "AirData: \r\n Temp %f, Press %u, Hum %f, gas_resistance %u \r\n"
                    "AirQualityData: \r\n aqi %f, co2 %f, voc %f, aqiAccuracy: %u \r\n"
                    "LightData: \r\n Illuminance %f, White Light %u \r\n"
                    "SoundData: \r\n dbA %f, peak %f, stable %d, db1 %f, db2 %f, db3 %f, db4 %f, db5 %f, db6 %f\r\n",
            temp, pressure, hum, gas_sens_r, aqi, co2, voc, aqi_accuracy, illuminance, white_light_level,
            dbA, peak_sound_amplitude, sound_stable, db[0], db[1], db[2], db[3], db[4], db[5]);
    return result;
}
