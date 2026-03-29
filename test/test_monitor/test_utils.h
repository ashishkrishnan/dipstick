#pragma once
#include "app/monitor.h"
#include "hal/isensor.h"
#include "hal/temperature.h"

// Mock sensor for testing
class MockSensor : public ISensor {
public:
    double mockVoltage = 12.0;
    double mockCurrent = -5.0;
    double mockPower = -60.0;
    double mockTemp = 45.0;

    double readVoltage() override { return mockVoltage; }
    double readCurrent() override { return mockCurrent; }
    double readPower() override { return mockPower; }
    double readTemperature() override { return mockTemp; }
    void update() override {}
    double getLastKnownVoltage() const override { return mockVoltage; }
    double getLastKnownCurrent() const override { return mockCurrent; }
    double getLastKnownPower() const override { return mockPower; }
    double getLastKnownTemperature() const override { return mockTemp; }
};

// Mock temperature for testing
class MockTemperature : public ITemperature {
public:
    double mockTemp = 25.0;
    double readTemperature() override { return mockTemp; }
};
