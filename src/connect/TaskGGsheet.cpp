#include "TaskGGsheet.h"





unsigned long epochTime;
static bool headerDone = false;

unsigned long lastUpload = 0;
const uint32_t UPLOAD_INTERVAL = 10000; 

Sensordata pendingData;
bool hasPending = false;

// ================= TIME =================
unsigned long getTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        return 0;

    time_t now;
    time(&now);
    return now;
}

String formatTimestamp(unsigned long epochTime)
{
    struct tm *ptm = localtime((time_t *)&epochTime);
    char buffer[25];

    sprintf(buffer, "%02d/%02d/%04d %02d:%02d:%02d",
            ptm->tm_mday,
            ptm->tm_mon + 1,
            ptm->tm_year + 1900,
            ptm->tm_hour,
            ptm->tm_min,
            ptm->tm_sec);

    return String(buffer);
}

// ================= INIT =================
void tokenStatusCallback(TokenInfo info)
{
    if (info.status == token_status_error)
    {
        GSheet.printf(" Token error: %s\n", GSheet.getTokenError(info).c_str());
    }
    else
    {
        GSheet.printf("Token OK\n");
    }
}

void initGGsheet()
{
    configTime(7 * 3600, 0, "pool.ntp.org");

    GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);
    GSheet.setTokenCallback(tokenStatusCallback);
    GSheet.setPrerefreshSeconds(10 * 60);
    GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}

void TaskGGsheet(void *pvParameters)
{
    Sensordata data;
    bool headerDone = false;

    Sensordata pendingData;
    bool hasPending = false;

    // Wait until WiFi is connected before starting TLS/NTP/token flow
    while (WiFi.status() != WL_CONNECTED)
        vTaskDelay(pdMS_TO_TICKS(500));

    initGGsheet();

    // Reuse JSON objects to reduce peak stack/alloc churn
    FirebaseJson headerRow;
    FirebaseJson headerResp;
    FirebaseJson valueRange;
    FirebaseJson response;

    for (;;)
    {
        if (!hasPending)
        {
            if (xQueueReceive(GGSheetQueue, &data, portMAX_DELAY) == pdTRUE)
            {
                pendingData = data;
                hasPending = true;
            }
        }
        if (!GSheet.ready())
        {
            Serial.println("GSheet chưa ready...");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        if (!hasPending)
        {
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        if (!headerDone)
        {
            headerRow.add("majorDimension", "ROWS");
            headerRow.set("values/[0]/[0]", "Timestamp");
            headerRow.set("values/[0]/[1]", "Temperature");
            headerRow.set("values/[0]/[2]", "Humidity");
            if (GSheet.values.append(&headerResp, spreadsheetId, "Sheet1!A:C", &headerRow))
            {
                Serial.println("Header OK");
                headerDone = true;
            }
            else
            {
                Serial.println("Header lỗi: " + GSheet.errorReason());
                vTaskDelay(pdMS_TO_TICKS(3000));
                continue;
            }
        }

        epochTime = getTime();
        if (epochTime == 0)
        {
            Serial.println("Lỗi time");
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        String timestamp = formatTimestamp(epochTime);

        Serial.printf("Upload NOW: %s | T=%.2f | H=%.2f\n",
                      timestamp.c_str(),
                      (double)pendingData.temp,
                      (double)pendingData.humi);

        // ================= PUSH =================
        valueRange.clear();
        response.clear();
        valueRange.add("majorDimension", "ROWS");
        valueRange.set("values/[0]/[0]", timestamp);
        valueRange.set("values/[0]/[1]", pendingData.temp);
        valueRange.set("values/[0]/[2]", pendingData.humi);

        if (GSheet.values.append(&response, spreadsheetId, "Sheet1!A:C", &valueRange))
        {
            Serial.println("Upload OK");
            hasPending = false; 
        }
        else
        {
            Serial.println("Upload lỗi: " + GSheet.errorReason());
            vTaskDelay(pdMS_TO_TICKS(3000));
        }

        Serial.println("Heap: " + String(ESP.getFreeHeap()));
    }
}