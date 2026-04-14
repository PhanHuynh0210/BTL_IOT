#include "TaskEspNow.h"
#include "globals.h"
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// Tạo một Queue riêng cho ESP-NOW
static QueueHandle_t espNowRxQueue;

// Hàm Callback: CHỈ LÀM NHIỆM VỤ ĐẨY VÀO QUEUE (Nhanh, gọn, nhẹ)
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  if (len == sizeof(EspNowDht20Packet)) {
    EspNowDht20Packet pkt;
    memcpy(&pkt, incomingData, sizeof(pkt));
    
    // Nếu đúng Magic Number thì đẩy vào Queue
    if (pkt.magic == 0xB71E10F0) {
      // Vì hàm này chạy ngầm, dùng hàm Send của FreeRTOS an toàn
      xQueueSend(espNowRxQueue, &pkt, 0); 
    }
  }
}

// ===== TASK XỬ LÝ CHÍNH =====
void TaskEspNowRecv(void *pvParameters) {
  (void)pvParameters;

  // 1. Tạo Queue chứa tối đa 10 gói tin ESP-NOW chờ xử lý
  espNowRxQueue = xQueueCreate(10, sizeof(EspNowDht20Packet));

  // 2. Khởi tạo WiFi & ESP-NOW
  // LƯU Ý: Nếu TaskWiFi của bạn có kết nối vào Router mạng nhà, 
  // bạn PHẢI đảm bảo ESPNOW_WIFI_CHANNEL trùng với kênh của cục Router đó.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false); 
  esp_wifi_set_channel(ESPNOW_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESPNOW] Lỗi khởi tạo!");
    vTaskDelete(NULL);
    return;
  }
  
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  Serial.println("[ESPNOW] Đang lắng nghe dữ liệu...");

  // Biến dùng để hứng dữ liệu lấy ra từ Queue
  EspNowDht20Packet rxData;

  for(;;) {
    // 3. Task sẽ "ngủ" ở đây cho đến khi có dữ liệu mới trong Queue
    if (xQueueReceive(espNowRxQueue, &rxData, portMAX_DELAY) == pdTRUE) {
      
      // -- In ra Serial Monitor --
      Serial.println("======= CÓ DỮ LIỆU TỪ SENDER =======");
      Serial.printf("Sensor ID : %u \n", rxData.sensorId);
      Serial.printf("Nhiệt độ  : %4.2f °C\n", rxData.temp);
      Serial.printf("Độ ẩm     : %4.2f %%\n", rxData.humi);
      Serial.println("====================================\n");

      // -- TÍCH HỢP VÀO HỆ THỐNG CỦA BẠN --
      // Chuyển đổi sang chuẩn Sensordata của hệ thống
      Sensordata sData;
      sData.temp = rxData.temp;
      sData.humi = rxData.humi;

        xQueueOverwrite(lcdQueue, &sData); 
        xQueueOverwrite(GGSheetQueue, &sData);
        xQueueOverwrite(coreIOTQueue, &sData);
        xQueueOverwrite(MLTinyQueue, &sData);
    }
  }
}