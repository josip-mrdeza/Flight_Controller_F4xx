# Flight Controller F4xx

A custom, bare-metal / HAL embedded flight controller software stack targeting **STM32F4-series** microcontrollers (such as STM32F405 / STM32F411). Designed for RC fixed-wing gliders and multirotor systems, this firmware handles sensor data acquisition, orientation estimation, actuator outputs, and telemetry.

---

## Features

- **Hardware Target:** STM32F4 (ARM Cortex-M4 with FPU) running at high clock frequencies along with a RPI2040 coprocessor (ARM Cortex-M0+).
- **IMU Integration:** High-speed sensor readout (MPU6050 / MPU6500) over I2C/SPI with low-pass filtering.
- **Magnetometer & Barometer Support:** Sensor processing for compass heading and altitude tracking.
- **Long-Range Telemetry:** Wireless link integration (e.g., SX1262 / CC1101 LoRa transceiver modules) for real-time telemetry output.
- **Actuator Control:** PWM signal generation via hardware timers for motor ESCs and control surface servos (ailerons, elevator, rudder).
- **Efficient Processing:** DMA-driven peripheral communication to minimize CPU overhead during real-time loops.

---

## Hardware Requirements

- **MCU:** STM32F4 Discovery / STM32F405 / STM32F411 board + RPI2040 board
- **IMU:** MPU6050 or MPU6500 / MPU9250
- **Radio / Telemetry:** LoRa module (SX1262 or CC1101), currently DXLR30 868/915 MHz
- **Display (Optional):** SSD1306 / SSD1315 OLED for status telemetry
- **Programmer:** ST-Link V2 / V3

---

## Project Structure

```text
Core/
 ├── Inc/          # Header files for drivers, PID algorithms, and system config
 ├── Src/          # Driver implementations, interrupt handlers, main loop
Drivers/           # STM32 HAL peripheral drivers
