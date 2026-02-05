#include "TaskCOREIOT.h"

const char* coreIOT_Server = "app.coreiot.io";  
const char* coreIOT_Token = "gtf8cv81mqtv4zv799mj";   
const int   mqttPort = 1883;

bool mqttReconnect()
{
    if (client.connected())
        return true;

    Serial.print("[MQTT] Connecting...");
    if (client.connect("ESP32Client", coreIOT_Token, NULL))
    {
        Serial.println("OK");
        client.subscribe("v1/devices/me/rpc/request/+");
        return true;
    }

    Serial.print("Fail, rc=");
    Serial.println(client.state());
    return false;
}

void callback(char* topic, byte* payload, unsigned int length) {

  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

    const char* method = doc["method"];
    if (strcmp(method, "checkFirmware") == 0) {
        xSemaphoreGive(otaSem);   
    }
}



void setup_coreiot()
{
    Serial.println("[CoreIOT] Waiting for WiFi...");
    xSemaphoreTake(CoreIOTSem, portMAX_DELAY);

    Serial.println("[CoreIOT] WiFi ready!");

    client.setServer(coreIOT_Server, mqttPort);
    client.setCallback(callback);
}
void coreiot_task(void *pvParameters){

    setup_coreiot();
    Sensordata data;

    while(true){
        if(xQueueReceive(coreIOTQueue, &data, portMAX_DELAY)){
             if (WiFi.status() != WL_CONNECTED){
                xSemaphoreTake(CoreIOTSem, portMAX_DELAY);
            }

            if (mqttReconnect()){
                client.loop();

                StaticJsonDocument<128> doc;
                doc["temperature"] = data.temp;
                doc["humidity"]    = data.humi;

                char buffer[128];
                serializeJson(doc, buffer);

                client.publish("v1/devices/me/telemetry", buffer);

                Serial.print("[CoreIOT] Published: ");
                Serial.println(buffer);
            }
            
        }
    }

}