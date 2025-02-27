#include "SensorData.hpp"

#include "rtos/semaphore/Mutex.hpp"

#include <mutex>

namespace Sensor
{

void SensorData::setTemperature(double temperature)
{
    std::lock_guard<Rtos::Semaphore::Mutex> lock(m_Access);
    m_Temperature = temperature;
}

void SensorData::setHumidity(double humidity)
{
    std::lock_guard<Rtos::Semaphore::Mutex> lock(m_Access);
    m_Humidity = humidity;
}

void SensorData::setPressure(double pressure)
{
    std::lock_guard<Rtos::Semaphore::Mutex> lock(m_Access);
    m_Pressure = pressure;
}

void SensorData::setCo2(double co2)
{
    std::lock_guard<Rtos::Semaphore::Mutex> lock(m_Access);
    m_Co2 = co2;
}

void SensorData::setParticles(double mass1_0,
                              double mass2_5,
                              double mass4_0,
                              double mass10_0,
                              double number1_0,
                              double number2_5,
                              double number4_0,
                              double number10_0,
                              double partSize)
{
    std::lock_guard<Rtos::Semaphore::Mutex> lock(m_Access);
    m_mass1_0 = mass1_0;
    m_mass2_5 = mass2_5;
    m_mass4_0 = mass4_0;
    m_mass10_0 = mass10_0;
    m_number1_0 = number1_0;
    m_number2_5 = number2_5;
    m_number4_0 = number4_0;
    m_number10_0 = number10_0;
    m_partSize = partSize;
}

void SensorData::setIllumination(double illumination)
{
    std::lock_guard<Rtos::Semaphore::Mutex> lock(m_Access);
    m_Illumination = illumination;
}

void SensorData::setNoise(double noise)
{
    std::lock_guard<Rtos::Semaphore::Mutex> lock(m_Access);
    m_Noise = noise;
}

void SensorData::setVoc(double voc)
{
    std::lock_guard<Rtos::Semaphore::Mutex> lock(m_Access);
    m_Voc = voc;
}

std::string SensorData::getJson()
{
    std::lock_guard<Rtos::Semaphore::Mutex> lock(m_Access);
    return std::string{R"([
        {
          "name": {
            "ger": "Temperatur",
            "fin": "Lämpötila",
            "en": "Temperature"
          },
          "value": ")" + std::to_string(m_Temperature)
                       + R"(",
          "unit": "°C"
        },
        {
          "name": {
            "ger": "Luftfeuchtigkeit",
            "fin": "Ilmankosteus",
            "en": "Humidity"
          },
          "value": ")" + std::to_string(m_Humidity)
                       + R"(",
          "unit": "%"
        },
        {
          "name": {
            "ger": "Luftdruck",
            "fin": "Ilmanpaine",
            "en": "Pressure"
          },
          "value": ")" + std::to_string(m_Pressure)
                       + R"(",
          "unit": "hPa"
        },
{
      "name": {
        "ger": "VOC-Luftqualität",
        "fin": "VOC ilmanlaatu",
        "en": "VOC air quality"
      },
      "value": ")" + std::to_string(m_number10_0)
                       + R"(",
      "unit": "ppm"
    },
        {
          "name": {
            "ger": "CO2",
            "fin": "CO2",
            "en": "CO2"
          },
          "value": ")" + std::to_string(m_Co2)
                       + R"(",
          "unit": "ppm"
        },
        {
          "name": {
            "ger": "Lichtstärke",
            "fin": "Valonvoimakkuus",
            "en": "Illumination"
          },
          "value": ")" + std::to_string(m_Illumination)
                       + R"(",
          "unit": "lux"
        },
        {
          "name": {
            "ger": "Lautstärke",
            "fin": "Volume",
            "en": "Noise"
          },
          "value": ")" + std::to_string(m_Noise)
                       + R"(",
          "unit": "dBA"
        },
        {
          "name": {
            "ger": "Partikel",
            "fin": "Hiukkaset",
            "en": "Particles"
          },
         "value": "",
          "unit": ""
        },
{
          "name": {
            "ger": "Partikelgröße",
            "fin": "Hiukkaskoko",
            "en": "Particle size"
          },
          "value": ")" + std::to_string(m_partSize)
                       + R"(",
          "unit": "um"
        },

{
       "name": {
         "ger": "Anzahl",
         "fin": "Määrä",
         "en": "Number"
       },
       "value": "",
       "unit": ""
     },
      {
       "name": {
         "ger": "PM1.0",
         "fin": "PM1.0",
         "en": "PM1.0"
       },
       "value": ")" + std::to_string(m_number1_0)
                       + R"(",
       "unit": "N/cm³"
     },
      {
      "name": {
      "ger": "PM2.5",
      "fin": "PM2.5",
      "en": "PM2.5"
      },
      "value": ")" + std::to_string(m_number2_5)
                       + R"(",
      "unit": "N/cm³"
      },
      {
      "name": {
        "ger": "PM4.0",
        "fin": "PM4.0",
        "en": "PM4.0"
      },
      "value": ")" + std::to_string(m_number4_0)
                       + R"(",
      "unit": "N/cm³"
    },
{
      "name": {
        "ger": "PM10.0",
        "fin": "PM10.0",
        "en": "PM10.0"
      },
      "value": ")" + std::to_string(m_number10_0)
                       + R"(",
      "unit": "N/cm³"
    },
    {
         "name": {
           "ger": "Masse",
           "fin": "Massa",
           "en": "Mass"
         },
         "value": "",
         "unit": ""
       },
        {
         "name": {
           "ger": "PM1.0",
           "fin": "PM1.0",
           "en": "PM1.0"
         },
         "value": ")" + std::to_string(m_mass1_0)
                       + R"(",
         "unit": "ug/m³"
       },
        {
        "name": {
        "ger": "PM2.5",
        "fin": "PM2.5",
        "en": "PM2.5"
        },
        "value": ")" + std::to_string(m_mass2_5)
                       + R"(",
        "unit": "ug/m³"
        },
        {
        "name": {
          "ger": "PM4.0",
          "fin": "PM4.0",
          "en": "PM4.0"
        },
        "value": ")" + std::to_string(m_mass4_0)
                       + R"(",
        "unit": "ug/m³"
      },
{
        "name": {
          "ger": "PM10.0",
          "fin": "PM10.0",
          "en": "PM10.0"
        },
        "value": ")" + std::to_string(m_mass10_0)
                       + R"(",
        "unit": "ug/m³"
      }
      ])"};
}
} // namespace Sensor