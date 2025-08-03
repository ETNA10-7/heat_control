#include <OneWire.h>
#include <DallasTemperature.h>

// Pin assignments
#define ONE_WIRE_BUS 2         // DS18B20 data pin connected to pin 2
#define HEATER_PIN 12          // Heater simulation LED
#define STATUS_LED_PIN 13      // Status blinking LED

// Temperature thresholds
#define IDLE_TEMP 5
#define TEMP_ON_THRESHOLD 25.0
#define TEMP_OFF_THRESHOLD 30.0
#define TEMP_OVERHEAT 40.0

// States
enum HeaterState { IDLE, HEATING, STABILIZING, TARGET_REACHED, OVERHEAT };
HeaterState state = IDLE;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// LED blinking logic
unsigned long lastBlinkTime = 0;
bool ledState = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(HEATER_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  sensors.begin();
  Serial.println("Heater Control System Initialized");
}

void loop() {
  static unsigned long lastSensorRead = 0;
  unsigned long currentMillis = millis();

  if (currentMillis - lastSensorRead >= 2000) {
    lastSensorRead = currentMillis;

    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);

    updateState(temperature);
    logStatus(temperature);
  }

  handleStatusLED();  // Only blinks STATUS_LED_PIN
}

void updateState(float temp) {
  // Overheat protection
  if (temp >= TEMP_OVERHEAT) {
    digitalWrite(HEATER_PIN, LOW);  // Heater OFF
    state = OVERHEAT;
    return;
  }

  switch (state) {
    case IDLE:
      digitalWrite(HEATER_PIN, LOW);  // Heater OFF
      if (temp > IDLE_TEMP && temp <= TEMP_ON_THRESHOLD) {
        digitalWrite(HEATER_PIN, HIGH);  // Heater ON
        state = HEATING;
      }else if (temp > TEMP_ON_THRESHOLD && temp <= TEMP_OFF_THRESHOLD) {
        digitalWrite(HEATER_PIN, HIGH);  // Heater ON
        state = STABILIZING;
      }else if (temp > TEMP_OFF_THRESHOLD && temp < TEMP_OVERHEAT) {
        digitalWrite(HEATER_PIN, HIGH);  // Heater ON
        state = TARGET_REACHED;
      }else if( temp > TEMP_OVERHEAT) {
        digitalWrite(HEATER_PIN, LOW);
        state = OVERHEAT;
      }
      break;

    case HEATING:
      if (temp <= IDLE_TEMP) {
        digitalWrite(HEATER_PIN, LOW);  // Heater OFF
        state = IDLE;
      } else if (temp >= TEMP_ON_THRESHOLD && temp <= TEMP_OFF_THRESHOLD) {
        digitalWrite(HEATER_PIN, HIGH);  // Stay ON
        state = STABILIZING;
      } else if (temp >= TEMP_OFF_THRESHOLD ) {
        digitalWrite(HEATER_PIN, HIGH);  // Stay ON
        state = TARGET_REACHED;
       } //else if (temp > TEMP_OVERHEAT) {
      //   digitalWrite(HEATER_PIN, LOW);  // Stay ON
      //   state = OVERHEAT;
      // }
      break;

    case STABILIZING:
      digitalWrite(HEATER_PIN, HIGH);  // Heater ON
      if (temp < TEMP_ON_THRESHOLD) {
        state = HEATING;
      } else if (temp > TEMP_OFF_THRESHOLD) {
        state = TARGET_REACHED;
      }
      break;

    case TARGET_REACHED:
      digitalWrite(HEATER_PIN, HIGH);  // Heater ON
      if(temp < IDLE_TEMP){
        state = IDLE;
      }else if (temp < TEMP_ON_THRESHOLD){
        state = HEATING;
      }else if (temp <= TEMP_OFF_THRESHOLD) {
        state = STABILIZING;
      }else if(temp > TEMP_OVERHEAT){
        state = OVERHEAT;
      }
      break;

    case OVERHEAT:
      digitalWrite(HEATER_PIN, LOW);  // Heater OFF
      if (temp < TEMP_OVERHEAT) {
        // Resume heating logic
        if (temp < IDLE_TEMP){
          digitalWrite(HEATER_PIN, LOW);
          state = IDLE;
        }else if (temp < TEMP_ON_THRESHOLD) {
          digitalWrite(HEATER_PIN, HIGH);
          state = HEATING;
        } else if (temp <= TEMP_OFF_THRESHOLD) {
          digitalWrite(HEATER_PIN, HIGH);
          state = STABILIZING;
        } else {
          digitalWrite(HEATER_PIN, HIGH);
          state = TARGET_REACHED;
        }
      }
      break;
  }
}


void handleStatusLED() {
  static unsigned long previousMillis = 0;
  static bool ledStateLocal = LOW;
  unsigned long currentMillis = millis();
  unsigned long onTime, offTime;

  switch (state) {
    case HEATING:
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
      digitalWrite(STATUS_LED_PIN, LOW);  // OFF in other states
      ledStateLocal = LOW;
      return;
  }

  if (ledStateLocal == HIGH && (currentMillis - previousMillis >= onTime)) {
    digitalWrite(STATUS_LED_PIN, LOW);
    ledStateLocal = LOW;
    previousMillis = currentMillis;
  } else if (ledStateLocal == LOW && (currentMillis - previousMillis >= offTime)) {
    digitalWrite(STATUS_LED_PIN, HIGH);
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
