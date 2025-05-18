#include <Arduino_FreeRTOS.h>
#include <queue.h>
#include <DHT.h>

// DHT Sensor setup
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Data struct
typedef struct {
  float temperature;
  float humidity;
} SensorData;

// Handles
QueueHandle_t sensorDataQueue;
TaskHandle_t sensorTaskHandle = NULL;

// Tasks
void TaskReadSensor(void *pvParameters);
void TaskLogData(void *pvParameters);
void TaskCommandListener(void *pvParameters);

void setup() {
  Serial.begin(9600);
  dht.begin();

  sensorDataQueue = xQueueCreate(5, sizeof(SensorData));
  if (sensorDataQueue == NULL) {
    Serial.println("Failed to create queue.");
    while (1);
  }

  // Create tasks
  xTaskCreate(TaskReadSensor, "ReadSensor", 128, NULL, 1, &sensorTaskHandle);
  xTaskCreate(TaskLogData, "LogData", 128, NULL, 1, NULL);
  xTaskCreate(TaskCommandListener, "CommandListener", 128, NULL, 2, NULL);
}

void loop() {
  // Empty – RTOS manages tasks
}

// Reads DHT and sends to queue
void TaskReadSensor(void *pvParameters) {
  SensorData data;

  while (1) {
    data.humidity = dht.readHumidity();
    data.temperature = dht.readTemperature();

    if (!isnan(data.humidity) && !isnan(data.temperature)) {
      xQueueSend(sensorDataQueue, &data, portMAX_DELAY);
    } else {
      Serial.println("DHT read failed.");
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

// Logs received data
void TaskLogData(void *pvParameters) {
  SensorData data;

  while (1) {
    if (xQueueReceive(sensorDataQueue, &data, portMAX_DELAY)) {
      Serial.print("Temp: ");
      Serial.print(data.temperature);
      Serial.print(" °C | Humidity: ");
      Serial.print(data.humidity);
      Serial.println(" %");
    }
  }
}

// Listens to terminal commands: "start" or "stop"
void TaskCommandListener(void *pvParameters) {
  String input = "";

  while (1) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        input.trim();

        if (input == "start") {
          vTaskResume(sensorTaskHandle);
          Serial.println("Sensor task resumed.");
        } else if (input == "stop") {
          vTaskSuspend(sensorTaskHandle);
          Serial.println("Sensor task suspended.");
        } else {
          Serial.println("Unknown command. Use 'start' or 'stop'.");
        }

        input = ""; // Reset
      } else {
        input += c;
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS); // Avoid busy waiting
  }
}

gkjdfgjlkdfjlgdf
fkjsdlkjfsd
