#ifndef INC_TASKWIFI_H_
#define INC_TASKWIFI_H_

#include "globals.h"
extern void loadWiFi(String &ssid, String &pass);
extern void TaskWiFi(void *pvParameters);

#endif /* INC_TASKWIFI_H_ */