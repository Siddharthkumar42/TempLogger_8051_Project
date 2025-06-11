# 🌡️ 8051 Temperature Logger with LCD, EEPROM, and Buzzer Alert

This project implements a real-time temperature data logging system using the **8051 microcontroller**, displaying live temperature on an **LM044L 20x4 LCD**, storing readings in **I2C EEPROM**, and triggering a **buzzer alert** when a temperature threshold is exceeded.

## 🔧 Features

- 📟 20x4 LCD Display (LM044L)
- 🔄 SPI Communication with ADC (e.g., MCP3208)
- 💾 I2C EEPROM Data Storage (24C02)
- 🔊 Buzzer Alert on High Temperature
- 📥 External Interrupt to View Last 5 Logged Readings
- 🧮 Temperature scaling and EEPROM memory management

---

## 📁 Components Used

| Component         | Description                         |
|------------------|-------------------------------------|
| AT89C51 (8051 MCU) | Core microcontroller                |
| LM044L           | 20x4 Alphanumeric LCD               |
| MCP3208 / SPI ADC| 12-bit ADC for analog temperature   |
| 24C02 EEPROM     | I2C-based data storage              |
| Buzzer           | Alerts user on over-temperature     |
| External Switch  | Triggers EEPROM temperature display |

---

## System Workflow

1. Continuously reads temperature via **SPI ADC**.
2. Displays live temperature on **LCD**.
3. Scales and stores readings to **I2C EEPROM** (8-bit).
4. Pressing external switch triggers **INT0**, displays last 5 logs.
5. If the temperature exceeds **40°C**, the buzzer alerts and "EMERGENCY ALERT" is shown.

---

## Tools & Languages

- Embedded C (Keil µVision)
- Proteus 8 (Simulation & Circuit Design)
- 8051 Assembly Structure
- GitHub for version control

---

## 🔁 How to Use

1. Flash the hex code to your 8051 microcontroller or simulate in Proteus.
2. Observe live temperature updates on the LCD.
3. Trigger INT0 (external switch) to see the last 5 logged temperatures.
4. If the temperature exceeds the set threshold, the buzzer will alert.

---
## 📷 Screenshot


![image](https://github.com/user-attachments/assets/900c5f83-33b6-485a-b3d9-206769631831)


