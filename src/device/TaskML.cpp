#include "TaskML.h"
#include "../model.h"


tflite::MicroErrorReporter micro_error_reporter;
tflite::AllOpsResolver resolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;

TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

constexpr int tensorArenaSize = 20 * 1024;
uint8_t tensorArena[tensorArenaSize];

void TaskTinyML(void *pvParameters) {

  Serial.println("TinyML Task Starting...");

  tflModel = tflite::GetModel(model_tflite);

  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    vTaskDelete(NULL);
  }

  interpreter = new tflite::MicroInterpreter(
      tflModel,
      resolver,
      tensorArena,
      tensorArenaSize,
      &micro_error_reporter
  );

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors failed!");
    vTaskDelete(NULL);
  }

  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("TinyML Ready!");

  Sensordata data;

  while (1) {

    if(xQueueReceive(MLTinyQueue, &data, pdMS_TO_TICKS(1000))){
        input->data.f[0] = data.temp;
        input->data.f[1] = data.humi;

        // Run inference
        if (interpreter->Invoke() != kTfLiteOk) {
        Serial.println("Invoke failed!");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        continue;
        }

        float normal = output->data.f[0];
        float hot    = output->data.f[1];
        float humid  = output->data.f[2];

        // Argmax
        int predicted = 0;
        float maxVal = normal;

        if (hot > maxVal) {
        maxVal = hot;
        predicted = 1;
        }

        if (humid > maxVal) {
        predicted = 2;
        }

        Serial.print("Prediction: ");
        if (predicted == 0) Serial.println("Normal");
        else if (predicted == 1) Serial.println("Hot");
        else Serial.println("Humid");
        }
    }
}