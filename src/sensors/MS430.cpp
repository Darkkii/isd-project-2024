//
// Created by Carina on 21.12.2024.
//

#include "MS430.h"

#include <hardware/gpio.h>

MS430::MS430(std::shared_ptr<I2c::PicoI2C> i2cDevice)
{
    this->i2cDevice = std::move(i2cDevice);
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

int MS430::reset() {
    waitReady();
    uint8_t value = 0xE2;
    uint ret =  i2cDevice->write(0x71, &value, 1); //Reset
    if(ret == 1) return 0;
    else return 1;
}

int MS430::configCycleMode() {
    waitReady();
    uint8_t val2[2] = {0x89, 0x00};
    uint ret =  i2cDevice->write(0x71, &val2[0], 2); //Cycle Mode Config
    if(ret == 2) return 0;
    else return 1;
}

int MS430::startCycleMode() {
    waitReady();
    uint8_t value = 0xE4;
    uint ret =  i2cDevice->write(0x71, &value, 1); //Cycle Mode
    if(ret == 1) return 0;
    else return 1;
}

void MS430::waitReady() {
    bool rdy2 = gpio_get(RDY_PIN);
    while(rdy2){
        rdy2 = gpio_get(RDY_PIN);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int MS430::updateEnvironmentData() {
    waitReady();
    uint8_t value = 0x10;
    uint8_t airData[12];
    uint32_t ret = i2cDevice->transaction(0x71, &value, 1, &airData[0], 12);
    if(ret != 13){
        return 1;
    }
    temp = airData[1];
    temp = temp/10;
    temp += (float) airData[0];
    pressure = (airData[5] << 24) | (airData[4] << 16) | (airData[3] << 8) | airData[2];
    hum = airData[7];
    hum = hum/10;
    hum += (float) airData[6];
    gas_sens_r = (airData[11] << 24) | (airData[10] << 16) | (airData[9] << 8) | airData[8];
    return 0;
}

int MS430::updateAirQualityData() {
    waitReady();
    uint8_t value = 0x11;
    uint8_t airQualityData[10];
    uint32_t ret = i2cDevice->transaction(0x71, &value,1, &airQualityData[0],10);
    if(ret != 11){
        return 1;
    }
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
    return 0;
}

int MS430::updateLightData() {
    waitReady();
    uint8_t value = 0x12;
    uint8_t lightData[5];
    uint32_t ret = i2cDevice->transaction(0x71, &value,1, &lightData[0],5);
    if(ret != 6){
        return 1;
    }
    illuminance = lightData[2];
    illuminance = illuminance/100;
    illuminance += (float) ((lightData[1] << 8) | lightData[0]);
    white_light_level = (lightData[4] << 8) | lightData[3];
    return 0;
}

int MS430::updateSoundData() {
    waitReady();
    uint8_t value = 0x13;
    uint8_t soundData[18];
    uint32_t ret = i2cDevice->transaction(0x71, &value,1, &soundData[0],18);
    if(ret != 19){
        return 1;
    }
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
    return 0;
}
