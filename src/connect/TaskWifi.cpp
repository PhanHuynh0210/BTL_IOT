#include "TaskWifi.h"

constexpr char WIFI_SSID[]     = "HOANG HUYNH VNPT";
constexpr char WIFI_PASSWORD[] = "0917683220";

void InitWiFi()
{
    Serial.println("[WiFi] Connecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint8_t retry = 0;
    
    while (WiFi.status() != WL_CONNECTED && retry < 20){
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED){
        Serial.println("\n[WiFi] Connected");
        xSemaphoreGive(CoreIOTSem);  
    }
    

}


void TaskWiFi(void *pvParameters)
{
    for (;;){
        if (WiFi.status() != WL_CONNECTED){
            InitWiFi();
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
