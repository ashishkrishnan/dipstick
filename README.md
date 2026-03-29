# Project Dipstick

An industrial-grade ESP32-based UPS battery monitor that bypasses the UPS's internal logic to provide raw, highly accurate telemetry (Voltage, Current, Wattage, and State of Charge) via an external shunt. Uses coulomb counting and a multi-variable state machine to trigger automated server shutdowns via MQTT.

## Quick Start

### Prerequisites
- ESP32-S3 DevKitC board
- INA226 power sensor with 100A/75mV shunt
- PlatformIO (recommended) or Arduino IDE
- MQTT broker (e.g., Mosquitto)

### PlatformIO Setup
```bash
# Clone repository
git clone <repository-url>
cd dipstick

# Install dependencies
pio pkg install

# Build for ESP32-S3
pio run -e esp32-s3-devkitc-1

# Run native tests
pio test -e native
```

### Arduino IDE Setup
**Note:** Arduino IDE requires headers to be in the sketch root or `src/` folder.

1. Copy `src/dipstick.ino` to your Arduino sketch folder
2. Copy all files from `src/include/` directly into the sketch folder (flatten structure)
3. Rename `wifi_manager.h` if you encounter naming conflicts
4. Install required libraries:
   - INA226 (by robtillaart)
   - ArduinoJson (by bblanchon)
   - PubSubClient (by knolleary)
5. Select board: ESP32-S3 DevKitC
6. Upload

## Features

### Core Capabilities
- **Real-time Telemetry**: Voltage, current, power, and State of Charge (SOC)
- **Coulomb Counting**: Accurate battery capacity tracking with deadband filtering
- **State Machine**: Multi-variable debounce for stable state transitions
- **MQTT Integration**: Publishes comprehensive system status to configured topic

### Safety Features
- **3-Sample Debounce**: Prevents false triggers from transient conditions
- **Thermal Override**: Forces critical state when temperature exceeds 75°C
- **Thermal Throttling**: Reduces publish frequency at high temperatures
- **I2C Noise Protection**: Timeout and validation to prevent hangs
- **Watchdog Timer**: Prevents system lockup

### Implemented Logic
- Coulomb counting with proper energy calculation
- SOC capping at 100% during charging
- Deadband filter (< 0.02A treated as 0A)
- Exponential backoff for MQTT reconnection (1s → 2s → 4s → max 60s)
- Thermal override bypasses all debounce logic

## Architecture

```
dipstick/
├── src/
│   ├── include/
│   │   ├── hal/              # Hardware Abstraction Layer
│   │   │   ├── board.h       # Pin definitions and board config
│   │   │   ├── ina226.h      # INA226 sensor driver
│   │   │   ├── isensor.h     # Sensor interface (abstract)
│   │   │   └── temperature.h # Temperature sensor interface
│   │   ├── app/              # Application Logic
│   │   │   └── monitor.h     # State machine, coulomb counting, DTO
│   │   └── net/              # Networking
│   │       ├── mqtt_manager.h # MQTT client with backoff
│   │       └── wifi_manager.h # WiFi connection manager
│   └── dipstick.ino          # Main orchestration
├── test/
│   └── test_monitor/         # Unit tests
├── docs/
│   └── examples.md           # Configuration and usage examples
├── platformio.ini
└── README.md
```

### Data Flow
```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│  INA226     │───►│  Monitor    │───►│  MQTT       │
│  (Sensor)   │    │  (State     │    │  (Publish)  │
└─────────────┘    │  Machine)   │    └─────────────┘
                   └─────────────┘
                         ▲
                   ┌─────┴─────┐
                   │  Temperature│
                   └─────────────┘
```

## Hardware Requirements

### Specifications
| Parameter | Value |
|-----------|-------|
| **Shunt** | 100A / 75mV External Shunt (Low-Side) |
| **Max Capacity** | 108.0 Wh (12V 9Ah SLA Battery) |
| **I2C Pins** | GPIO8 (SDA), GPIO9 (SCL) |
| **Sensor Calibration** | 3413 (based on 0.00075Ω shunt) |
| **Current LSB** | 0.002A per bit |

### Wiring Diagram
```
ESP32-S3                INA226              Battery
────────                ──────              ───────
GPIO8  ────────────────► SDA               │
GPIO9  ────────────────► SCL               │
GND    ────────────────► GND              ─┼─► Load
3.3V   ────────────────► VIN              ─┼─►
                                         Shunt
                                        ┌─────┐
                                        │     │
                                        └─────┘
                                          │ │
                                         +V -V
```

