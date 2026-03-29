#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "include/hal/board.h"
#include "include/hal/isensor.h"
#include "include/hal/ina226.h"
#include "include/hal/temperature.h"
#include "include/app/monitor.h"
#include "include/net/wifi_manager.h"
#include "include/net/mqtt.h"
#include "include/config.h"

WiFiManager wifi(WIFI_SSID, WIFI_PASSWORD);
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
INA226 sensor;
ESP32Temperature esp32Temp;
Monitor monitor(&sensor, &esp32Temp);
MQTTManager mqttManager(mqttClient, wifi, monitor);

void setup() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setTimeOut(I2C_TIMEOUT);
    sensor.begin();
    wifi.begin();
    mqttManager.begin(MQTT_SERVER, MQTT_PORT, MQTT_TOPIC);
    delay(SETUP_DELAY);
}

void loop() {
    static uint32_t lastLoop = 0;
    uint32_t currentMillis = millis();

    double temp = esp32Temp.readTemperature();
    monitor.setInternalTemp(temp);
    sensor.update();
    monitor.update(currentMillis);
    wifi.update();
    mqttManager.update();

    while (millis() - lastLoop < LOOP_INTERVAL) {
        delay(1);
    }
    lastLoop = millis();
}