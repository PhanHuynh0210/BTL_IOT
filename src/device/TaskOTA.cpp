#include "TaskOTA.h"

#define CURRENT_FW_VERSION "v1.0.0"

void checkFirmwareVersion()
{
    const char* githubApi =
        "https://api.github.com/repos/PhanHuynh0210/TTNT/releases/latest";

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[OTA] WiFi not connected");
        return;
    }

    Serial.println("[OTA] Checking firmware version...");

    WiFiClientSecure secureClient;
    secureClient.setInsecure();  // CA

    HTTPClient http;
    if (!http.begin(secureClient, githubApi))
    {
        Serial.println("[OTA] HTTP begin failed");
        return;
    }

    http.addHeader("User-Agent", "ESP32");

    int httpCode = http.GET();
    Serial.print("[OTA] HTTP code: ");
    Serial.println(httpCode);

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.println("[OTA] GitHub API request failed");
        http.end();
        return;
    }

    String payload = http.getString();
    http.end();

    StaticJsonDocument<64> filter;
    filter["tag_name"] = true;

    StaticJsonDocument<256> doc;
    DeserializationError error =
        deserializeJson(doc, payload, DeserializationOption::Filter(filter));

    if (error)
    {
        Serial.print("[OTA] JSON parse failed: ");
        Serial.println(error.c_str());
        return;
    }

    const char* latestVersion = doc["tag_name"];
    if (latestVersion == nullptr)
    {
        Serial.println("[OTA] tag_name not found");
        return;
    }

    bool needUpdate = strcmp(latestVersion, CURRENT_FW_VERSION) != 0;

    StaticJsonDocument<256> mgs;
    mgs["firmware_current"] = CURRENT_FW_VERSION;
    mgs["firmware_latest"]  = latestVersion;
    mgs["need_update"]      = needUpdate;

    char buffer[256];
    serializeJson(mgs, buffer);

    client.publish("v1/devices/me/attributes", buffer);

    Serial.println("[OTA] Firmware info published");
}


void ota_task(void *pvParameters)
{
    Serial.println("[OTA] OTA task started");

    for (;;)
    {
        if (xSemaphoreTake(otaSem, portMAX_DELAY))
        {
            Serial.println("[OTA] Semaphore received");
            checkFirmwareVersion();  
        }
    }
}