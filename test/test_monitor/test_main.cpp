#include <Arduino.h>
#include <ArduinoFake.h>
#include <unity.h>
#include "app/monitor.h"
#include "test_utils.h"

void setUp() {
    ArduinoFakeReset();
}

void tearDown() {
}

void test_monitor_initial_state() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

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
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    mock.mockVoltage = 11.0;
    mock.mockCurrent = -1.0;
    mock.mockPower = -11.0;
    monitor.update(0);

    for (int i = 1; i <= 3; i++) {
        monitor.update(i * 250);
    }

    TelemetryDTO& dto = monitor.getDTO();

    TEST_ASSERT_EQUAL(POWER_STATE_CRITICAL, dto.power_state);
    TEST_ASSERT_EQUAL(ALERT_LEVEL_CRITICAL, dto.alert_level);
    TEST_ASSERT_EQUAL_STRING("capacity_low", dto.reason);
}

void test_monitor_deadband_filter() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    mock.mockCurrent = 0.01;
    mock.mockVoltage = 12.0;
    mock.mockPower = 0.12;
    monitor.update(0);

    TelemetryDTO& dto = monitor.getDTO();
    TEST_ASSERT_EQUAL_FLOAT(0.0, dto.current_amps);
}

void test_monitor_soc_only_decreases() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    mock.mockVoltage = 12.0;
    mock.mockCurrent = -1.0;
    mock.mockPower = -12.0;
    monitor.update(0);

    mock.mockCurrent = 1.0;
    mock.mockPower = 12.0;
    monitor.update(250);

    TelemetryDTO& dto = monitor.getDTO();
    TEST_ASSERT_TRUE(dto.estimated_soc_pct <= 100.0);
}

void test_monitor_3_sample_debounce() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Start with normal state (high SOC)
    mock.mockVoltage = 12.5;
    mock.mockCurrent = -5.0;
    mock.mockPower = -62.5;
    monitor.update(0);
    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, monitor.getDTO().power_state);

    // Now set low SOC to trigger warning condition
    mock.mockVoltage = 12.0;
    mock.mockCurrent = -0.5;
    mock.mockPower = -6.0;
    monitor.setCapacityWHRemaining(32.4);  // 30% of 108Wh
    monitor.setEstimatedSocPct(30.0);
    
    // First sample with warning condition - should still be ON_BATTERY (debounce)
    monitor.update(250);
    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, monitor.getDTO().power_state);
    
    // Second sample - still ON_BATTERY (need 3 consecutive)
    monitor.update(500);
    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, monitor.getDTO().power_state);

    // Third sample - now should trigger WARNING
    monitor.update(750);
    TEST_ASSERT_EQUAL(POWER_STATE_WARNING, monitor.getDTO().power_state);
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
