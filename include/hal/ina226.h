#pragma once

#include <Wire.h>
#include "ISensor.h"
#include "board.h"

// INA226 Registers
#define INA226_REG_CONFIG           0x00
#define INA226_REG_SHUNT_VOLTAGE    0x01
#define INA226_REG_BUS_VOLTAGE      0x02
#define INA226_REG_POWER            0x03
#define INA226_REG_CURRENT          0x04
#define INA226_REG_CALIBRATION      0x05

// Configuration settings
#define INA226_CALIBRATION_VALUE    3413  // Based on 0.00075Ω shunt, 0.002A LSB
#define INA226_BUS_RANGE            32    // 32V range
#define INA226_GAIN               1       // Gain 1 (x1)

class INA226 : public ISensor {
public:
    INA226() : lastGoodVoltage(0.0), lastGoodCurrent(0.0), lastGoodPower(0.0), lastGoodTemp(0.0) {}

    void begin() {
        Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
        Wire.setTimeOut(100); // Prevent infinite hangs on I2C noise

        // Set calibration
        writeRegister(INA226_REG_CALIBRATION, INA226_CALIBRATION_VALUE);

        // Configure: 32V range, x1 gain, 1.1ms avg, 1.1ms bus, 1.1ms shunt, continuous mode
        uint16_t config = (0b000 << 13) | // 32V range
                          (0b000 << 11) | // x1 gain
                          (0b100 << 8)  | // 1.1ms avg
                          (0b100 << 5)  | // 1.1ms bus
                          (0b100 << 2)  | // 1.1ms shunt
                          (0b11);           // Continuous mode
        writeRegister(INA226_REG_CONFIG, config);
    }

    double readVoltage() override {
        uint16_t raw = readRegister(INA226_REG_BUS_VOLTAGE);
        double voltage = (raw * 1.25) / 1000.0; // 1.25mV per bit

        // Validate: reject out-of-range values
        if (voltage > 15.0 || voltage < 5.0) {
            return lastGoodVoltage;
        }
        lastGoodVoltage = voltage;
        return voltage;
    }

    double readCurrent() override {
        int16_t raw = readRegister(INA226_REG_CURRENT);
        double current = (raw * 0.002);

        lastGoodCurrent = current;
        return current;
    }

    double readPower() override {
        uint16_t raw = readRegister(INA226_REG_POWER);
        double power = (raw * 0.004); // 4mW per bit

        lastGoodPower = power;
        return power;
    }

    double readTemperature() override {
        // Not implemented — use ESP32 internal temp sensor in main
        return lastGoodTemp;
    }

    void update() override {
        // Update all readings
        readVoltage();
        readCurrent();
        readPower();
    }

    double getLastKnownVoltage() const override { return lastGoodVoltage; }
    double getLastKnownCurrent() const override { return lastGoodCurrent; }
    double getLastKnownPower() const override { return lastGoodPower; }
    double getLastKnownTemperature() const override { return lastGoodTemp; }

private:
    double lastGoodVoltage, lastGoodCurrent, lastGoodPower, lastGoodTemp;

    uint16_t readRegister(uint8_t reg) {
        Wire.beginTransmission(0x40);
        Wire.write(reg);
        Wire.endTransmission();
        Wire.requestFrom(0x40, 2);
        if (Wire.available() != 2) return 0;
        uint16_t value = (Wire.read() << 8) | Wire.read();
        return value;
    }

    void writeRegister(uint8_t reg, uint16_t value) {
        Wire.beginTransmission(0x40);
        Wire.write(reg);
        Wire.write(value >> 8);
        Wire.write(value & 0xFF);
        Wire.endTransmission();
    }
};