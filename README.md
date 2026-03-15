# Project Dipstick

An industrial-grade ESP32-based UPS battery monitor with raw telemetry, coulomb counting, and automated shutdown via MQTT.

## Setup

1. Clone this repository.
2. Install PlatformIO dependencies:
   ```bash
   /Users/ashishkrishnan/Library/Python/3.9/bin/pio pkg install
   ```
3. Open in PlatformIO IDE or use `pio run` to build.
4. Run tests on native platform:
   ```bash
   pio test -e native
   ```

## Hardware

- **Shunt**: 100A / 75mV External Shunt on Low-Side
- **Max Capacity**: 108.0 Watt-Hours (12V 9Ah SLA Battery)
- **I2C**: ESP32 GPIO8 (SDA), GPIO9 (SCL)
- **Sensor Math**: $R_{shunt} = 0.00075 \, \Omega$, Current LSB = $0.002\text{A}$, INA226 Cal Register = $3413$

## Architecture

- `hal/`: Hardware Abstraction Layer (INA226, Board Pins)
- `app/`: Core Logic (Monitoring, State Machine, DTO)
- `net/`: Networking (WiFi, MQTT)
- `src/main.cpp`: Orchestration and WDT

## Testing

- Tests run on native platform using `ArduinoFake`.
- Mocks `ISensor` interface to validate state machine logic without hardware.
