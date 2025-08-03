# 🔥 Heater Control System using Arduino and DS18B20

This project simulates a **heater control system** using an Arduino UNO, a **DS18B20 temperature sensor**, and **two LEDs** to indicate heater and system status. The system adjusts the heater state based on temperature thresholds and simulates different operating conditions.

---

## 🚀 Features

- ✅ Reads temperature using **DS18B20** (OneWire protocol)
- ✅ Controls a simulated heater (LED on pin 12)
- ✅ Status LED (pin 13) blinks based on current system state
- ✅ Handles:
  - Idle
  - Heating
  - Stabilizing
  - Target Reached
  - Overheat

---

## 🔌 Hardware Setup (Simulated in Wokwi)

| Component         | Pin         | Description                       |
|------------------|-------------|-----------------------------------|
| DS18B20 Sensor    | D2          | Temperature data input (OneWire)  |
| Heater LED        | D12         | Simulates heater ON/OFF           |
| Status LED        | D13         | Blinks based on system state      |

> **Resistor**: A 4.7kΩ pull-up resistor is connected between D2 and VCC (as required by OneWire).

---

## 🌡️ Temperature Thresholds

| State             | Range (°C)           | Heater LED (D12) | Status LED (D13)             |
|------------------|----------------------|------------------|------------------------------|
| IDLE             | ≤ 5                  | OFF              | OFF                          |
| HEATING          | 5 < temp ≤ 25        | ON               | Blinks slow (2.5s OFF, 0.5s ON) |
| STABILIZING      | 25 < temp ≤ 30       | ON               | Same as above               |
| TARGET_REACHED   | 30 < temp < 40       | ON               | Same as above               |
| OVERHEAT         | ≥ 40                 | OFF              | Blinks fast (250ms ON/OFF)  |

---

## 📸 Simulation

You can try this simulation directly on Wokwi:

🔗 **[Wokwi Simulation Link](https://wokwi.com/projects/438087336845423617)**

---

## 💾 Libraries Used

Install these libraries from the Arduino Library Manager:

- `OneWire` — for communicating with the DS18B20
- `DallasTemperature` — abstraction for DS18B20 temperature readings

---

