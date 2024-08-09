#ifndef ESP32_SWITCHBOT_H

#include <Arduino.h>

/// @brief Make a GET request to the SwitchBot API. Takes care of v1.1 signature and nonce.
/// @param myPath 
/// @param httpCode 
/// @return String containing the body of the response or error message AND the HTTP code will be set to the HTTP return code
String esp32_switchbot_GET(const char* myPath, int* httpCode);

/// @brief Initialize the SwitchBot API with the token and secret
/// @param token
/// @param secret
void esp32_switchbot_init(const char* token, const char* secret);
#endif