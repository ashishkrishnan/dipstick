#pragma once

#include <WiFi.h>
#include <Arduino.h>
#include "../config.h"

class WiFiManager {
public:
    WiFiManager(const char* ssid, const char* password) : ssid(ssid), password(password), lastConnect(0), connectInterval(WIFI_CONNECT_INTERVAL) {}

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