#pragma once

#include <Wire.h>
#include "../hal/isensor.h"
#include "../hal/board.h"
#include "../config.h"

// INA226 Registers
#define INA226_REG_CONFIG           0x00
#define INA226_REG_SHUNT_VOLTAGE    0x01
#define INA226_REG_BUS_VOLTAGE      0x02
#define INA226_REG_POWER            0x03
#define INA226_REG_CURRENT          0x04
#define INA226_REG_CALIBRATION      0x05

class INA226 : public ISensor {
public:
    INA226() : lastGoodVoltage(0.0), lastGoodCurrent(0.0), lastGoodPower(0.0), lastGoodTemp(0.0) {}

    void begin() {
        Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
        Wire.setTimeOut(I2C_TIMEOUT);

        writeRegister(INA226_REG_CALIBRATION, INA226_CALIBRATION);

        uint16_t config = (0b000 << 13) |
                          (0b000 << 11) |
                          (0b100 << 8)  |
                          (0b100 << 5)  |
                          (0b100 << 2)  |
                          (0b11);
        writeRegister(INA226_REG_CONFIG, config);
    }

    double readVoltage() override {
        uint16_t raw = readRegister(INA226_REG_BUS_VOLTAGE);
        double voltage = (raw * 1.25) / 1000.0;

        if (voltage > VOLTAGE_MAX_VALID || voltage < VOLTAGE_MIN_VALID) {
            return lastGoodVoltage;
        }
        lastGoodVoltage = voltage;
        return voltage;
    }

    double readCurrent() override {
        int16_t raw = readRegister(INA226_REG_CURRENT);
        double current = (raw * CURRENT_LSB);

        lastGoodCurrent = current;
        return current;
    }

    double readPower() override {
        uint16_t raw = readRegister(INA226_REG_POWER);
        double power = (raw * 0.004);

        lastGoodPower = power;
        return power;
    }

    double readTemperature() override {
        return lastGoodTemp;
    }

    void update() override {
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
        Wire.beginTransmission(INA226_ADDRESS);
        Wire.write(reg);
        Wire.endTransmission();
        Wire.requestFrom(INA226_ADDRESS, 2);
        if (Wire.available() != 2) return 0;
        uint16_t value = (Wire.read() << 8) | Wire.read();
        return value;
    }

    void writeRegister(uint8_t reg, uint16_t value) {
        Wire.beginTransmission(INA226_ADDRESS);
        Wire.write(reg);
        Wire.write(value >> 8);
        Wire.write(value & 0xFF);
        Wire.endTransmission();
    }
};