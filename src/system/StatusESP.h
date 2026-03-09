#ifndef STATUSESP_H
#define STATUSESP_H

#include "globals.h"

extern void LedStatus(void *pvParameters);
extern void bootTimeoutCallback(TimerHandle_t xTimer);
extern void taskStateManager(void *pvParameters);
#endif // STATUSESP_H