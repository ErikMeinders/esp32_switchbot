#ifndef SB_CLASS_H
#define SB_CLASS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "esp32_switchbot.h"

class SwitchBotManager {
public:
    static SwitchBotManager& getInstance();

    void init();
    void updateData();
    float getTemperatureForRoom(const char* roomName);
    void dump();

private:
    SwitchBotManager();
    ~SwitchBotManager() = default;
    SwitchBotManager(const SwitchBotManager&) = delete;
    SwitchBotManager& operator=(const SwitchBotManager&) = delete;

    char* getRoomFromDeviceName(const char* deviceName);
    void initDeviceList(bool blocking);
    void retrieveTemperatures();

    JsonDocument m_deviceList;
    unsigned long m_lastDump;
    int m_currentDeviceIndex;
    
};

#endif // SB_CLASS_H