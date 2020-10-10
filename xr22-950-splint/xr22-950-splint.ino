/*
   Fixes the behavior of the XR22-950-3A chip in the Vector Graphic 4 keyboard.

   It listens to the 4 control pins (ABCD) and sends a pulse on the corresponding output pin
   for the broken pins on my XR22-950-3A.

   It's called "splint" because it only replicates part of the XR22-950 functionality.

   See https://docs.google.com/spreadsheets/d/1VIy-GSjdG2L6Flt3NPSkQuhC4tW-Hb9asVCBoYnCuk4/edit
   for the key matrix in spreadsheet form.
*/

// This MUST be an interrupt-enabled pin.
const int outputEnabledPin = 2;

// Control (input) pins: PF4 (A) through PF7 (D), a.k.a A0 through A3
// These aren't used because we need to use direct port manipulation to 
// read all 4 pins at the same time.
// const int aPin = A3;
// const int bPin = A2;
// const int cPin = A1;
// const int dPin = A0;

// Output pins are PB2 through PB6.
// These aren't used because we need to use direct port manipulation to 
// set the appropriate pin high for a very short duration, and digitalWrite
// is too slow.
// const int y0 = 16; // PB2, XR22-950 pin 12 (Y0)
// const int y1 = 14; // PB3, XR22-950 pin 13 (Y1)
// const int y2 = 8;  // PB4, XR22-950 pin 14 (Y2)
// const int y3 = 9;  // PB5, XR22-950 pin 15 (Y3)
// const int y4 = 10; // PB6, XR22-950 pin 16 (Y4)

void setup() {
  Serial.begin(9600);
  Serial.println("Hello xr22-950");

  pinMode(outputEnabledPin, INPUT);

  // Input (control) pins are PF4 (D) through PF7 (A)
  DDRF = B00000000;
  // Output (splint) pins are PB2 through PB6
  DDRB |= B01111100;
  // Set outputs to low
  PORTB = 0;

  setupSplintPins();

  attachInterrupt(digitalPinToInterrupt(outputEnabledPin), intHandler, FALLING);
}

byte splintPins[16];

void setupSplintPins() {
  for (int i = 0; i < 16; ++i) {
    splintPins[i] = 0;
  }

  splintPins[0] = B00000100; // PB2, XR22-950 Pin 12 (Y0). Restores shift, ctrl, caps lock. Erratic behavior on prototype.
  splintPins[1] = B00001000; // PB3, XR22-950 Pin 13 (Y1): Should restore the right half of the numeric keypad. Untested as of yet.
  splintPins[2] = B00010000; // PB4, XR22-950 Pin 14 (Y2): Should restore the left half of the numeric keypad. Untested as of yet.
  splintPins[3] = B00100000; // PB5, XR22-950 Pin 15 (Y3). Should restore right third of keys (enter, backspace), etc. This didn't work at all; possibly wiring issues.
  splintPins[4] = B01000000; // PB6, XR22-950 Pin 16 (Y4). Restores AZXCV, space. Worked on prototype.
}

volatile int printed = 0; // debugging

// Delay 1.4 microseconds. One nop = 62.5ns, so we use 22 nops (1.375 us)
#define DELAY_14_US "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"

void intHandler() {
  // Read pins a,b,c,d at the same time (upper 4 bits, then shift to the bottom nibble)
  int controlPins = (PINF & B11110000) >> 4;

  if (printed == 100) {
    int a = (controlPins & B00000001),
        b = (controlPins & B00000010) >> 1,
        c = (controlPins & B00000100) >> 2,
        d = (controlPins & B00001000) >> 3;
    // Debugging
    Serial.print("DCBA="); Serial.print(d); Serial.print(c); Serial.print(b); Serial.println(a);
    printed = 0;
  } else {
    printed++;
  }

  byte outputPins = splintPin[controlPins];
  if (outputPins != 0) {
    // Set all outputs on Port B
    PORTB = outputPins;
    __asm__(DELAY_14_US);
    PORTB = B00000000;
  }
}

void loop() {
  // Do nothing. All processing in the interript handler.
}
