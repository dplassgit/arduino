# Keyboard v1

An Arduino sketch to retrieve serial binary from the Vector Graphic 4 keyboard.

After an active-low trigger, the first bit is skipped, then
subsequent bits are shifted in LSB to MSB, for 7 bits.

Then, a simple lookup is used to translate to the key to send via USB.

This version must be run on an Arduino with the ATmega32U4 chip (e.g., ProMicro,
Leonardo) that can act as a HID (human input device), i.e., a USB keyboard.
