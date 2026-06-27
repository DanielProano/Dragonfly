# Dragonfly

Dragonfly is the flight controller application for a custom autonomous drone, written in C and built on top of FireFly, a custom preemptive RTOS for the STM32F401 microcontroller. It handles IMU sensor reading, attitude estimation, PID-based stabilization, motor mixing, and communication with a laptop ground station relayed through an ESP32. Dragonfly runs above Buffalo, a cryptographically verified OTA bootloader that occupies the first 32KB of flash and handles secure firmware updates.
