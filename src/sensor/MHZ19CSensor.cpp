#include "MHZ19CSensor.h"

#include <cstring>
#include <cstdio>   // for sprintf/printf if needed
#include "FreeRTOS.h"
#include "task.h"

MHZ19CSensor::MHZ19CSensor(std::shared_ptr<Uart::PicoOsUart> sensorUart,
                           std::shared_ptr<Uart::PicoOsUart> logUart)
    : m_sensorUart(sensorUart),
      m_logUart(logUart),
      m_co2ppm(0)
{
}

bool MHZ19CSensor::initSensor() {
    // Typical warmup recommendation: ~1 minute
    vTaskDelay(pdMS_TO_TICKS(60000));

    // Optionally disable auto-calibration
    setAutoCalibration(false);

    if (m_logUart) {
        m_logUart->send("[MHZ19CSensor] Finished initSensor() warmup.\r\n");
    }
    return true;
}

bool MHZ19CSensor::readSensor() {
    // 9-byte command: 0x86
    uint8_t cmd[9] = {0xFF, 0x01, 0x86, 0,0,0,0,0,0};
    cmd[8] = calculateChecksum(cmd);  // Some sensor docs require you to do this

    uint8_t resp[9] = {0};

    if (m_logUart) {
        m_logUart->send("[MHZ19CSensor] Sending readSensor() command...\r\n");
    }
    bool ok = sendCommandAndGetReply(cmd, resp, 200);

    if (!ok) {
        if (m_logUart) {
            m_logUart->send("[MHZ19CSensor] No valid response from sensor.\r\n");
        }
        return false;
    }

    // Check the structure
    if (resp[0] == 0xFF && resp[1] == 0x86) {
        // Validate checksum
        uint8_t cs = calculateChecksum(resp);
        if (cs == resp[8]) {
            m_co2ppm = (resp[2] << 8) | resp[3];
            if (m_logUart) {
                char msg[64];
                snprintf(msg, sizeof(msg), "[MHZ19CSensor] CO2 = %d ppm\r\n", m_co2ppm);
                m_logUart->send(msg);
            }
            return true;
        }
    }

    if (m_logUart) {
        m_logUart->send("[MHZ19CSensor] Response format/checksum invalid.\r\n");
    }
    return false;
}

bool MHZ19CSensor::setAutoCalibration(bool enable) {
    uint8_t cmd[9] = {0xFF, 0x01, 0x79, 0x00,0,0,0,0,0};
    cmd[3] = enable ? 0xA0 : 0x00;
    cmd[8] = calculateChecksum(cmd);

    uint8_t resp[9];
    bool ok = sendCommandAndGetReply(cmd, resp, 200);
    if (m_logUart) {
        m_logUart->send(enable
            ? "[MHZ19CSensor] Auto-calibration ON command sent.\r\n"
            : "[MHZ19CSensor] Auto-calibration OFF command sent.\r\n");
    }
    return ok;
}

bool MHZ19CSensor::calibrateZero() {
    uint8_t cmd[9] = {0xFF, 0x01, 0x87, 0x00,0,0,0,0,0};
    cmd[8] = calculateChecksum(cmd);

    uint8_t resp[9];
    bool ok = sendCommandAndGetReply(cmd, resp, 200);
    if (m_logUart) {
        m_logUart->send("[MHZ19CSensor] Zero calibration command sent.\r\n");
    }
    return ok;
}

bool MHZ19CSensor::calibrateSpan(uint16_t ppm) {
    uint8_t hi = (ppm >> 8) & 0xFF;
    uint8_t lo = (ppm & 0xFF);

    uint8_t cmd[9] = {0xFF,0x01,0x88, hi,lo, 0,0,0,0};
    cmd[8] = calculateChecksum(cmd);

    uint8_t resp[9];
    bool ok = sendCommandAndGetReply(cmd, resp, 200);
    if (m_logUart) {
        char msg[80];
        snprintf(msg, sizeof(msg),
                 "[MHZ19CSensor] Span calibration at %u ppm command sent.\r\n",
                 ppm);
        m_logUart->send(msg);
    }
    return ok;
}

bool MHZ19CSensor::sendCommandAndGetReply(const uint8_t cmd[9],
                                          uint8_t reply[9],
                                          uint32_t readTimeoutMs)
{
    // 1) flush old data
    m_sensorUart->flush();

    // 2) small delay to ensure flush completed
    vTaskDelay(pdMS_TO_TICKS(10));

    // 3) send
    int written = m_sensorUart->write(cmd, 9, pdMS_TO_TICKS(100));
    if (m_logUart) {
        char msg[64];
        snprintf(msg, sizeof(msg), "[MHZ19CSensor] wrote %d/9 bytes.\r\n", written);
        m_logUart->send(msg);
    }

    // optional small delay for sensor response
    vTaskDelay(pdMS_TO_TICKS(100));

    // 4) read 9 bytes
    int got = m_sensorUart->read(reply, 9, pdMS_TO_TICKS(readTimeoutMs));
    if (m_logUart) {
        char msg[64];
        snprintf(msg, sizeof(msg), "[MHZ19CSensor] read %d bytes.\r\n", got);
        m_logUart->send(msg);
    }

    // Return true if we got 9 bytes
    return (got == 9);
}

uint8_t MHZ19CSensor::calculateChecksum(const uint8_t packet[9]) {
    // Per sensor docs:
    //   Checksum = 0xFF - (sum of bytes[1..7]) + 1
    uint8_t sum = 0;
    for (int i = 1; i < 8; i++) {
        sum += packet[i];
    }
    return 0xFF - sum + 1;
}
