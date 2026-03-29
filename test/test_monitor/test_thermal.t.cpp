#include <unity.h>
#include "app/monitor.h"
#include "test_utils.h"

void test_thermal_override_critical() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Normal temperature
    mockTemp.mockTemp = 25.0;
    mock.mockVoltage = 12.0;
    mock.mockCurrent = -5.0;
    mock.mockPower = -60.0;
    monitor.update(0);
    TelemetryDTO& dto1 = monitor.getDTO();
    TEST_ASSERT_EQUAL(POWER_STATE_ON_BATTERY, dto1.power_state);
    TEST_ASSERT_EQUAL(ALERT_LEVEL_NONE, dto1.alert_level);
    
    // Set to critical temperature
    mockTemp.mockTemp = 80.0;
    monitor.update(250);
    TelemetryDTO& dto2 = monitor.getDTO();
    
    TEST_ASSERT_EQUAL(POWER_STATE_CRITICAL, dto2.power_state);
    TEST_ASSERT_EQUAL(ALERT_LEVEL_CRITICAL, dto2.alert_level);
    TEST_ASSERT_EQUAL_STRING("overheat", dto2.reason);
}

void test_thermal_override_bypasses_debounce() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Normal state
    mockTemp.mockTemp = 25.0;
    mock.mockVoltage = 12.0;
    mock.mockCurrent = -5.0;
    mock.mockPower = -60.0;
    monitor.update(0);
    
    // Set to critical temperature - should be immediate
    mockTemp.mockTemp = 76.0;
    monitor.update(250);
    
    TelemetryDTO& dto = monitor.getDTO();
    TEST_ASSERT_EQUAL(POWER_STATE_CRITICAL, dto.power_state);
    TEST_ASSERT_EQUAL_STRING("overheat", dto.reason);
}
