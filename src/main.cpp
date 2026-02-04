
#include "globals.h"

void setup()
{
  Serial.begin(115200);



\
  xTaskCreate(TaskDHT11,   "DHT11",    4096, NULL, 3, NULL);
  xTaskCreate(TaskDHT20,   "DHT20",    4096, NULL, 3, NULL);
  xTaskCreate(TaskBlink,"TaskBlink", 2048, NULL, 2, NULL);
  xTaskCreate(TaskNeoPixel,"NeoPixel", 2048, NULL, 2, NULL);
  xTaskCreate(TaskLCD,     "LCD",      4096, NULL, 1, NULL);
  
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