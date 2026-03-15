#pragma once

#include <Arduino.h>
#include "ISensor.h"

// State Machine States
enum PowerState {
    POWER_STATE_CHARGING,
    POWER_STATE_ON_BATTERY,
    POWER_STATE_CRITICAL,
    POWER_STATE_WARNING
};

enum AlertLevel {
    ALERT_LEVEL_NONE,
    ALERT_LEVEL_WARNING,
    ALERT_LEVEL_CRITICAL
};

// Data Transfer Object (DTO) for telemetry
struct TelemetryDTO {
    double voltage_bus;
    double current_amps;
    double power_watts;
    double capacity_wh_remaining;
    double estimated_soc_pct;
    double estimated_runtime_seconds;
    PowerState power_state;
    AlertLevel alert_level;
    const char* reason;
    uint32_t uptime_seconds;
    double internal_temp_c;
};

// Monitor class: handles coulomb counting, deadband, 3-sample debounce
class Monitor {
public:
    Monitor(ISensor* sensor) : sensor(sensor), lastUpdate(0), capacity_wh(108.0), capacity_wh_remaining(108.0), lastCurrent(0.0), sampleCount(0), lastState(POWER_STATE_ON_BATTERY) {}

    void update(uint32_t currentMillis) {
        if (currentMillis - lastUpdate < 250) return; // 250ms loop
        lastUpdate = currentMillis;

        sensor->update();

        double voltage = sensor->readVoltage();
        double current = sensor->readCurrent();
        double power = sensor->readPower();

        // Coulomb counting: integrate current over time (in Ah)
        // 250ms = 0.25s, current in A → charge in As = A·s → divide by 3600 to get Ah
        double charge_delta_ah = (current * 0.25) / 3600.0;
        capacity_wh_remaining -= (charge_delta_ah * voltage); // Wh = Ah × V

        // Enforce: SOC only decreases while on battery
        if (lastState == POWER_STATE_ON_BATTERY && current < 0) {
            double new_soc_pct = (capacity_wh_remaining / capacity_wh) * 100.0;
            if (new_soc_pct > estimated_soc_pct) {
                capacity_wh_remaining = (estimated_soc_pct / 100.0) * capacity_wh;
            } else {
                estimated_soc_pct = new_soc_pct;
            }
        } else {
            estimated_soc_pct = (capacity_wh_remaining / capacity_wh) * 100.0;
        }

        // Estimate runtime: remaining capacity / discharge rate
        if (current < 0) {
            estimated_runtime_seconds = (capacity_wh_remaining / (-power)) * 3600.0;
        } else {
            estimated_runtime_seconds = 0;
        }

        // 3-sample debounce for state transitions
        bool belowCritical = voltage <= 11.2 || estimated_soc_pct <= 10 || estimated_runtime_seconds <= 120;
        bool belowWarning = estimated_runtime_seconds <= 300 || estimated_soc_pct <= 30;

        if (belowCritical) {
            sampleCount++;
        } else {
            sampleCount = 0;
        }

        if (sampleCount >= 3) {
            if (lastState != POWER_STATE_CRITICAL) {
                lastState = POWER_STATE_CRITICAL;
                alert_level = ALERT_LEVEL_CRITICAL;
                reason = "capacity_low";
            }
        } else if (belowWarning) {
            sampleCount++;
            if (sampleCount >= 3 && lastState != POWER_STATE_WARNING) {
                lastState = POWER_STATE_WARNING;
                alert_level = ALERT_LEVEL_WARNING;
                reason = "capacity_low";
            }
        } else if (voltage >= 13.2 && current > 0) {
            lastState = POWER_STATE_CHARGING;
            alert_level = ALERT_LEVEL_NONE;
            reason = nullptr;
        } else {
            lastState = POWER_STATE_ON_BATTERY;
            alert_level = ALERT_LEVEL_NONE;
            reason = nullptr;
        }

        // Update DTO
        dto.voltage_bus = voltage;
        dto.current_amps = current;
        dto.power_watts = power;
        dto.capacity_wh_remaining = capacity_wh_remaining;
        dto.estimated_soc_pct = estimated_soc_pct;
        dto.estimated_runtime_seconds = estimated_runtime_seconds;
        dto.power_state = lastState;
        dto.alert_level = alert_level;
        dto.reason = reason;
    }

    TelemetryDTO& getDTO() { return dto; }

    void setUptime(uint32_t uptime) { dto.uptime_seconds = uptime; }
    void setInternalTemp(double temp) { dto.internal_temp_c = temp; }

private:
    ISensor* sensor;
    TelemetryDTO dto;
    uint32_t lastUpdate;
    double capacity_wh;
    double capacity_wh_remaining;
    double estimated_soc_pct;
    double estimated_runtime_seconds;
    double lastCurrent;
    int sampleCount;
    PowerState lastState;
    AlertLevel alert_level;
    const char* reason;
};