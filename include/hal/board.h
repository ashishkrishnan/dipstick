#pragma once

// ESP32-S3 DevKitC-1 Pin Definitions for Project Dipstick

// I2C for INA226
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

// Watchdog Timer (WDT) - 10s timeout
#define WDT_TIMEOUT_SECONDS 10

// LED indicator (optional, for status)
#define LED_PIN 10

// Optional: External reset button (if used)
#define RESET_BUTTON_PIN 11

// Battery voltage divider (if used for direct measurement)
// Not used in this design — voltage measured via INA226