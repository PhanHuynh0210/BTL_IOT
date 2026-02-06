#include "TaskWifi.h"

constexpr char WIFI_SSID[]     = "HOANG HUYNH VNPT";
constexpr char WIFI_PASSWORD[] = "0917683220";

void TaskWiFi(void *pvParameters)
{
    system_event evt;

    for (;;)
    {
        if (xQueueReceive(wifiQueue, &evt, portMAX_DELAY) == pdTRUE)
        {
            if (evt == EVT_WIFI_START)
            {
                Serial.println("[WiFi] Connecting...");
                WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

                uint8_t retry = 0;
                while (WiFi.status() != WL_CONNECTED && retry < 20)
                {
                    vTaskDelay(pdMS_TO_TICKS(500));
                    retry++;
                }

                if (WiFi.status() == WL_CONNECTED)
                {
                    Serial.println("[WiFi] Connected");

                    system_event ok = EVT_WIFI_OK;
                    xQueueSend(stateQueue, &ok, 0);
                    xSemaphoreGive(CoreIOTSem);

                }
                else
                {
                    Serial.println("[WiFi] Failed");

                    system_event fail = EVT_WIFI_FAIL;
                    xQueueSend(stateQueue, &fail, 0);
                }
            }
        }
    }
}
