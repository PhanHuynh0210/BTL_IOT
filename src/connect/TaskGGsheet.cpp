#include "TaskGGsheet.h"


#define PROJECT_ID "hazel-cipher-466204-g6"
#define CLIENT_EMAIL "mil-837@hazel-cipher-466204-g6.iam.gserviceaccount.com"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQC/YJh0J/hXiS44\n0f8jaH32FnB9IXu7sux/pOy1z3GJVr1LuhWJhFUDv8/7NUAflNQMjrMA6F8kyilL\n2eLJqHyUYDBmD212AXdrX5n+t/wedzBv3lmY9Hs2XsLq4T750asMvEWBrf4QhK87\nBQumwrBWoVKpccdE4oRXljLFWDczHaoujLgUNep/dl0JJTlJAwJSRUngg8SFC10g\n48tzf5OH3qw0ZGP1dez1iXNaCguOGp4zViGOyxzirikNKeTYzSWZwvJF2oEVND5Z\nLE22ffZxvVOthTOlFJPWK3EJHoNy/QO15TGxWmUJilPVwLM6rJbJ8cqyNTZzufRA\nm0KQuYTzAgMBAAECggEAT8c8xm9DH3qYQieIbIJKwCNqkT88DWrxG+TsqH3pno8C\nKmdQQxtfNUjospfoO0IXg4SZKm+z+eosfOo/mEEcbd4LTwBvaRoDZkdcMssP1sNo\n9bmDMDf6hRfLuY5calf1LkHt17loiApH4hqgWMwgJnzpsKk1oImtEbYTRCEaIbwp\ntm0EFqxwtHVwpVFzsqRUpDtxDC/7J8FvuTEram+bgwEdKt1wKa98tGByqRWuDog8\nVbwlQr1FqL1HsVzh8b/EwtcH/ylJtSXzX2lVtAtqOgbv28FAjl86Iz+TOZLetSiW\nCBONTZy4EK/EgL33gTodDKAsrHy8WXGB1ohxh6kyQQKBgQD+d7WNX+zytNHwhsqP\nMBqDg5alDXw30dGYvo1ZNlPZoAo3BgOvSTG45E/0nS+uRKaRbrr4G4ma3dXfufgP\nN0PEgyFge3WWk/KLjtxVNLmHK1cPEdsN8ViKxeUeycpz/QyT/LvRSQgfeTx/eIYU\nVvM8E07eBGR09tiHzHP+2apclQKBgQDAh6AewB/RoHSMpISPONeionP5Fi/i1eQD\njbutueGufJbC4YQPYwfMSB/d2hXktNEgpv218Suwlv72py+N+pO8+BXi18z1A7I2\nrPUkYpJzRaOSBSCy78g/6eCcOblVU4mht7j0v8jithZO2X7ClGXJS9bv29Rr0lvi\nEiejhG7xZwKBgQDdeLTDsbxZVLTvoXEEhHpfAhUH3B/DuVKxnQ8KmTULULXzQ8HL\nOmoTIGrSXEgWZVuPcg86R0VRS0eMq5/RJmlNzE8tWOZzwpoziogLhJMAqODhlcnQ\nbcCBL1GFpV0V7CSZXDrfIK/4LJ1R2EThyPdzzt4ybyI7hmfYcUJ3iVwzrQKBgDqk\nlPcI2ihF0XpNWjstuzDEfsKuKWPUdqFGeJYP+lflhx5RBqKRLXzhsyMmHGkPUmu5\nsanLjd06AOssPWXnnzx97FyuErXGrG/O+tRcWfNWkN4XRcB7HOmq2Ul8o6ED0vY5\nOdMC37N+dwkJYnzuKprHyO5CbU9yQZ54nS5HVmudAoGBAIeivMOprFTKORm8TNSS\ny3Hzbm/MSzJ2XJF1beqdjezWJD4YxQbmc+gznk8Dykv8SLMZoic3cH1xBwuGgd0x\nFtyCLUb226VTOQ4QrdqPemSdAPZsS8sJOhnnQ287CIyohS5vwEvu+mChxR0UY4Fs\n7pxYNrGV9dx45H20Qh1gN6n+\n-----END PRIVATE KEY-----\n";
const char spreadsheetId[] = "1JNnVTIMrVkB4BE4ygjzXM6DohIG-1_ieRZhER9bWqFs";


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
            headerRow.set("values/[0]/[1]", "SensorID");
            headerRow.set("values/[0]/[2]", "Temperature");
            headerRow.set("values/[0]/[3]", "Humidity");
            if (GSheet.values.append(&headerResp, spreadsheetId, "Sheet1!A1:D1", &headerRow))
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

        Serial.printf("Upload NOW: %s | ID=%u | T=%.2f | H=%.2f\n",
                      timestamp.c_str(),
                      (unsigned)pendingData.sensorId,
                      (double)pendingData.temp,
                      (double)pendingData.humi);

        // ================= PUSH =================
        valueRange.clear();
        response.clear();
        valueRange.add("majorDimension", "ROWS");
        valueRange.set("values/[0]/[0]", timestamp);
        valueRange.set("values/[0]/[1]", (int)pendingData.sensorId);
        valueRange.set("values/[0]/[2]", pendingData.temp);
        valueRange.set("values/[0]/[3]", pendingData.humi);

        if (GSheet.values.append(&response, spreadsheetId, "Sheet1!A1", &valueRange))
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