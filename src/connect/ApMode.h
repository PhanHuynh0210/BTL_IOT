#ifndef APMODE_H
#define APMODE_H

#include "globals.h"

extern void initAP();
extern void stopAP();
extern void taskBootButton(void *pvParameters);
extern void taskAccessPoint(void *pvParameters);
#endif // APMODE_HledQueue