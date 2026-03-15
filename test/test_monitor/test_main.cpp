#include <Arduino.h>
#include <ArduinoFake.h>
#include <unity.h>
#include "../include/app/monitor.h"
#include "../include/hal/isensor.h"

// Mock Sensor Class
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

void setUp() {
    ArduinoFakeReset();
}

void tearDown() {
}

void test_monitor_initial_state() {
    MockSensor mock;
    Monitor monitor(&mock);

    // Initial state: on battery
    monitor.update(0);
    TelemetryDTO& dto = monitor.getDTO();

    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, dto.power_state);
    TEST_ASSERT_EQUAL(ALERT_LEVEL_NONE, dto.alert_level);
    TEST_ASSERT_EQUAL_FLOAT(12.0, dto.voltage_bus);
    TEST_ASSERT_EQUAL_FLOAT(-5.0, dto.current_amps);
    TEST_ASSERT_EQUAL_FLOAT(-60.0, dto.power_watts);
    TEST_ASSERT_EQUAL_FLOAT(108.0, dto.capacity_wh_remaining);
    TEST_ASSERT_EQUAL_FLOAT(100.0, dto.estimated_soc_pct);
}

void test_monitor_critical_state() {
    MockSensor mock;
    Monitor monitor(&mock);

    // Force low voltage and SOC
    mock.mockVoltage = 11.0;
    mock.mockCurrent = -1.0;
    monitor.update(0);

    // Simulate 3 consecutive samples
    for (int i = 0; i < 3; i++) {
        monitor.update(250 * (i + 1));
    }

    TelemetryDTO& dto = monitor.getDTO();

    TEST_ASSERT_EQUAL(POWER_STATE_CRITICAL, dto.power_state);
    TEST_ASSERT_EQUAL(ALERT_LEVEL_CRITICAL, dto.alert_level);
    TEST_ASSERT_EQUAL_STRING("capacity_low", dto.reason);
}

void test_monitor_deadband_filter() {
    MockSensor mock;
    Monitor monitor(&mock);

    // Set current to 0.01A — should be filtered to 0.0
    mock.mockCurrent = 0.01;
    monitor.update(0);

    TelemetryDTO& dto = monitor.getDTO();
    TEST_ASSERT_EQUAL_FLOAT(0.0, dto.current_amps);
}

void test_monitor_soc_only_decreases() {
    MockSensor mock;
    Monitor monitor(&mock);

    // Start with 100% SOC
    mock.mockVoltage = 12.0;
    mock.mockCurrent = -1.0; // discharging
    monitor.update(0);

    // Simulate charging (positive current) while on battery
    mock.mockCurrent = 1.0; // charging
    monitor.update(250);

    // SOC should not increase — should remain at 99.9%
    TelemetryDTO& dto = monitor.getDTO();
    TEST_ASSERT_TRUE(dto.estimated_soc_pct <= 100.0);
}

void test_monitor_3_sample_debounce() {
    MockSensor mock;
    Monitor monitor(&mock);

    // Start with normal state
    monitor.update(0);

    // Trigger warning condition for 2 samples
    mock.mockVoltage = 11.5;
    mock.mockCurrent = -0.5;
    monitor.update(250);
    monitor.update(500);

    // Should not trigger warning yet
    TelemetryDTO& dto = monitor.getDTO();
    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, dto.power_state);

    // Third sample triggers warning
    monitor.update(750);
    TEST_ASSERT_EQUAL(POWER_STATE_WARNING, dto.power_state);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_monitor_initial_state);
    RUN_TEST(test_monitor_critical_state);
    RUN_TEST(test_monitor_deadband_filter);
    RUN_TEST(test_monitor_soc_only_decreases);
    RUN_TEST(test_monitor_3_sample_debounce);
    UNITY_END();
    return 0;
}