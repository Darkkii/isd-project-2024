#include "SensorReading.hpp"

namespace Sensor
{

SensorReading::SensorReading(SensorType type, double value) :
    m_Type{type},
    m_Value{value}
{}

SensorType SensorReading::getType() const { return m_Type; }

double SensorReading::getValue() const { return m_Value; }

} // namespace Sensor
