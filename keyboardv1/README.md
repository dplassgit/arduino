# Keyboard v1

An Arduino sketch to retrieve serial binary from the Vector Graph 4 keyboard.

After an active-low trigger, the first bit is skipped, then
subsequent bits are shifted in LSB to MSB, for 7 bits.

Then, a simple lookup is used to translate to the key to send via USB.

This version must be run on an Arduino that [CITATION NEEDED] can send back 
USB commands as keyboard commands.

