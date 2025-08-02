#include <OneWire.h>
#include <DallasTemperature.h>

// Pin assignments
#define ONE_WIRE_BUS 2     // DS18B20 data pin connected to pin 2
#define HEATER_PIN  13     // Simulated heater (LED on pin 13)

// Temperature thresholds
#define TEMP_ON_THRESHOLD   25.0
#define TEMP_OFF_THRESHOLD  30.0
#define TEMP_OVERHEAT       40.0

// States
enum HeaterState { IDLE, HEATING, STABILIZING, TARGET_REACHED, OVERHEAT };
HeaterState state = IDLE;

// OneWire and temperature sensor setup
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Blinking logic
unsigned long lastBlinkTime = 0;
bool ledState = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  sensors.begin();
  Serial.println("Heater Control System Initialized");
}

void loop() {
  static unsigned long lastSensorRead = 0;
  unsigned long currentMillis = millis();

  // Read temperature every 2000 ms
  if (currentMillis - lastSensorRead >= 2000) {
    lastSensorRead = currentMillis;

    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);

    updateState(temperature);
    logStatus(temperature);
  }

  handleLED();  // Call continuously for responsive blinking
}


void updateState(float temp) {
  switch (state) {
    case IDLE:
      digitalWrite(HEATER_PIN, LOW);
      if (temp < TEMP_ON_THRESHOLD) {
        digitalWrite(HEATER_PIN, HIGH);
        state = HEATING;
      }
      break;

    case HEATING:
      if (temp >= TEMP_ON_THRESHOLD) {
        digitalWrite(HEATER_PIN, LOW);
        state = STABILIZING;
      }
      break;

    case STABILIZING:
      if (temp < TEMP_ON_THRESHOLD) {
        digitalWrite(HEATER_PIN, HIGH);
        state = HEATING;
      } else if (temp >= TEMP_OFF_THRESHOLD - 1) {
        state = TARGET_REACHED;
      }
      break;

    case TARGET_REACHED:
      if (temp < TEMP_ON_THRESHOLD) {
        digitalWrite(HEATER_PIN, HIGH);
        state = HEATING;
      }
      break;

    case OVERHEAT:
      digitalWrite(HEATER_PIN, LOW);
      if (temp <= TEMP_OFF_THRESHOLD) {
        state = STABILIZING;
      }
      break;
  }

  // Overheat protection (highest priority)
  if (temp > TEMP_OVERHEAT) {
    digitalWrite(HEATER_PIN, LOW);
    state = OVERHEAT;
  }
}

void handleLED() {
  static unsigned long previousMillis = 0;
  static bool ledStateLocal = LOW;

  unsigned long currentMillis = millis();
  unsigned long onTime, offTime;

  // Set blink timing based on state
  switch (state) {
    case STABILIZING:
    case TARGET_REACHED:
      onTime = 500;
      offTime = 2500;
      break;
    case OVERHEAT:
      onTime = 250;
      offTime = 250;
      break;
    default:
      digitalWrite(HEATER_PIN, LOW);  // LED OFF in other states
      ledStateLocal = LOW;
      return;
  }

  // Blinking logic
  if (ledStateLocal == HIGH && (currentMillis - previousMillis >= onTime)) {
    digitalWrite(HEATER_PIN, LOW);
    ledStateLocal = LOW;
    previousMillis = currentMillis;
  } else if (ledStateLocal == LOW && (currentMillis - previousMillis >= offTime)) {
    digitalWrite(HEATER_PIN, HIGH);
    ledStateLocal = HIGH;
    previousMillis = currentMillis;
  }
}



void logStatus(float temp) {
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" °C | State: ");

  switch (state) {
    case IDLE: Serial.println("IDLE"); break;
    case HEATING: Serial.println("HEATING"); break;
    case STABILIZING: Serial.println("STABILIZING"); break;
    case TARGET_REACHED: Serial.println("TARGET_REACHED"); break;
    case OVERHEAT: Serial.println("OVERHEAT"); break;
  }
}
