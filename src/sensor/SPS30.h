//
// Created by Carina on 23.12.2024.
//

#ifndef ISD_PROJECT_SPS30_H
#define ISD_PROJECT_SPS30_H

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <stdfloat>
#include <string>
#include <vector>
#include <cstring>
#include <cmath>

std::vector<float> unpackRawData(const std::vector<uint8_t>& rawData) {
    if (rawData.size() != 40) {

    }

    std::vector<float> unpackedData(10);
    for (int i = 0; i < 10; ++i) {
        float value;
        std::memcpy(&value, &rawData[i * 4], 4);
        unpackedData[i] = value;
    }

    return unpackedData;
}


std::vector<uint8_t> reverseByteStuffing(std::vector<uint8_t>& rawData) {
    for (size_t i = 0; i < rawData.size() - 1; ++i) {
        if (rawData[i] == 0x7D) {
            if (rawData[i + 1] == 0x5E) {
                rawData[i] = 0x7E;
            } else if (rawData[i + 1] == 0x5D) {
                rawData[i] = 0x7D;
            } else if (rawData[i + 1] == 0x31) {
                rawData[i] = 0x11;
            } else if (rawData[i + 1] == 0x33) {
                rawData[i] = 0x13;
            }
            rawData.erase(rawData.begin() + i + 1);
        }
    }
    return rawData;
}

template <typename T>
class SPS30
{
  public:
    T mass1_0;
    T mass2_5;
    T mass4_0;
    T mass10_0;

    T number0_5;
    T number1_0;
    T number2_5;
    T number4_0;
    T number10_0;
    T partSize;

    explicit SPS30(uint8_t * data);
    std::string toString();
};

uint8_t calcCrc(const uint8_t data[2]) {
    uint8_t crc = 0xFF;
    for(int i = 0; i < 2; i++) {
        crc ^= data[i];
        for(uint8_t bit = 8; bit > 0; --bit) {
            if(crc & 0x80) {
                crc = (crc << 1) ^ 0x31u;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

template <typename T>
T decodeSPS30(uint8_t * data){
    if constexpr (std::is_same_v<T, uint16_t>)
    {
        T var = (data[0] << 8) | data[1];
        uint8_t check = calcCrc(&data[0]);
        if (check != data[2]) { var = 65535; }
        return var;
    }else if constexpr (std::is_same_v<T,float>){
    }
}

template <typename T>
SPS30<T>::SPS30(uint8_t *data)
{
    if constexpr (std::is_same_v<T, uint16_t>) {
        mass1_0 = decodeSPS30<T>(&data[0]);
        mass2_5 = decodeSPS30<T>(&data[3]);
        mass4_0 = decodeSPS30<T>(&data[6]);
        mass10_0 = decodeSPS30<T>(&data[9]);

        number0_5 = decodeSPS30<T>(&data[12]);
        number1_0 = decodeSPS30<T>(&data[15]);
        number2_5 = decodeSPS30<T>(&data[18]);
        number4_0 = decodeSPS30<T>(&data[21]);
        number10_0 = decodeSPS30<T>(&data[24]);

        partSize = decodeSPS30<T>(&data[27]);
    }else if constexpr (std::is_same_v<T,float>){
        std::vector<uint8_t> raw(data, data + 60);
        raw = reverseByteStuffing(raw);
        std::vector<uint8_t> rawData = std::vector<uint8_t>(raw.begin(), raw.end());

        for (int i = rawData.size() - 1; i >= 0; --i) {
            if ((i + 1) % 3 == 0) {
                rawData.erase(rawData.begin() + i);
            }
        }

        for (size_t i = 0; i < rawData.size(); i += 4) {
            if (i + 3 < rawData.size()) {
                std::reverse(rawData.begin() + i, rawData.begin() + i + 4);
            }
        }

        std::vector<float> unpackedData = unpackRawData(rawData);
        mass1_0 = std::round(unpackedData[0] * 1000.0f) / 1000.0f;
        mass2_5 = std::round(unpackedData[1] * 1000.0f) / 1000.0f;
        mass4_0 = std::round(unpackedData[2] * 1000.0f) / 1000.0f;
        mass10_0 = std::round(unpackedData[3] * 1000.0f) / 1000.0f;
        number0_5 = std::round(unpackedData[4] * 1000.0f) / 1000.0f;
        number1_0 = std::round(unpackedData[5] * 1000.0f) / 1000.0f;
        number2_5 = std::round(unpackedData[6] * 1000.0f) / 1000.0f;
        number4_0 = std::round(unpackedData[7] * 1000.0f) / 1000.0f;
        number10_0 = std::round(unpackedData[8]* 1000.0f) / 1000.0f;
        partSize = std::round(unpackedData[9]* 1000.0f) / 1000.0f;
    }
}


template <typename T>
std::string SPS30<T>::toString()
{
    char result[500];
    if constexpr (std::is_same_v<T, uint16_t>) {
         sprintf(result, "partSize: %u um \r\nMass [ug/m^3]: PM1.0: %u, PM2.5: %u, PM4.0: %u, PM10.0: %u\r\n"
                "Number [#/cm^3]: PM0.5: %u, PM1.0: %u, PM2.5: %u, PM4.0: %u, PM10.0: %u\r\n",
                        partSize, mass1_0, mass2_5, mass4_0, mass10_0,
                        number0_5, number1_0, number2_5, number4_0, number10_0
        );
    } else if constexpr (std::is_same_v<T, float>) {
        sprintf(result, "partSize: %.3f mm \r\nMass [ug/m^3]: PM1.0: %.3f, PM2.5: %.3f, PM4.0: %.3f, PM10.0: %.3f\r\n"
                        "Number [#/cm^3]: PM0.5: %.3f, PM1.0: %.3f, PM2.5: %.3f, PM4.0: %.3f, PM10.0: %.3f\r\n",
                partSize, mass1_0, mass2_5, mass4_0, mass10_0,
                number0_5, number1_0, number2_5, number4_0, number10_0
        );
    }

    return result;
}
#endif // ISD_PROJECT_SPS30_H
