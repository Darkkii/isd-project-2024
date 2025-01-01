//
// Created by Carina on 23.12.2024.
//

#ifndef ISD_PROJECT_SPS30_H
#define ISD_PROJECT_SPS30_H

#include <cstdint>
#include <string>

template <typename T>
class SPS30
{
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

  public:
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
    T var = (data[0] << 8) | data[1];
    uint8_t check = calcCrc(&data[0]);
    if(check != data[2]){
        var = 65535;
    }
    return var;
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
    }
}


template <typename T>
std::string SPS30<T>::toString()
{
    char result[500];
    if constexpr (std::is_same_v<T, uint16_t>) {
         sprintf(result, "partSize: %u um \r\nMass [um/m^3]: PM1.0: %u, PM2.5: %u, PM4.0: %u, PM10.0: %u\r\n"
                "Number [#/cm^3]: PM0.5: %u, PM1.0: %u, PM2.5: %u, PM4.0: %u, PM10.0: %u\r\n",
                        partSize, mass1_0, mass2_5, mass4_0, mass10_0,
                        number0_5, number1_0, number2_5, number4_0, number10_0
        );
    }

    return result;
}
#endif // ISD_PROJECT_SPS30_H
