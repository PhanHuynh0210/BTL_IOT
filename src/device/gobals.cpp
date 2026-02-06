#include "globals.h"

SemaphoreHandle_t tempLowSem  = nullptr;
SemaphoreHandle_t tempMidSem  = nullptr;
SemaphoreHandle_t tempHighSem = nullptr;

SemaphoreHandle_t humLowSem  = nullptr;
SemaphoreHandle_t humMidSem  = nullptr;
SemaphoreHandle_t humHighSem = nullptr;

// wifi done connect mqtt
SemaphoreHandle_t CoreIOTSem = xSemaphoreCreateBinary();

//check ota version
SemaphoreHandle_t otaSem = nullptr;

// data
QueueHandle_t lcdQueue = nullptr;
QueueHandle_t coreIOTQueue = nullptr;

//update mqtt
SemaphoreHandle_t mqttUpdateSem = nullptr;


//status sys
QueueHandle_t stateQueue;
QueueHandle_t ledQueue;

QueueHandle_t wifiQueue;


WiFiClient espClient;
PubSubClient client(espClient);
