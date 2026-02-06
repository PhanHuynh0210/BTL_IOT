#ifndef APMODE_H
#define APMODE_H

#include "globals.h"

extern void initAP();
extern void taskBootButton(void *pvParameters);
extern void taskAccessPoint(void *pvParameters);
#endif // APMODE_H