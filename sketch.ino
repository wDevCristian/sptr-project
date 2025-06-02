#include <Arduino_FreeRTOS.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C_STEM.h>

// --- Configurare senzori ---
#define DHTPIN 2
#define DHTTYPE DHT22
#define MQ2PIN A0

// --- Butoane ---
#define BUTTON_UP 3
#define BUTTON_DOWN 4

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C_STEM lcd(0x27, 20, 4);  // LCD I2C address

// Structura pentru stocarea datelor senzorilor
typedef struct {
  float temperature;
  float humidity;
  int co2ppm;
} AirData;

AirData airData; // global shared data

// Handle-uri pentru taskuri
TaskHandle_t monitorTaskHandle = NULL;
TaskHandle_t lcdTaskHandle = NULL;

// --- Conversie simulată CO2 ---
int analogToCO2ppm(int analogValue) {
  return map(analogValue, 0, 1023, 200, 10000);
}

// --- Task: Monitorizare senzori ---
void TaskMonitorAir(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int co2 = analogRead(MQ2PIN);
    co2 = analogToCO2ppm(co2);

    if (!isnan(t) && !isnan(h)) {
      // Protejăm accesul la datele globale
      taskENTER_CRITICAL();
      airData.temperature = t;
      airData.humidity = h;
      airData.co2ppm = co2;
      taskEXIT_CRITICAL();
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// --- Task: Afișare LCD ---
void TaskDisplayLCD(void *pvParameters) {
  (void) pvParameters;

  float temp, hum;
  int co2;

  for (;;) {
    // Copiem datele protejat
    taskENTER_CRITICAL();
    temp = airData.temperature;
    hum = airData.humidity;
    co2 = airData.co2ppm;
    taskEXIT_CRITICAL();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temp, 1);
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Umid: ");
    lcd.print(hum, 1);
    lcd.print(" %");

    lcd.setCursor(0, 2);
    lcd.print("CO2: ");
    lcd.print(co2);
    lcd.print(" ppm");

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void setup() {
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  Serial.begin(9600);
  //Wire.begin();       // Necesare pentru I2C
  dht.begin();
  lcd.init();         // Moved here from task
  lcd.backlight();

  // Creăm taskurile cu stack mai mare
  xTaskCreate(TaskMonitorAir, "Monitor", 384, NULL, 2, &monitorTaskHandle);
  xTaskCreate(TaskDisplayLCD, "Display", 384, NULL, 1, &lcdTaskHandle);
}

void loop() {
  // Nu este folosit - FreeRTOS rulează taskurile
}
