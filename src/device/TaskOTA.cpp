#include "TaskOTA.h"


String VersionNew;

#define OTA_BASE_URL "https://raw.githubusercontent.com/PhanHuynh0210/TTNT/gh-pages"
#define OTA_BIN_NAME "firmware.bin"

void updateFirmwareVersion()
{
    if (VersionNew.length() == 0)
    {
        Serial.println("[OTA] No new version available");
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[OTA] WiFi not connected");
        return;
    }

    String firmwareUrl =
        String(OTA_BASE_URL) + "/" + VersionNew + "/" OTA_BIN_NAME;

    Serial.println("=== OTA UPDATE START ===");
    Serial.println("Version: " + VersionNew);
    Serial.println("URL: " + firmwareUrl);
    Serial.println("========================");

    WiFiClientSecure otaClient;
    otaClient.setInsecure();

    httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    system_event evt = EVT_OTA_START;
    xQueueSend(stateQueue, &evt, 0);
    client.publish("esp32/status/wifi", "ota_update");
    delay(100);

    httpUpdate.onProgress([](int cur, int total) {
        if (total <= 0) return;
        int percent = (cur * 100) / total;
        Serial.printf("[OTA] %d%%\n", percent);
    });

    t_httpUpdate_return ret = httpUpdate.update(otaClient, firmwareUrl);

    // switch (ret)
    // {
    //     case HTTP_UPDATE_FAILED:
    //         Serial.printf("[OTA] Failed: %s\n",
    //             httpUpdate.getLastErrorString().c_str());
    //         setStatus(STATUS_ERROR);
    //         break;

    //     case HTTP_UPDATE_NO_UPDATES:
    //         Serial.println("[OTA] No update");
    //         setStatus(STATUS_NORMAL);
    //         break;

    //     case HTTP_UPDATE_OK:
    //         Serial.println("[OTA] Update success – rebooting");
    //         break;
    // }
}



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
    VersionNew = latestVersion;
    if (latestVersion == nullptr)
    {
        Serial.println("[OTA] tag_name not found");
        return;
    }


    StaticJsonDocument<256> mgs;
    mgs["firmware_latest"]  = latestVersion;

    char buffer[256];
    serializeJson(mgs, buffer);

    client.publish("v1/devices/me/attributes", buffer);

    Serial.println("[OTA] Firmware info published");
}


void ota_task(void *pvParameters)
{
    OTA_SYS ota;
    for (;;)
    {
        if (xQueueReceive(otaQueue,&ota, portMAX_DELAY))
        {
            switch (ota)
            {
            case OTA_CHECK:
                Serial.println("[OTA] Semaphore received");
                checkFirmwareVersion();
                break;

            case OTA_UPDATE:
                Serial.println("[OTA] Semaphore received");
                updateFirmwareVersion();
                break;
            
            default:
                break;
            }
             
        }
    }
}