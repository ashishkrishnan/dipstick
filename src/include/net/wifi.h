#pragma once

#include <WiFi.h>
#include <Arduino.h>

class WiFiManager {
public:
    WiFiManager(const char* ssid, const char* password) : ssid(ssid), password(password), lastConnect(0), connectInterval(1000) {}

    void begin() {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        lastConnect = millis();
    }

    bool isConnected() {
        return WiFi.status() == WL_CONNECTED;
    }

    void update() {
        if (!isConnected() && (millis() - lastConnect) >= connectInterval) {
            // Exponential backoff: 1s, 2s, 4s, 8s, 16s, 32s, 60s max
            if (connectInterval < 60000) {
                connectInterval *= 2;
            }
            WiFi.disconnect();
            WiFi.begin(ssid, password);
            lastConnect = millis();
        }
    }

    const char* getIPAddress() {
        return WiFi.localIP().toString().c_str();
    }

    int getRSSI() {
        return WiFi.RSSI();
    }

private:
    const char* ssid;
    const char* password;
    uint32_t lastConnect;
    uint32_t connectInterval;
};