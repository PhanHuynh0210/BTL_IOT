#include "TaskOTA.h"

String VersionNew;

#define OTA_BASE_URL "https://raw.githubusercontent.com/PhanHuynh0210/TTNT/gh-pages"
#define OTA_BIN_NAME "firmware.bin"

#define CURRENT_FIRMWARE_VERSION "v1.0" 

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

    String firmwareUrl = String(OTA_BASE_URL) + "/" + VersionNew + "/" + OTA_BIN_NAME;

    Serial.println("=== OTA UPDATE START ===");
    Serial.println("Đang nâng cấp lên: " + VersionNew);
    Serial.println("URL tải: " + firmwareUrl);
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

    switch (ret)
    {
        case HTTP_UPDATE_FAILED:
            Serial.printf("[OTA] Failed: %s\n", httpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("[OTA] No update");
            break;

        case HTTP_UPDATE_OK:
            Serial.println("[OTA] Update success – rebooting");
            break;
    }
}

void reportCurrentFirmware()
{
    StaticJsonDocument<128> mgs;
    mgs["firmware_latest"] = CURRENT_FIRMWARE_VERSION;

    char buffer[128];
    size_t len = serializeJson(mgs, buffer);

    if (client.publish("v1/devices/me/telemetry", buffer, len)) {
        Serial.print("[OTA] Đã báo cáo phiên bản hiện hành lên Server: ");
        Serial.println(CURRENT_FIRMWARE_VERSION);
    }
}

void ota_task(void *pvParameters)
{    
    while (true) {
        if (client.connected()) {
            vTaskDelay(pdMS_TO_TICKS(1000)); 
            reportCurrentFirmware();  
            break;    
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }

    OTA_SYS ota;
    for (;;)
    {
        if (xQueueReceive(otaQueue, &ota, portMAX_DELAY))
        {
            if (ota == OTA_UPDATE)
            {
                Serial.println("[OTA] Nhận lệnh từ Queue, bắt đầu nạp Firmware...");
                updateFirmwareVersion();
            }
        }
    }
}