//
// Created by Carina on 21.12.2024.
//

#include "MS430.h"

MS430::MS430(const uint8_t *airData, const uint8_t *airQualityData, const uint8_t *lightData, const uint8_t *soundData)
{
    temp = airData[1];
    temp = temp/10;
    temp += (float) airData[0];
    pressure = (airData[5] << 24) | (airData[4] << 16) | (airData[3] << 8) | airData[2];
    hum = airData[7];
    hum = hum/10;
    hum += (float) airData[6];
    gas_sens_r = (airData[11] << 24) | (airData[10] << 16) | (airData[9] << 8) | airData[8];
    aqi = airQualityData[2];
    aqi = aqi/10;
    aqi += (float) ((airQualityData[1] << 8) | airQualityData[0]);
    co2 = airQualityData[5];
    co2 = co2/10;
    co2 += (float) ((airQualityData[4] << 8) | airQualityData[3]);
    voc = airQualityData[8];
    voc = voc/100;
    voc = (float) ((airQualityData[7] << 8) | airQualityData[6]);
    aqi_accuracy = airQualityData[9];
    illuminance = lightData[2];
    illuminance = illuminance/100;
    illuminance += (float) ((lightData[1] << 8) | lightData[0]);
    white_light_level = (lightData[4] << 8) | lightData[3];
    dbA = soundData[1];
    dbA = dbA/10;
    dbA += (float) soundData[0];
    for(int i = 0; i<6; i++){
        db[i] = soundData[8+i];
        db[i] = db[i]/10;
        db[i] += (float) soundData[2+i];
    }
    peak_sound_amplitude = soundData[16];
    peak_sound_amplitude = peak_sound_amplitude/100;
    peak_sound_amplitude += (float) ((soundData[15] << 8) | soundData[14]);
    sound_stable = soundData[17];
}

std::string MS430::toString() const {
    char result[500];
    sprintf(result, "AirData: \r\n Temp %.1f C, Press %u Pa, Hum %.1f %%, gas_resistance %u \r\n"
                    "AirQualityData: \r\n aqi %.1f, co2 %.1f, voc %.2f, aqiAccuracy: %u \r\n"
                    "LightData: \r\n Illuminance %.2f lux, White Light %u \r\n"
                    "SoundData: \r\n dbA %.1f, peak %.2f, stable %d, \r\n",
            //   db1 %.1f, db2 %.1f, db3 %.1f, db4 %.1f, db5 %.1f, db6 %.1f\r\n",
            temp, pressure, hum, gas_sens_r,
            aqi, co2, voc, aqi_accuracy,
                illuminance, white_light_level,
            dbA, peak_sound_amplitude, sound_stable);
            // db[0], db[1], db[2], db[3], db[4], db[5]);
    return result;
}
