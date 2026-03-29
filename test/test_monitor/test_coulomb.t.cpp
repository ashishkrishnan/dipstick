#include <unity.h>
#include "../../src/include/app/monitor.h"
#include "test_utils.h"

void test_coulomb_counting_discharge() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Initial state
    monitor.update(0);
    TelemetryDTO& dto1 = monitor.getDTO();
    double initialCapacity = dto1.capacity_wh_remaining;
    
    // Discharge: -60W at 12V for 250ms
    mock.mockCurrent = -5.0;
    mock.mockVoltage = 12.0;
    mock.mockPower = -60.0;
    monitor.update(250);
    TelemetryDTO& dto2 = monitor.getDTO();
    
    // Energy consumed: 60W * (250/3600)h = 4.17 Wh
    // Capacity should decrease from 108 to ~103.83 Wh
    TEST_ASSERT_LESS_THAN(initialCapacity, dto2.capacity_wh_remaining);
}

void test_coulomb_counting_charge() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Initial state
    monitor.update(0);
    TelemetryDTO& dto1 = monitor.getDTO();
    double initialCapacity = dto1.capacity_wh_remaining;
    
    // Charge: +65W at 13V for 250ms
    mock.mockCurrent = 5.0;
    mock.mockVoltage = 13.0;
    mock.mockPower = 65.0;
    monitor.update(250);
    TelemetryDTO& dto2 = monitor.getDTO();
    
    // Energy added: 65W * (250/3600)h = 4.5 Wh
    // Capacity should increase
    TEST_ASSERT_GREATER_THAN(initialCapacity, dto2.capacity_wh_remaining);
}

void test_coulomb_deadband_filter() {
    MockSensor mock;
    MockTemperature mockTemp;
    Monitor monitor(&mock, &mockTemp);

    // Current below deadband threshold
    mock.mockCurrent = 0.01;
    mock.mockVoltage = 12.0;
    mock.mockPower = 0.12;
    monitor.update(0);
    
    TelemetryDTO& dto = monitor.getDTO();
    TEST_ASSERT_EQUAL_FLOAT(0.0, dto.current_amps);
}
