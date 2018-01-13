# PS2Serial
Schematic and Arduino code for building your own PS/2 to Serial mouse adapter to be used with vintage computers that lack support for PS/2 mouse. Intended as a cheap and much more reliable DIY alternative to a vintage serial mouse and it allows for a modern laser mouse to be used with a vintage computer that would otherwise not support it.

The adapter emulates a 3 button Logitech serial mouse but should be Microsoft Mouse compatible though only 2 buttons will be usable. Scrollwheel is not supported and will likely never be implemented as hardly any software or operating system supports a serial mouse with a scroll wheel.

This project was prototyped using an Arduino Leonardo but it should work with any 5v Arduino with a built-in serial UART with little to no modification to the Arduino code. The adapter worked flawlessly on a 386DX connected to one of its serial ports.

## Features
* Logitech 3-button mouse compatible.
* ~40 reports per second event rate (when not holding down the middle button).
* Easy to configure Arduino sketch code to suit your build.

## Requirements
* Any 5v Arduino with built-in UART.
* 74LS00 NAND gate (74HC00 will also do and may allow for 12v output (untested)).
* LM7805 or AM7805 voltage regulator.
* 200ohm resistor.
* DB9 female connector (for connecting to PC).
* 6-pin mini-DIN female connector (for connecting PS/2 mouse).

You will also need the PS2Mouse Arduino library by kristopher on Github:
https://github.com/kristopher/PS2-Mouse-Arduino

## Disclaimer

As with all DIY electronics projects, this project comes with absolutely no warranty and will not liable for any loss or damage caused to your Arduino or vintage PC you used this adapter on. Use at your own risk!