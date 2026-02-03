
#include "globals.h"

void setup()
{
  Serial.begin(115200);
  // xTaskCreate(TaskTemp, "TaskTemp", 4096, NULL, 1, NULL);
  xTaskCreate(TaskDHT11, "TaskDHT11", 4096, NULL, 1, NULL);
  xTaskCreate(TaskNeoPixel, "TaskNeoPixel", 2048, NULL, 1, NULL);
  initMQTT();  
}

void loop()
{
  if (!Wifi_reconnect())
  {
    return;
  }
  reconnectMQTT();
}