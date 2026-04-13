
#include "globals.h"

void setup()
{
  Serial.begin(115200);

  // Tạo semaphore và queue trước khi tạo task
  tempLowSem  = xSemaphoreCreateBinary();
  tempMidSem  = xSemaphoreCreateBinary();
  tempHighSem = xSemaphoreCreateBinary();

  humLowSem  = xSemaphoreCreateBinary();
  humMidSem  = xSemaphoreCreateBinary();
  humHighSem = xSemaphoreCreateBinary();

  otaQueue = xQueueCreate(2, sizeof(OTA_SYS));

  mqttUpdateSem = xSemaphoreCreateBinary();

  //data
  lcdQueue = xQueueCreate(1, sizeof(Sensordata));
  coreIOTQueue = xQueueCreate(1, sizeof(Sensordata));
  MLTinyQueue = xQueueCreate(1, sizeof(Sensordata));
  GGSheetQueue = xQueueCreate(1, sizeof(Sensordata));


  stateQueue = xQueueCreate(10, sizeof(system_event));
  ledQueue = xQueueCreate(5, sizeof(system_status));
  wifiQueue = xQueueCreate(10, sizeof(system_event));




  bootTimeoutTimer = xTimerCreate("BootTimeout",pdMS_TO_TICKS(10000), pdFALSE, NULL,bootTimeoutCallback);


  
  xTaskCreate(taskStateManager,   "taskStateManager",    4096, NULL, 6, 0);
  xTaskCreate(LedStatus,   "LedStatus",    4096, NULL, 6, 0);

  xTaskCreate(taskBootButton,   "taskBootButton",    4096, NULL, 5, 0);
  xTaskCreate(TaskWiFi,   "TaskWiFi",    4096, NULL, 5, 0);
  xTaskCreate(taskAccessPoint,   "taskAccessPoint",    4096, NULL, 5, 0);

  xTaskCreate(coreiot_task,   "coreiot_task",    4096, NULL, 4, 0);

  xTaskCreate(ota_task,   "ota_task",    8192, NULL, 3, 0);

  // xTaskCreate(TaskDHT11,   "TaskDHT11",    4096, NULL, 3, NULL);
  xTaskCreate(TaskGGsheet,   "TaskGGsheet",    20480, NULL, 3, NULL);

  xTaskCreate(TaskDHT20,   "DHT20",    4096, NULL, 3, NULL);
  xTaskCreate(TaskBlink,"TaskBlink", 4096, NULL, 2, NULL);
  xTaskCreate(TaskNeoPixel,"NeoPixel", 4096, NULL, 2, NULL);
  xTaskCreate(TaskLCD,     "LCD",      4096, NULL, 1, NULL);
  // xTaskCreatePinnedToCore(TaskTinyML,"TinyML Task",10000,NULL,1, NULL,1);

}

void loop()
{
}