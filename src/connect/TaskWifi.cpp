#include "TaskWifi.h"


void TaskWiFi(void *pvParameters)
{
    system_event evt;
    String ssid, pass;

    for (;;)
    {
        if (xQueueReceive(wifiQueue, &evt, portMAX_DELAY) == pdTRUE)
        {
            if (evt == EVT_WIFI_START)
            {
                loadWiFi(ssid, pass);

                if (ssid.length() == 0) {
                    system_event fail = EVT_WIFI_FAIL;
                    xQueueSend(stateQueue, &fail, 0);
                    continue;
                }

                WiFi.mode(WIFI_STA);
                WiFi.begin(ssid.c_str(), pass.c_str());

                uint8_t retry = 0;
                while (WiFi.status() != WL_CONNECTED && retry < 20)
                {
                    vTaskDelay(pdMS_TO_TICKS(500));
                    retry++;
                }

                if (WiFi.status() == WL_CONNECTED)
                {
                    Serial.println("[WiFi] Connected");
                    Serial.print("[WiFi] IP: ");
                    Serial.println(WiFi.localIP());

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
