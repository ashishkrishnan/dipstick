#include <unity.h>
#include "../../src/include/app/monitor.h"
#include "test_utils.h"

void test_warning_debounce_requires_3_samples() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Normal state
    mock.mockVoltage = 12.5;
    mock.mockCurrent = -5.0;
    mock.mockPower = -62.5;
    monitor.update(0);
    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, monitor.getDTO().power_state);
    
    // Trigger warning condition for 2 samples
    mock.mockVoltage = 11.5;
    mock.mockCurrent = -0.5;
    mock.mockPower = -5.75;
    monitor.update(250);
    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, monitor.getDTO().power_state);
    
    monitor.update(500);
    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, monitor.getDTO().power_state);
    
    // Third sample triggers warning
    monitor.update(750);
    TEST_ASSERT_EQUAL(POWER_STATE_WARNING, monitor.getDTO().power_state);
}

void test_critical_debounce_requires_3_samples() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Normal state
    mock.mockVoltage = 12.5;
    mock.mockCurrent = -5.0;
    mock.mockPower = -62.5;
    monitor.update(0);
    
    // Trigger critical condition for 2 samples
    mock.mockVoltage = 11.0;
    mock.mockCurrent = -1.0;
    mock.mockPower = -11.0;
    monitor.update(250);
    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, monitor.getDTO().power_state);
    
    monitor.update(500);
    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, monitor.getDTO().power_state);
    
    // Third sample triggers critical
    monitor.update(750);
    TEST_ASSERT_EQUAL(POWER_STATE_CRITICAL, monitor.getDTO().power_state);
    TEST_ASSERT_EQUAL(ALERT_LEVEL_CRITICAL, monitor.getDTO().alert_level);
}

void test_debounce_resets_on_condition_clear() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Trigger warning condition for 2 samples
    mock.mockVoltage = 11.5;
    mock.mockCurrent = -0.5;
    mock.mockPower = -5.75;
    monitor.update(0);
    monitor.update(250);
    
    // Condition clears before 3rd sample
    mock.mockVoltage = 12.5;
    mock.mockCurrent = -5.0;
    mock.mockPower = -62.5;
    monitor.update(500);
    
    // Should reset to normal
    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, monitor.getDTO().power_state);
}
