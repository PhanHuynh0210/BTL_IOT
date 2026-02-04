#include "globals.h"

SemaphoreHandle_t tempLowSem  = nullptr;
SemaphoreHandle_t tempMidSem  = nullptr;
SemaphoreHandle_t tempHighSem = nullptr;

SemaphoreHandle_t humLowSem  = nullptr;
SemaphoreHandle_t humMidSem  = nullptr;
SemaphoreHandle_t humHighSem = nullptr;

SemaphoreHandle_t CoreIOTSem = xSemaphoreCreateBinary();

QueueHandle_t lcdQueue = nullptr;
QueueHandle_t coreIOTQueue = nullptr;
