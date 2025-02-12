#include "SensorReading.hpp"

namespace Sensor
{

SensorReading::SensorReading(SensorType type, double value) :
    m_Type{type},
    m_Value{value}
{}

SensorType SensorReading::type() const { return m_Type; }

double SensorReading::value() const { return m_Value; }

} // namespace Sensor
