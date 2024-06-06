# DDR-using-MSPM0

## Overview
DDR is a remake of the classic rhythm game Dance Dance Revolution in which players press buttons corresponding with arrows moving up the screen towards a target zone while a song is playing.

## Background 
This project was developed as a final project for my Intro to Embedded Systems class at UT Austin. Using a LP-MSPM0G3507, we were to implment a game in C++ that consists of buttons, a slide-pot sampled by the ADC (software developed in a previous lab) at a periodic rate according to Nyquist Theorem, moving sprites, and sounds generated by the DAC (software developed in a previous lab).

## Setup
This project was configured with two LP-MSPM0G3507 microcontrollers and the LCD display used was the "1.8" Color TFT LCD display with MicroSD Card Breakout - ST7735R" by Adafriut. The circuit is built on a breadboard.

### Buttons
The four buttons are configured with positive logic with a 10k resistor. They're arranged horizontally level to each other and are assigned to pins PB0-PB3. 

### Slidepot
A slidepot is connected to PB18.

### LCD 
The pin assignments for the LCD are shown below.
![ST7735pins](https://github.com/nathan-dlr/DDR-using-MSPM0/assets/154288475/7c1a1716-b01d-4a88-8e1b-25313032e0a8)


