#include <unity.h>
#include "../../src/include/app/monitor.h"
#include "test_utils.h"

void test_charging_state_detection() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Normal discharge
    mock.mockVoltage = 12.0;
    mock.mockCurrent = -5.0;
    mock.mockPower = -60.0;
    monitor.update(0);
    TEST_ASSERT_EQUAL(POWER_STATE_CHARGING, monitor.getDTO().power_state);
    
    // Switch to charging
    mock.mockVoltage = 13.5;
    mock.mockCurrent = 2.0;
    mock.mockPower = 27.0;
    monitor.update(250);
    
    TEST_ASSERT_EQUAL(POWER_STATE_CHARGING, monitor.getDTO().power_state);
    TEST_ASSERT_EQUAL(ALERT_LEVEL_NONE, monitor.getDTO().alert_level);
}

void test_on_battery_safe_state() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Normal discharge (not critical or warning)
    mock.mockVoltage = 12.5;
    mock.mockCurrent = -5.0;
    mock.mockPower = -62.5;
    monitor.update(0);
    
    TelemetryDTO& dto = monitor.getDTO();
    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, dto.power_state);
    TEST_ASSERT_EQUAL(ALERT_LEVEL_NONE, dto.alert_level);
}
