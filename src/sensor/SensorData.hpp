#ifndef SENSORDATA_HPP
#define SENSORDATA_HPP

#include "rtos/semaphore/Mutex.hpp"

#include <string>

namespace Sensor
{

class SensorData
{
  public:
    void setTemperature(double temperature);
    void setHumidity(double humidity);
    void setPressure(double pressure);
    void setCo2(double co2);
    void setParticles(double mass1_0, double mass2_5, double mass4_0, double mass10_0,
                      double number1_0, double number2_5, double number4_0, double number10_0, double partSize);
    void setIllumination(double illumination);
    void setNoise(double noise);
    void setVoc(double voc);
    [[nodiscard]] std::string getJson() const;

  private:
    Rtos::Semaphore::Mutex m_Access{};
    double m_Temperature{21.3};
    double m_Humidity{50.5};
    double m_Pressure{500};
    double m_Co2{250};
    double m_mass1_0{0};
    double m_mass2_5{0};
    double m_mass4_0{0};
    double m_mass10_0{0};

    double m_number0_5{0};
    double m_number1_0{0};
    double m_number2_5{0};
    double m_number4_0{0};
    double m_number10_0{0};
    double m_partSize{0};

    double m_Illumination{255};
    double m_Noise{45};
    double m_Voc{50.0};

};

} // namespace Sensor

#endif /* SENSORDATA_HPP */
