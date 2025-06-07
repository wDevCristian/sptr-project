#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>
#include <DHT.h>
#include "queue.h"

#define I2C_ADDR    0x27
#define LCD_COLUMNS 20
#define LCD_LINES   4

// --- Configurare senzori ---
#define DHTPIN 2
#define DHTTYPE DHT22
#define MQ2PIN A0

// --- Butoane ---
#define BUTTON_UP 27
#define BUTTON_DOWN 29

#define GasThreshold 400

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);

typedef struct {
  float temperature;
  float humidity;
  float heatIndex;
  unsigned int co2ppm;
} AirData;

QueueHandle_t queueSensorData;

TaskHandle_t TaskHandleLCD;
TaskHandle_t TaskHandleReadSensors;

int readInterval = 2000;

void setup() {
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  Serial.begin(9600);

  queueSensorData = xQueueCreate(5, sizeof(AirData));

  xTaskCreate(TaskLCD, "LCD", 128, NULL, 1, &TaskHandleLCD);
  xTaskCreate(TaskReadSensors, "ReadSensors", 128, NULL, 2, &TaskHandleReadSensors);

}

void TaskReadSensors(void *pvParameters) {
  AirData data;
  dht.begin();
  for (;;) {
    // float h = dht.readHumidity();
    // float t = dht.readTemperature();

    // if (isnan(h) || isnan(t)) {
    //   Serial.println(F("Failed to read from DHT sensor!"));
    // }

    //data.humidity = h;
    //data.temperature = t;
    //data.heatIndex = dht.computeHeatIndex(t, h, false);

    data.co2ppm = analogRead(MQ2PIN); 

    xQueueSend(queueSensorData, &data, 0);
    
    vTaskDelay(pdMS_TO_TICKS(readInterval));
  }
}


void TaskLCD(void *pvParameters){
  lcd.init();
  lcd.backlight();
  uint8_t lcdIndex=11;

  for(;;){
    AirData data;

    if (xQueueReceive(queueSensorData, &data, portMAX_DELAY) == pdTRUE)
    {
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(data.temperature);
      lcd.print(" C  ");

      lcd.setCursor(0, 1);
      lcd.print("Hum:  ");
      lcd.print(data.humidity);
      lcd.print(" %   ");

      lcd.setCursor(0, 2);
      if (data.co2ppm > GasThreshold)
      {
        lcd.print("Gas: High Level!");
      }else
      {
        lcd.print("Gas: Safe Level");
      }

      lcd.print("     ");
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void loop() {}