## Building

### PlatformIO
```bash
# Build for ESP32-S3
pio run -e esp32-s3-devkitc-1

# Build and upload
pio run -e esp32-s3-devkitc-1 -t upload

# Run all tests
pio test -e native
```

### Arduino IDE
1. Open `dipstick.ino` in Arduino IDE
2. Select Board: "ESP32-S3 DevKitC"
3. Configure MQTT broker in `wifi_manager.h` and `mqtt_manager.h`
4. Upload

## Testing

### Test Structure
```
test/test_monitor/
├── test_main.cpp           # Basic functionality tests
├── test_coulomb.t.cpp      # Coulomb counting verification
├── test_soc.t.cpp          # SOC calculation tests
├── test_thermal.t.cpp      # Thermal override tests
├── test_debounce.t.cpp     # State debounce tests
├── test_state_machine.t.cpp# State transition tests
└── test_utils.h            # Mock objects for testing
```

### Running Tests
```bash
# Run all native tests
pio test -e native

# Run specific test file
pio test -e native -f test_coulomb

# Run with verbose output
pio test -e native -vvv
```

### Test Coverage
| Test Suite | Tests | Coverage |
|------------|-------|----------|
| Basic Functionality | 5 | Initial state, critical state, deadband, SOC, debounce |
| Coulomb Counting | 3 | Discharge, charge, deadband filter |
| SOC Logic | 2 | Decrease on discharge, cap at 100% |
| Thermal Override | 2 | Critical temp, bypass debounce |
| Debounce | 3 | Warning, critical, reset |
| State Machine | 2 | Charging detection, on_battery |
| **Total** | **17** | **All critical paths** |

### Mock Strategy
- `MockSensor`: Injects controlled voltage, current, power values
- `MockTemperature`: Simulates temperature readings for thermal tests
- `ArduinoFake`: Mocks Arduino runtime functions for native testing

## MQTT API

### Topic
`dipstick/state`

### Message Format
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
    "alert_level": "warning",
    "reason": "capacity_low"
  }
}
```

### State Definitions

| Power State | Description |
|-------------|-------------|
| `charging` | Voltage ≥ 13.2V AND Current > 0A |
| `on_battery` | Normal discharging (safe) |
| `warning` | Runtime ≤ 300s OR SOC ≤ 30% (3 samples) |
| `critical` | Runtime ≤ 120s OR SOC ≤ 10% OR Voltage ≤ 11.2V OR Temp > 75°C |

| Alert Level | Description |
|-------------|-------------|
| `none` | Normal operation |
| `warning` | Attention needed, system stable |
| `critical` | Immediate action required |

| Reason | Description |
|--------|-------------|
| `capacity_low` | SOC or runtime below warning threshold |
| `voltage_low` | Voltage below critical threshold |
| `overheat` | Temperature exceeds 75°C |

## Configuration

### Key Parameters (platformio.ini)
```ini
[env:esp32-s3-devkitc-1]
build_flags = 
    -D BOARD_HAS_PSRAM      # Enable PSRAM
    -Isrc/include           # Include path for headers
lib_deps =
    robtillaart/INA226      # INA226 driver
    bblanchon/ArduinoJson   # JSON handling (v7)
    knolleary/PubSubClient  # MQTT client
```

### Thresholds
| Parameter | Warning | Critical |
|-----------|---------|----------|
| Runtime | ≤ 300s | ≤ 120s |
| SOC | ≤ 30% | ≤ 10% |
| Voltage | - | ≤ 11.2V |
| Temperature | > 65°C (throttle) | > 75°C (override) |

## Troubleshooting

### I2C Communication Issues
- Check wiring: SDA to GPIO8, SCL to GPIO9
- Verify shunt connection (low-side configuration)
- Check `Wire.setTimeOut(100)` is preventing hangs

### MQTT Connection Problems
- Verify broker is running and accessible
- Check WiFi credentials in `wifi_manager.h`
- Monitor reconnection backoff (exponential, max 60s)

### Incorrect SOC Readings
- Verify battery capacity setting (default: 108 Wh)
- Check deadband filter threshold (0.02A)
- Ensure coulomb counting formula uses correct time delta

## License

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
