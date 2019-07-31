# SSD1306 on ATtiny45

## Overview

This project contains a software implementation of an I2C interface, which is then used to operate the SSD1306 (128x64 monochrome) display.
The project was built and tested with the bare minimum components: 

* 1x ATtiny45 (ATtiny85 would also work)
* 1x SSD1306 display unit with I2C interface
* 2x 1k resistors (to pull up the I2C lines)
* (optionally) 1x 10k pull-up resistor for RESET pin

## Implementation specifics

One can remap the I2C pins by re-defining the `MY_SCL` and `MY_SDA` macros in the `main.c` file to the corresponding port letter and pin number. 
