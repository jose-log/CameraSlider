# Camera Slider
A 1-axis stepper motor camera slider controller

A neat application for stepper motors is by controlling camera sliders. They can include 1, 2 or 3 motors depending on the degrees of freedom needed for the filming task.

This project starts with only one motor. Once the algorithm is ready, I'll explore the possibilities with it.

## Core implementation:
Stepper motor control implementing soft movements is not a menial task for an 8-bit MCU. Based on the implementation of the [AccelStepper] library available for Arduino, which in turn is based on the algorithm described in the 2005 [paper by David Austin] "Generate stepper-motor speed profiles in real time", I made my own implementation of this algorithm.

## Features:
- Interrupt-based computation of timer coefficients. This offers great flexibility when dealing with multiple components in the system like rotary encoders, LCD displays and buttons, since the execution of routines for these modules do not affect the accurate periodic execution of motor timing parameters.
- Two modes of operation:
	* **Manual movement**: The user controls the slider using a rotary encoder, either changing its position or its velocity.
	* **Automatic movement**: The user configures some movement parameters like initial/final position, time, repetitions, etc. and once ready, the slider executes the movement automatically.
- Debug messages are reported using the UART serial interface, which comes in very handy when reviewing computantions or checking for code execution flow.

## Toolchain:
This project is based on an [Arduino Nano] platform, but does not use the Arduino IDE, nor the Arduino bootloader. Instead, the standard AVR toolchain was used:
- [avr-gcc] for compiling and building.
- [avr-libc] for AVR libraries.
- [avrdude] for programming, using [usbasp] programmer.
There're many guides on how to install and set-up the toolchain. [Here's one] of many guides available.

## Project folders
- **main**: contains all the code files needed. This is the one that should be used to implement the stepper motor control.
- **accelStepper**: test code based on the [AccelStepper] library. Not particularly useful.
- **computingTime**: test code to validate execution times for the motor timing parameters. Particularly useful for decission making on the algorithm simplicity, stepping mode selected and variable types for computation. It's disorganized, but it's working.

## Hardware characteristics:
- Controller platform: Arduino Nano (ATmega328p MCU)
- Stepper motor driver: A4988 module
- Display: 16x2 LCD based on the Hitachi HD44780
- Power: 12V for motor. 12V for arduino (via Arduino Nano linear regulator). 5V for LCD (via Arduino Nano linear regulator)
- Limit switch at the beginning of the slider

[AccelStepper]: <http://www.airspayce.com/mikem/arduino/AccelStepper/>
[paper by David Austin]: <https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=4&cad=rja&uact=8&ved=2ahUKEwjD0NzFzProAhWCmuAKHZ4MAvUQFjADegQIBRAB&url=https%3A%2F%2Fforum.arduino.cc%2Findex.php%3Faction%3Ddlattach%3Btopic%3D449173.0%3Battach%3D195514&usg=AOvVaw2CUw-VjgaUTza02KF_hWdM>
[Arduino Nano]: <https://store.arduino.cc/usa/arduino-nano>
[avr-gcc]: <https://www.microchip.com/mplab/avr-support/avr-and-arm-toolchains-c-compilers>
[avr-libc]: <https://www.nongnu.org/avr-libc/user-manual/overview.html>
[avrdude]: <https://www.nongnu.org/avrdude/>
[usbasp]: <https://www.fischl.de/usbasp/>
[Here's one]: <http://maxembedded.com/2015/06/setting-up-avr-gcc-toolchain-on-linux-and-mac-os-x/>
