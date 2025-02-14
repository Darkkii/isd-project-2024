#ifndef MHZ19CSENSOR_H
#define MHZ19CSENSOR_H

#include <memory>
#include <cstdint>
#include <uart/PicoOsUart.hpp>

/**
 * A driver for the MH-Z19C CO2 sensor.
 *
 * Now includes a logUart pointer to send debug/log output
 * separate from the sensorUart used for actual sensor communication.
 */
class MHZ19CSensor {
public:
    /**
     * Constructor:
     *   @param sensorUart  The UART to communicate with the MH-Z19C (9600 baud).
     *   @param logUart     An optional UART for printing debug logs. Can be nullptr if not needed.
     */
    MHZ19CSensor(std::shared_ptr<Uart::PicoOsUart> sensorUart,
                 std::shared_ptr<Uart::PicoOsUart> logUart = nullptr);
    ~MHZ19CSensor() = default;

    // Optional init step: e.g. wait for warmup or set default calibration
    bool initSensor();

    // The actual reading (command 0x86)
    bool readSensor();
    int getCO2() const { return m_co2ppm; }

    // Additional sensor commands
    bool setAutoCalibration(bool enable);
    bool calibrateZero();
    bool calibrateSpan(uint16_t ppm);

private:
    // Separate UARTs
    std::shared_ptr<Uart::PicoOsUart> m_sensorUart;
    std::shared_ptr<Uart::PicoOsUart> m_logUart;

    int m_co2ppm = 0;

    // Common routines
    bool sendCommandAndGetReply(const uint8_t cmd[9], uint8_t reply[9], uint32_t readTimeoutMs);
    uint8_t calculateChecksum(const uint8_t packet[9]);
};

#endif // MHZ19CSENSOR_H
