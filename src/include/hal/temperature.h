#pragma once

// Interface for temperature reading to enable mocking in tests
class ITemperature {
public:
    virtual ~ITemperature() = default;
    virtual double readTemperature() = 0;
};

// ESP32 implementation
class ESP32Temperature : public ITemperature {
public:
    double readTemperature() override {
#ifdef ARDUINO_ARCH_ESP32
        return temperatureRead();
#else
        return 25.0;  // Default for native tests
#endif
    }
};

// Mock implementation for testing
class MockTemperature : public ITemperature {
public:
    double mockTemp = 25.0;
    double readTemperature() override { return mockTemp; }
};
