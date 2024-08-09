#include <WiFi.h>
#include "sb-class.h"

// Replace with your network credentials
const char* ssid = "hotel-new-akao";
const char* password = "0557825151";

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    

    // Initialize SwitchBotManager
    SwitchBotManager::getInstance().init();
}

void loop() {
    SwitchBotManager& sbManager = SwitchBotManager::getInstance();
    
    sbManager.updateData();

    // Get the temperature for specific rooms

    float temperature = sbManager.getTemperatureForRoom("Kitchen");
    Serial.printf("Temperature in Kitchen: %4.1f\n", temperature);

    temperature = sbManager.getTemperatureForRoom("Office");
    Serial.printf("Temperature in Office: %4.1f\n", temperature);

    temperature = sbManager.getTemperatureForRoom("Musicroom");
    Serial.printf("Temperature in Musicroom: %4.1f\n", temperature);

    delay(15000); // Reduced delay to 15 seconds for more frequent updates

}