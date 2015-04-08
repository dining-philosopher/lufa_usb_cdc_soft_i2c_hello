# lufa_usb_cdc_soft_i2c_hello

https://github.com/dining-philosopher/lufa_usb_cdc_soft_i2c_hello

AVR USB CDC device with real time clock connected by software I2C.

The clock used is M41T56M6 connected to PF0 leg as SDA and PF1 leg as SCL.

Based on one of LUFA examples.

## Libraries

Needs LUFA-140928 (you need to copy it directly in the program directory or make a soft link with this name)

LUFA: http://www.fourwalledcubicle.com/LUFA.php

LUFA-140928: http://www.github.com/abcminiuser/lufa/archive/LUFA-140928.zip

Also it uses (slightly modified) software I2C library by Avinash Gupta http://extremeelectronics.co.in/avr-tutorials/software-i2c-library-for-avr-mcus/

## Making:

make && make dfu

(if you use direct usb programming)
