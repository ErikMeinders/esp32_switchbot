#include "esp32_switchbot.h"

// cryptographic libraries
#include <mbedtls/md.h>
#include <mbedtls/base64.h>

// nonce generation
#include <UUID.h>

// http GET
#include <HTTPClient.h>

// NTP client
#include <WiFiUdp.h>
#include <NTPClient.h>

static String token;
static String secret;

static bool esp32_switchbot_initialized = false;

/// @brief Initialize the SwitchBot API with the token and secret
/// @param token
/// @param secret
void esp32_switchbot_init( const char* m_token, const char* m_secret)
{
    token = String(m_token);
    secret = String(m_secret);
    esp32_switchbot_initialized = true;
}

/// @brief Get the current epoch time, no timezone adjustment. Initialize the NTP client if needed
/// @return seconds since 1970/01/01
static unsigned long getEpochTime() {

    static WiFiUDP m_ntpUDP;
    static NTPClient m_timeClient(m_ntpUDP, "pool.ntp.org", 0, 60000);

    if (m_timeClient.getEpochTime() < 3600) {
        m_timeClient.begin();
        while (!m_timeClient.update()) {
            m_timeClient.forceUpdate();
        }
        Serial.println("Time synchronized");
    }
    return m_timeClient.getEpochTime();
}

/// @brief calculate the signature for the SwitchBot API
/// @param token 
/// @param secret 
/// @param nonce 
/// @param t 
/// @return signature
static String createSignature(const String& token, const String& secret, String& nonce, unsigned long t) 
{
    String stringToSign = token + String(t)+"000" + nonce;
    const char* stringToSignBytes = stringToSign.c_str();
    const char* secretBytes = secret.c_str();

    uint8_t hmacResult[32];
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char*)secretBytes, strlen(secretBytes));
    mbedtls_md_hmac_update(&ctx, (const unsigned char*)stringToSignBytes, strlen(stringToSignBytes));
    mbedtls_md_hmac_finish(&ctx, hmacResult);
    mbedtls_md_free(&ctx);

    unsigned char base64Result[64];
    size_t base64Len;
    mbedtls_base64_encode(base64Result, sizeof(base64Result), &base64Len, hmacResult, sizeof(hmacResult));

    return String((char*)base64Result).substring(0, base64Len);
}

/// @brief add headers for the SwitchBot API
/// @param https 
static void addHeaders(HTTPClient& https) 
{
    unsigned long t;
    UUID uuid;
    static String signature;
    static unsigned long lastSignatureTime = 0;
    static String nonce;

    if ((getEpochTime() - lastSignatureTime) > 30) {
        uint32_t seed1 = random(999999999);
        uint32_t seed2 = random(999999999);
        uuid.seed(seed1, seed2);
        uuid.generate();
        nonce = String(uuid.toCharArray());
        nonce.toUpperCase();
        t = getEpochTime();
        signature = createSignature(token, secret, nonce, t);
        signature.toUpperCase();
        lastSignatureTime = t;
    }

    https.addHeader("Content-Type", "application/json");
    https.addHeader("Authorization", String(token));
    https.addHeader("t", String(lastSignatureTime)+"000");
    https.addHeader("sign", signature);
    https.addHeader("nonce", nonce);

    Serial.printf("Headers: %s, %s, %s, %s\n", token.c_str(), String(lastSignatureTime)+"000", signature.c_str(), nonce.c_str());
}

/// @brief make a GET request to the SwitchBot API
/// @param myPath - the path to the API, starting with /v1.1/
/// @param httpCode 
/// @return Body of the response or error message AND the HTTP code
String esp32_switchbot_GET(const char* myPath, int* httpCode) 
{
    String toReturn;
    HTTPClient https;
    char FullPath[128];

    if(!esp32_switchbot_initialized) {
        Serial.println("SwitchBot API not initialized");
        return String("SwitchBot API not initialized");
    }
    
    if(*myPath == '/') 
        ++myPath;
    
    sprintf(FullPath, "https://api.switch-bot.com/%s", myPath);

    https.begin(FullPath);

    addHeaders(https);

    *httpCode = https.GET();

    if (*httpCode > 0) {
        if (*httpCode == HTTP_CODE_OK) {
            toReturn = https.getString();
        } else {
            toReturn = https.errorToString(*httpCode);
        }
    } else {
        Serial.printf("HTTP GET failed, error: %s\n", https.errorToString(*httpCode).c_str());
    }

    https.end();
    return toReturn;
}