# Keyboard v0

An Arduino sketch to retrieve serial binary from the Vector Graph 4 keyboard.

After an active-low trigger, the first bit is skipped, then
subsequent bits are shifted in LSB to MSB, for 7 bits.
