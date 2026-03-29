#include <unity.h>
#include "app/monitor.h"
#include "test_utils.h"

void test_soc_decreases_on_discharge() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Start at 100%
    mock.mockCurrent = 0.0;
    mock.mockVoltage = 12.0;
    mock.mockPower = 0.0;
    monitor.update(0);
    TelemetryDTO& dto1 = monitor.getDTO();
    TEST_ASSERT_EQUAL_FLOAT(100.0, dto1.estimated_soc_pct);
    
    // Discharge for 5 seconds
    mock.mockCurrent = -5.0;
    mock.mockPower = -60.0;
    for (int i = 1; i <= 5; i++) {
        monitor.update(i * 250);
    }
    
    TelemetryDTO& dto2 = monitor.getDTO();
    TEST_ASSERT_LESS_THAN(dto1.estimated_soc_pct, dto2.estimated_soc_pct);
}

void test_soc_capped_at_100_during_charge() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Start at 100%
    mock.mockCurrent = 0.0;
    mock.mockVoltage = 12.0;
    mock.mockPower = 0.0;
    monitor.update(0);
    TelemetryDTO& dto1 = monitor.getDTO();
    TEST_ASSERT_EQUAL_FLOAT(100.0, dto1.estimated_soc_pct);
    
    // Charge for 5 seconds
    mock.mockCurrent = 5.0;
    mock.mockVoltage = 13.0;
    mock.mockPower = 65.0;
    for (int i = 1; i <= 5; i++) {
        monitor.update(i * 250);
    }
    
    TelemetryDTO& dto2 = monitor.getDTO();
    TEST_ASSERT_LESS_THAN_OR_EQUAL(100.0, dto2.estimated_soc_pct);
    TEST_ASSERT_EQUAL_FLOAT(100.0, dto2.estimated_soc_pct);
}
