# Project Dipstick - Examples & Reference

## MQTT Payload Examples

### Normal Operation (On Battery)
```json
{
  "system": {
    "uptime_seconds": 86400,
    "uptime_human": "1d 00h 00m 00s",
    "internal_temp_c": 48.5,
    "free_heap_bytes": 284520
  },
  "network": {
    "ip_address": "192.168.1.115",
    "wifi_rssi": -62,
    "publish_interval_ms": 2000
  },
  "battery": {
    "voltage_bus": 12.15,
    "current_amps": -25.40,
    "power_watts": -308.6,
    "capacity_wh_remaining": 83.5,
    "estimated_soc_pct": 76,
    "estimated_runtime_seconds": 950
  },
  "status": {
    "power_state": "on_battery",
    "alert_level": "none",
    "reason": null
  }
}
```

### Warning State (Low Capacity)
```json
{
  "system": {
    "uptime_seconds": 43200,
    "uptime_human": "12h 00m 00s",
    "internal_temp_c": 52.3,
    "free_heap_bytes": 275000
  },
  "network": {
    "ip_address": "192.168.1.115",
    "wifi_rssi": -65,
    "publish_interval_ms": 2000
  },
  "battery": {
    "voltage_bus": 11.8,
    "current_amps": -15.2,
    "power_watts": -179.4,
    "capacity_wh_remaining": 32.4,
    "estimated_soc_pct": 30,
    "estimated_runtime_seconds": 650
  },
  "status": {
    "power_state": "on_battery",
    "alert_level": "warning",
    "reason": "capacity_low"
  }
}
```

### Critical State (Panic)
```json
{
  "system": {
    "uptime_seconds": 7200,
    "uptime_human": "2h 00m 00s",
    "internal_temp_c": 78.2,
    "free_heap_bytes": 260000
  },
  "network": {
    "ip_address": "192.168.1.115",
    "wifi_rssi": -70,
    "publish_interval_ms": 10000
  },
  "battery": {
    "voltage_bus": 10.9,
    "current_amps": -8.5,
    "power_watts": -92.8,
    "capacity_wh_remaining": 10.8,
    "estimated_soc_pct": 10,
    "estimated_runtime_seconds": 120
  },
  "status": {
    "power_state": "critical",
    "alert_level": "critical",
    "reason": "overheat"
  }
}
```

### Charging State
```json
{
  "system": {
    "uptime_seconds": 172800,
    "uptime_human": "2d 00h 00m 00s",
    "internal_temp_c": 45.0,
    "free_heap_bytes": 290000
  },
  "network": {
    "ip_address": "192.168.1.115",
    "wifi_rssi": -60,
    "publish_interval_ms": 2000
  },
  "battery": {
    "voltage_bus": 13.6,
    "current_amps": 5.2,
    "power_watts": 70.7,
    "capacity_wh_remaining": 105.0,
    "estimated_soc_pct": 97,
    "estimated_runtime_seconds": 0
  },
  "status": {
    "power_state": "charging",
    "alert_level": "none",
    "reason": null
  }
}
```

## Configuration Examples

### PlatformIO Build Flags
```ini
[env:esp32-s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
board_build.mcu = esp32s3
build_flags = 
    -D BOARD_HAS_PSRAM
    -Isrc/include
lib_deps =
    robtillaart/INA226
    bblanchon/ArduinoJson
    knolleary/PubSubClient
```

### MQTT Subscription (Node-RED Example)
```javascript
// Subscribe to dipstick/state topic
// Trigger server shutdown when alert_level == "critical"

msg.payload = {
    "topic": "dipstick/state",
    "payload": JSON.parse(msg.payload)
};

if (msg.payload.status.alert_level === "critical") {
    // Initiate graceful shutdown
    return { topic: "server/shutdown", payload: "dipstick_critical" };
}

return msg;
```

### Home Assistant Configuration
```yaml
# configuration.yaml
sensor:
  - platform: mqtt
    name: "Dipstick Battery Voltage"
    state_topic: "dipstick/state"
    value_template: "{{ value_json.battery.voltage_bus }}"
    unit_of_measurement: "V"
    
  - platform: mqtt
    name: "Dipstick SOC"
    state_topic: "dipstick/state"
    value_template: "{{ value_json.battery.estimated_soc_pct }}"
    unit_of_measurement: "%"
    
  - platform: mqtt
    name: "Dipstick Alert Level"
    state_topic: "dipstick/state"
    value_template: "{{ value_json.status.alert_level }}"
```

## State Machine Transitions

```
                    ┌─────────────┐
                    │   Charging  │
                    │ (voltage≥13.2│
                    │  current>0) │
                    └──────┬──────┘
                           │
                           ▼
                    ┌─────────────┐
        ┌──────────►│ On Battery  │◄──────────┐
        │           │  (Safe)     │           │
        │           └──────┬──────┘           │
        │                  │                  │
        │  (3 samples)     │ (3 samples)      │
        │  voltage<11.5    │ voltage<11.2     │
        │  or SOC≤30%      │ or SOC≤10%       │
        │  or runtime≤300s │ or runtime≤120s  │
        │                  │                  │
        ▼                  ▼                  │
┌─────────────┐    ┌─────────────┐           │
│  Warning    │    │  Critical   │───────────┘
│ (Alert)     │    │ (Panic)     │  (Mains   │
└─────────────┘    └─────────────┘   Restored)
        ▲                  ▲
        │                  │
        └──────(Mains──────┘
             Restored)

Special: Any state ──► Critical (if temp > 75°C)
```

## Coulomb Counting Formula

```
Energy (Wh) = Power (W) × Time (h)
            = Voltage (V) × Current (A) × Time (h)

Capacity Update:
  capacity_wh_remaining += (current × voltage × time_delta_hours)

Where:
  - current < 0 (discharging): capacity decreases
  - current > 0 (charging): capacity increases
  - time_delta = 0.25 seconds = 0.25/3600 hours

Deadband Filter:
  if |current| < 0.02A:
    current = 0.0  // Prevent drift
```

## Thermal Throttling

```
Temperature Range      Publish Interval    State Override
─────────────────────────────────────────────────────────
< 65°C                2000ms (2s)        Normal
65°C - 75°C           5000ms (5s)        Normal
> 75°C                10000ms (10s)      Force CRITICAL
```

## Exponential Backoff (MQTT Reconnect)

```
Attempt    Delay    Cumulative    Notes
─────────────────────────────────────────
1          1s       1s            First attempt
2          2s       3s            
3          4s       7s            
4          8s       15s           
5          16s      31s           Max attempts reached
6+         60s      91s+          Reset counter, wait 1 minute

Max delay capped at 60 seconds.
Max 5 attempts per minute.
```
