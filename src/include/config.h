/**
 * @file config.h
 * @brief Project Dipstick Configuration
 * 
 * Centralized configuration for all adjustable parameters.
 * Modify these values to customize behavior for your specific setup.
 */

#pragma once

// ============================================================================
// Battery Configuration
// ============================================================================
/** Default battery capacity in Watt-hours */
#define BATTERY_CAPACITY_WH           108.0

/** Current threshold below which current is treated as zero (Amps) */
#define CURRENT_DEADBAND_THRESHOLD    0.02

// ============================================================================
// State Machine Thresholds
// ============================================================================
/** Critical voltage threshold (Volts) */
#define STATE_CRITICAL_VOLTAGE        11.2

/** Critical SOC threshold (Percent) */
#define STATE_CRITICAL_SOC            10.0

/** Critical runtime threshold (Seconds) */
#define STATE_CRITICAL_RUNTIME        120

/** Warning SOC threshold (Percent) */
#define STATE_WARNING_SOC             30.0

/** Warning runtime threshold (Seconds) */
#define STATE_WARNING_RUNTIME         300

/** Voltage threshold for charging state detection (Volts) */
#define STATE_CHARGING_VOLTAGE        13.2

/** Current threshold for charging state detection (Amps) */
#define STATE_CHARGING_CURRENT        0.0

// ============================================================================
// Thermal Configuration
// ============================================================================
/** Temperature threshold for throttling publish interval (Celsius) */
#define THERMAL_THROTTLE_TEMP         65.0

/** Temperature threshold for critical state override (Celsius) */
#define THERMAL_CRITICAL_TEMP         75.0

/** Publish interval when thermal throttling is active (Milliseconds) */
#define THERMAL_THROTTLE_INTERVAL     10000

/** Publish interval at moderate temperatures (Milliseconds) */
#define THERMAL_NORMAL_INTERVAL       5000

/** Default publish interval (Milliseconds) */
#define THERMAL_DEFAULT_INTERVAL      2000

// ============================================================================
// MQTT Configuration
// ============================================================================
/** MQTT broker server address */
#define MQTT_SERVER                   "mqtt.broker.com"

/** MQTT broker port */
#define MQTT_PORT                     1883

/** MQTT topic for publishing state */
#define MQTT_TOPIC                    "dipstick/state"

/** MQTT buffer size in bytes */
#define MQTT_BUFFER_SIZE              512

/** Base delay for MQTT reconnection attempts (Milliseconds) */
#define MQTT_RECONNECT_BASE_DELAY     1000

/** Maximum reconnection delay (Milliseconds) */
#define MQTT_RECONNECT_MAX_DELAY      60000

/** Maximum reconnection attempts per minute */
#define MQTT_MAX_RECONNECT_ATTEMPTS   5

// ============================================================================
// WiFi Configuration
// ============================================================================
/** WiFi SSID - Define CONFIG_WIFI_SSID in secrets.h or build flags */
#ifndef CONFIG_WIFI_SSID
#define CONFIG_WIFI_SSID "your_ssid"
#endif

/** WiFi password - Define CONFIG_WIFI_PASSWORD in secrets.h or build flags */
#ifndef CONFIG_WIFI_PASSWORD
#define CONFIG_WIFI_PASSWORD "your_password"
#endif

/** WiFi SSID alias for backward compatibility */
#define WIFI_SSID                     CONFIG_WIFI_SSID

/** WiFi password alias for backward compatibility */
#define WIFI_PASSWORD                 CONFIG_WIFI_PASSWORD

/** WiFi reconnection check interval (Milliseconds) */
#define WIFI_CONNECT_INTERVAL         1000

// ============================================================================
// Hardware Configuration
// ============================================================================
/** I2C SDA pin (ESP32-S3) */
#define I2C_SDA_PIN                   8

/** I2C SCL pin (ESP32-S3) */
#define I2C_SCL_PIN                   9

/** I2C communication timeout (Milliseconds) */
#define I2C_TIMEOUT                   100

/** INA226 I2C address */
#define INA226_ADDRESS                0x40

/** INA226 calibration register value */
#define INA226_CALIBRATION            3413

/** Shunt resistance value (Ohms) */
#define SHUNT_RESISTANCE              0.00075

/** Current LSB for INA226 (Amps per bit) */
#define CURRENT_LSB                   0.002

/** Maximum valid voltage reading (Volts) */
#define VOLTAGE_MAX_VALID             15.0

/** Minimum valid voltage reading (Volts) */
#define VOLTAGE_MIN_VALID             5.0

// ============================================================================
// System Configuration
// ============================================================================
/** Main loop interval (Milliseconds) */
#define LOOP_INTERVAL                 250

/** Initial setup delay (Milliseconds) */
#define SETUP_DELAY                   10

/** Watchdog timer timeout (Seconds) */
#define WDT_TIMEOUT                   10
