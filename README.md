# CameraSlider
A 1-axis stepper motor camera slider controller

A neat application for stepper motors is by controlling camera sliders. They can include 1, 2 or 3 motors depending on the degrees of freedom needed for the filming task.

This project starts with only one motor. Once the algorithm is ready, I'll explore the possibilities with it.

## Core implementation:
Stepper motor control implementing soft movements is not a menial task for an MCU. Based on the implementation of the [AccelStepper](http://www.airspayce.com/mikem/arduino/AccelStepper/) library available for Arduino, which in turn is based on the algorithm described in the 2005 [paper by David Austin](https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=4&cad=rja&uact=8&ved=2ahUKEwjD0NzFzProAhWCmuAKHZ4MAvUQFjADegQIBRAB&url=https%3A%2F%2Fforum.arduino.cc%2Findex.php%3Faction%3Ddlattach%3Btopic%3D449173.0%3Battach%3D195514&usg=AOvVaw2CUw-VjgaUTza02KF_hWdM) "Generate stepper-motor speed profiles in real time", I made my own implementation of this algorithm

## Characteristics:
- Controller platform: Arduino Nano (ATmega328p MCU)
- Stepper motor driver: A4988 module
- Display: 16x2 LCD based on the Hitachi HD44780
- Power: 12V for motor. 12V for arduino (via Arduino Nano linear regulator). 5V for LCD (via Arduino Nano linear regulator)
- Limit switch at the beginning of the slider
