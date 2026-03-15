#pragma once

// Interface for hardware sensors to enable mocking in tests
class ISensor {
public:
    virtual ~ISensor() = default;

    // Read voltage in volts
    virtual double readVoltage() = 0;

    // Read current in amps
    virtual double readCurrent() = 0;

    // Read power in watts
    virtual double readPower() = 0;

    // Read internal temperature in Celsius
    virtual double readTemperature() = 0;

    // Update sensor readings (called periodically)
    virtual void update() = 0;

    // Get last known good values (for fallback)
    virtual double getLastKnownVoltage() const = 0;
    virtual double getLastKnownCurrent() const = 0;
    virtual double getLastKnownPower() const = 0;
    virtual double getLastKnownTemperature() const = 0;
};