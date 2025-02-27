#include "SensorData.hpp"

#include <mutex>

namespace Sensor
{

void SensorData::setTemperature(double temperature)
{
    std::lock_guard lock(m_Access);
    m_Temperature = temperature;
}

void SensorData::setHumidity(double humidity)
{
    std::lock_guard lock(m_Access);
    m_Humidity = humidity;
}

void SensorData::setPressure(double pressure)
{
    std::lock_guard lock(m_Access);
    m_Pressure = pressure;
}

void SensorData::setCo2(double co2)
{
    std::lock_guard lock(m_Access);
    m_Co2 = co2;
}

void SensorData::setParticles(double particles)
{
    std::lock_guard lock(m_Access);
    m_Particles = particles;
}

void SensorData::setIllumination(double illumination)
{
    std::lock_guard lock(m_Access);
    m_Illumination = illumination;
}

void SensorData::setNoise(double noise)
{
    std::lock_guard lock(m_Access);
    m_Noise = noise;
}

void SensorData::setVoc(double voc)
{
    std::lock_guard lock(m_Access);
    m_Voc = voc;
}

std::string SensorData::getJson() const
{
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
            "ger": "???",
            "fin": "???",
            "en": "Particles"
          },
          "value": ")" + std::to_string(m_Particles)
                       + R"(",
          "unit": "???"
        },
        {
          "name": {
            "ger": "???",
            "fin": "???",
            "en": "Illumination"
          },
          "value": ")" + std::to_string(m_Illumination)
                       + R"(",
          "unit": "lux"
        },
        {
          "name": {
            "ger": "???",
            "fin": "???",
            "en": "Noise"
          },
          "value": ")" + std::to_string(m_Noise)
                       + R"(",
          "unit": "???"
        },
        {
          "name": {
            "ger": "???",
            "fin": "???",
            "en": "VOC"
          },
          "value": ")" + std::to_string(m_Voc)
                       + R"(",
          "unit": "???"
        }
      ])"};
}
} // namespace Sensor