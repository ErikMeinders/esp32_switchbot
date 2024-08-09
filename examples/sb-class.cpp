#include "sb-class.h"

SwitchBotManager& SwitchBotManager::getInstance() {
    static SwitchBotManager instance;
    return instance;
}

SwitchBotManager::SwitchBotManager()
    : m_deviceList(),
      m_lastDump(0),
      m_currentDeviceIndex(0) {
}

void SwitchBotManager::init() {
    initDeviceList(true);
}

char* SwitchBotManager::getRoomFromDeviceName(const char* deviceName) {
    char* roomName = (char*)malloc(strlen(deviceName) + 1);
    strcpy(roomName, deviceName);
    char* p = strstr(roomName, "thermometer");
    if (p != NULL) {
        *p = '\0';
    }
    p = roomName + strlen(roomName) - 1;
    while (p >= roomName && *p == ' ') {
        *p = '\0';
        p--;
    }
    return roomName;
}

void SwitchBotManager::initDeviceList(bool blocking = false) {
    // static DynamicJsonDocument doc(40960);
    static JsonDocument doc;
    String response;
    bool done = false;
    int httpCode = 0;


    while (!done) {
        response = esp32_switchbot_GET("/v1.1/devices", &httpCode);

        if (httpCode < 200 || httpCode >= 300) {
            if (!blocking) {
                return;
            }
            delay(2000);
        } else {
            deserializeJson(doc, response);
            const int statusCode = doc["statusCode"];
            if (statusCode != 100) {
                if (!blocking) {
                    return;
                }
            } else {
                done = true;
            }
        }
        if (!done) {
            Serial.println("Retrying (blocking)...");
        }
    }

    JsonArray devices = doc["body"]["deviceList"];
    m_deviceList["devices"] = JsonArray();
    printf("Found %d devices\n", devices.size());
    printf("Device List: ");
    serializeJsonPretty(devices, Serial);
    Serial.println();
    for (JsonObject device : devices) {
        const char* name = device["deviceName"];
        const char* deviceId = device["deviceId"];

        if (strstr(name, "thermometer") != NULL) {
            JsonObject newDevice = m_deviceList["devices"].add<JsonObject>();
            newDevice["roomName"] = getRoomFromDeviceName(name);
            newDevice["deviceId"] = deviceId;
            newDevice["temperature"] = 0.0;
            newDevice["age"] = -1;
        }
    }
    for( int i=0 ; i<m_deviceList["devices"].size() ; i++ ) {
        retrieveTemperatures();
    }
}

void SwitchBotManager::retrieveTemperatures() {
    JsonArray tempDevices = m_deviceList["devices"];
    int deviceCount = tempDevices.size();

    if (deviceCount == 0) {
        Serial.println("No devices found.");
        return;
    }

    if (m_currentDeviceIndex >= deviceCount) {
        m_currentDeviceIndex = 0;
    }

    JsonObject device = tempDevices[m_currentDeviceIndex];
    const char* deviceId = device["deviceId"];
    const char* roomName = device["roomName"];

    char Call[100];
    sprintf(Call, "/v1.1/devices/%s/status", deviceId);

    int httpCode = 0;
    String response = esp32_switchbot_GET(Call, &httpCode);
    
    if (httpCode >= 200 && httpCode < 300) {
        JsonDocument doc;
        deserializeJson(doc, response);
        Serial.printf("Device: %s\n", roomName);
        serializeJsonPretty(doc, Serial);
        Serial.println();
        double temperature = doc["body"]["temperature"];
        
        if (temperature == 0) {
            temperature = device["temperature"];
            device["age"] = String(int(device["age"]) + 1);
        } else {
            device["temperature"] = temperature;
            device["age"] = 0;
        }
    } else {
        device["age"] = String(int(device["age"]) + 1);
    }

    m_currentDeviceIndex++;
}

void SwitchBotManager::updateData() {
    if (m_deviceList.size() == 0) {
        initDeviceList(false);
    }
    
    retrieveTemperatures();
    
    if (m_lastDump + 60000 < millis()) {
        m_lastDump = millis();
        dump();
    }
}

float SwitchBotManager::getTemperatureForRoom(const char* roomName) {
    JsonArray tempDevices = m_deviceList["devices"];
    for (JsonObject device : tempDevices) {
        const char* room = device["roomName"];
        if (strcmp(room, roomName) == 0) {
            return device["temperature"];
        }
    }
    return 0.0;
}

void SwitchBotManager::dump() {
    
    Serial.println("Dumping Device List");
    serializeJsonPretty(m_deviceList, Serial);
    Serial.println();
}