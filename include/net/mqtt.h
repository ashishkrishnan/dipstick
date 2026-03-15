#pragma once

#include <PubSubClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "../app/monitor.h"
#include "../net/wifi.h"

class MQTTManager {
public:
    MQTTManager(PubSubClient& mqttClient, WiFiManager& wifi, Monitor& monitor) 
        : mqttClient(mqttClient), wifi(wifi), monitor(monitor), lastPublish(0), publishInterval(2000), reconnectAttempts(0), lastReconnect(0) {}

    void begin(const char* server, int port, const char* topic) {
        this->server = server;
        this->port = port;
        this->topic = topic;
        mqttClient.setServer(server, port);
        mqttClient.setBufferSize(512); // Required by spec
    }

    void update() {
        // Handle MQTT reconnection with exponential backoff
        if (!mqttClient.connected()) {
            if (millis() - lastReconnect >= 1000) {
                if (reconnectAttempts < 5) { // Max 5 attempts per minute
                    if (reconnectAttempts == 0) {
                        reconnectAttempts = 1;
                    } else {
                        reconnectAttempts *= 2;
                    }
                    if (reconnectAttempts > 60) reconnectAttempts = 60; // Cap at 60s
                    if (mqttClient.connect("dipstick")) {
                        reconnectAttempts = 0;
                    }
                }
                lastReconnect = millis();
            }
        } else {
            reconnectAttempts = 0;
        }

        // Handle thermal throttling
        double temp = monitor.getDTO().internal_temp_c;
        if (temp > 75.0) {
            publishInterval = 10000; // 10s
        } else if (temp > 65.0) {
            publishInterval = 5000; // 5s
        } else {
            publishInterval = 2000; // 2s
        }

        // Publish state if interval has passed
        if (mqttClient.connected() && (millis() - lastPublish) >= publishInterval) {
            publishState();
            lastPublish = millis();
        }
    }

    void publishState() {
        StaticJsonDocument<512> doc;
        auto& system = doc.createNestedObject("system");
        auto& network = doc.createNestedObject("network");
        auto& battery = doc.createNestedObject("battery");
        auto& status = doc.createNestedObject("status");

        // System info
        auto& dto = monitor.getDTO();
        system["uptime_seconds"] = dto.uptime_seconds;
        system["uptime_human"] = formatUptime(dto.uptime_seconds);
        system["internal_temp_c"] = dto.internal_temp_c;
        system["free_heap_bytes"] = ESP.getFreeHeap();

        // Network info
        network["ip_address"] = wifi.getIPAddress();
        network["wifi_rssi"] = wifi.getRSSI();
        network["publish_interval_ms"] = publishInterval;

        // Battery info
        battery["voltage_bus"] = dto.voltage_bus;
        battery["current_amps"] = dto.current_amps;
        battery["power_watts"] = dto.power_watts;
        battery["capacity_wh_remaining"] = dto.capacity_wh_remaining;
        battery["estimated_soc_pct"] = dto.estimated_soc_pct;
        battery["estimated_runtime_seconds"] = dto.estimated_runtime_seconds;

        // Status
        status["power_state"] = powerStateToString(dto.power_state);
        status["alert_level"] = alertLevelToString(dto.alert_level);
        if (dto.reason) {
            status["reason"] = dto.reason;
        }

        // Serialize and publish
        char jsonBuffer[512];
        serializeJson(doc, jsonBuffer);
        mqttClient.publish(topic, jsonBuffer);
    }

private:
    PubSubClient& mqttClient;
    WiFiManager& wifi;
    Monitor& monitor;
    const char* server;
    int port;
    const char* topic;
    uint32_t lastPublish;
    uint32_t publishInterval;
    uint32_t reconnectAttempts;
    uint32_t lastReconnect;

    const char* powerStateToString(PowerState state) {
        switch (state) {
            case POWER_STATE_CHARGING: return "charging";
            case POWER_STATE_ON_BATTERY: return "on_battery";
            case POWER_STATE_CRITICAL: return "critical";
            case POWER_STATE_WARNING: return "warning";
            default: return "unknown";
        }
    }

    const char* alertLevelToString(AlertLevel level) {
        switch (level) {
            case ALERT_LEVEL_NONE: return "none";
            case ALERT_LEVEL_WARNING: return "warning";
            case ALERT_LEVEL_CRITICAL: return "critical";
            default: return "unknown";
        }
    }

    char* formatUptime(uint32_t seconds) {
        static char buffer[32];
        uint32_t days = seconds / 86400;
        seconds %= 86400;
        uint32_t hours = seconds / 3600;
        seconds %= 3600;
        uint32_t minutes = seconds / 60;
        seconds %= 60;

        snprintf(buffer, sizeof(buffer), "%dd %02dh %02dm %02ds", days, hours, minutes, seconds);
        return buffer;
    }
};