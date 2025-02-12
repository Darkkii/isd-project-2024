#ifndef SENSORREADING_HPP
#define SENSORREADING_HPP

#include <cstdint>

namespace Sensor
{

enum SensorType : uint8_t
{
    TEMPERATURE,
    HUMIDITY,
    PRESSURE
};

class SensorReading
{
  public:
    SensorReading(SensorType type, double value);
    [[nodiscard]] SensorType type() const;
    [[nodiscard]] double value() const;

  private:
    const SensorType m_Type{};
    const double m_Value{};
};

} // namespace Sensor

#endif /* SENSORREADING_HPP */
