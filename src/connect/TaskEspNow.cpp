#include "TaskEspNow.h"
#include "globals.h"
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

static QueueHandle_t espNowRxQueue;

void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  if (len == sizeof(EspNowDht20Packet)) {
    EspNowDht20Packet pkt;
    memcpy(&pkt, incomingData, sizeof(pkt));
    
    if (pkt.magic == 0xB71E10F0) {
      xQueueSend(espNowRxQueue, &pkt, 0); 
    }
  }
}

void TaskEspNowRecv(void *pvParameters) {
  (void)pvParameters;

  espNowRxQueue = xQueueCreate(10, sizeof(EspNowDht20Packet));

  Serial.println("[ESPNOW] Chờ WiFi kết nối...");
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  uint8_t primaryChan = 0;
  wifi_second_chan_t secondChan;
  esp_wifi_get_channel(&primaryChan, &secondChan);
  Serial.printf("[ESPNOW] WiFi đã kết nối, kênh: %d\n", primaryChan);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESPNOW] Lỗi khởi tạo!");
    vTaskDelete(NULL);
    return;
  }
  
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  Serial.printf("[ESPNOW] Đang lắng nghe trên kênh %d...\n", primaryChan);

  EspNowDht20Packet rxData;

  for(;;) {
    if (xQueueReceive(espNowRxQueue, &rxData, portMAX_DELAY) == pdTRUE) {
      
      Serial.println("======= CÓ DỮ LIỆU TỪ SENDER =======");
      Serial.printf("Sensor ID : %u \n", rxData.sensorId);
      Serial.printf("Nhiệt độ  : %4.2f °C\n", rxData.temp);
      Serial.printf("Độ ẩm     : %4.2f %%\n", rxData.humi);
      Serial.println("====================================\n");

      Sensordata sData;
      sData.sensorId = rxData.sensorId;
      sData.temp     = rxData.temp;
      sData.humi     = rxData.humi;
      sData.uptimeMs = rxData.uptimeMs;

      // LCD chỉ cần bản mới nhất -> Overwrite (queue size = 1)
      xQueueOverwrite(lcdQueue, &sData);

      // Các consumer còn lại cần đầy đủ dữ liệu theo từng sensor -> Send (không Overwrite).
      // Nếu queue đầy sẽ bỏ gói này, nhưng không làm mất gói của sensor khác đã có sẵn trong queue.
      if (xQueueSend(coreIOTQueue, &sData, 0) != pdTRUE) {
        Serial.printf("[ESPNOW] coreIOTQueue full, drop sensor %u\n", rxData.sensorId);
      }
      if (xQueueSend(GGSheetQueue, &sData, 0) != pdTRUE) {
        Serial.printf("[ESPNOW] GGSheetQueue full, drop sensor %u\n", rxData.sensorId);
      }
      if (xQueueSend(MLTinyQueue, &sData, 0) != pdTRUE) {
        Serial.printf("[ESPNOW] MLTinyQueue full, drop sensor %u\n", rxData.sensorId);
      }
    }
  }
}