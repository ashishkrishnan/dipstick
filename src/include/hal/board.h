#pragma once

#include "../config.h"

// ESP32-S3 DevKitC-1 Pin Definitions for Project Dipstick
// Pin definitions are now in config.h

// Watchdog Timer timeout
#define WDT_TIMEOUT_SECONDS WDT_TIMEOUT

// LED indicator (optional, for status)
#define LED_PIN 10

// Optional: External reset button (if used)
#define RESET_BUTTON_PIN 11