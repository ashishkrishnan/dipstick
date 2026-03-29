#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "hal/board.h"
#include "hal/isensor.h"
#include "hal/ina226.h"
#include "hal/temperature.h"
#include "app/monitor.h"
#include "net/wifi.h"
#include "net/mqtt.h"

// Global instances
WiFiManager wifi("your_ssid", "your_password");
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
INA226 sensor;
ESP32Temperature esp32Temp;
Monitor monitor(&sensor, &esp32Temp);
MQTTManager mqttManager(mqttClient, wifi, monitor);

void setup() {
    // Initialize I2C
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setTimeOut(100);

    // Initialize sensor
    sensor.begin();

    // Initialize WiFi
    wifi.begin();

    // Initialize MQTT
    mqttManager.begin("mqtt.broker.com", 1883, "dipstick/state");

    // Small delay to allow hardware to stabilize
    delay(10);
}

void loop() {
    static uint32_t lastLoop = 0;
    uint32_t currentMillis = millis();

    // Update temperature
    double temp = esp32Temp.readTemperature();
    monitor.setInternalTemp(temp);

    // Update sensor readings
    sensor.update();

    // Update monitor logic
    monitor.update(currentMillis);

    // Update WiFi connection
    wifi.update();

    // Update MQTT connection and publishing
    mqttManager.update();

    // Ensure 250ms loop
    while (millis() - lastLoop < 250) {
        delay(1);
    }
    lastLoop = millis();
}