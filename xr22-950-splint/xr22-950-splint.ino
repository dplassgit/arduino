/*
   Fixes the behavior of the XR22-950-3A chip in the Vector Graphic 4 keyboard.

   It listens to the 4 control pins (ABCD) and sends a pulse on the corresponding output pin
   for the broken pins on my XR22-950-3A.

   It's called "splint" because it only replicates part of the XR22-950 functionality.

   See https://docs.google.com/spreadsheets/d/1VIy-GSjdG2L6Flt3NPSkQuhC4tW-Hb9asVCBoYnCuk4/edit
   for the key matrix in spreadsheet form.

   THIS VERSION FOR UNO ONLY.
*/

// This MUST be an interrupt-enabled pin.
const int outputEnabledPin = 2;

// Input (control) pins are PD6 (D) through PD3 (A)
// These aren't used because we need to use direct port manipulation to
// read all 4 pins at the same time.
// const int aPin = 3;
// const int bPin = 4;
// const int cPin = 5;
// const int dPin = 6;

// Output (splint) pins are PB0 (pin 8) through PB5 (pin 12)
// These aren't used because we need to use direct port manipulation to
// set the appropriate pin high for a very short duration, and digitalWrite
// is too slow.
// const int y0 = 8;  // PB2, XR22-950 pin 12 (Y0)
// const int y1 = 9;  // PB3, XR22-950 pin 13 (Y1)
// const int y2 = 10; // PB4, XR22-950 pin 14 (Y2)
// const int y3 = 11; // PB5, XR22-950 pin 15 (Y3)
// const int y4 = 12; // PB6, XR22-950 pin 16 (Y4)

void setup() {
  Serial.begin(9600);
  Serial.println("Hello xr22-950-splint for uno");

  pinMode(outputEnabledPin, INPUT);

  // Input (control) pins are PD6 (D) through PD3 (A)
  DDRD = B00000000;
  // Output (splint) pins are PB0 (pin8) through PB5 (pin 12)
  DDRB |= B00011111;
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

  splintPins[0] = B00000001; // PB2, XR22-950 Pin 12 (Y0). Restores shift, ctrl, caps lock.
  splintPins[1] = B00000010; // PB3, XR22-950 Pin 13 (Y1): Should restore the right half of the numeric keypad.
  splintPins[2] = B00000100; // PB4, XR22-950 Pin 14 (Y2): Should restore the left half of the numeric keypad.
  splintPins[3] = B00001000; // PB5, XR22-950 Pin 15 (Y3). Should restore right third of keys (enter, backspace), etc.
  splintPins[4] = B00010000; // PB6, XR22-950 Pin 16 (Y4). Restores AZXCV, space.
}

// One nop is 62NS
#define DELAY_62NS "nop\n\t"
#define DELAY_124NS DELAY_62NS DELAY_62NS
#define DELAY_248NS DELAY_124NS DELAY_124NS
#define DELAY_496NS DELAY_248NS DELAY_248NS
// Really 992 /shrug
#define DELAY_1US DELAY_496NS DELAY_496NS
#define DELAY_2US DELAY_1US DELAY_1US
#define DELAY_3US DELAY_2US DELAY_1US
#define DELAY_4US DELAY_2US DELAY_2US

// Delay 1.4 microseconds. One nop = 62.5ns, so we use 22 nops (1.375 us)
#define DELAY_1_4US "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"

#undef DEBUG

#ifdef DEBUG
#define DBG_COUNT 1000000
volatile int printed = 0; // debugging
#endif

void intHandler() {
  // Read pins a,b,c,d at the same time (upper 4 bits, then shift to the bottom nibble)
  int controlPins = (PIND & B01111000) >> 3;

#ifdef DEBUG
  if (printed == DBG_COUNT) {
    int a = (controlPins & B00000001),
        b = (controlPins & B00000010) >> 1,
        c = (controlPins & B00000100) >> 2,
        d = (controlPins & B00001000) >> 3;
    // Debugging
    Serial.print("DCBA="); Serial.print(d); Serial.print(c); Serial.print(b); Serial.println(a);
  } else {
    printed++;
  }
#endif

  byte outputPins = splintPins[controlPins];
  if (outputPins != 0) {
#ifdef DEBUG
    if (printed == DBG_COUNT) {
      Serial.print("sending blip on 0B"); Serial.println(outputPins, BIN);
    }
#endif

    // Wait 3 us between the OE and the pulse.
    __asm__(DELAY_3US);
    // Set all outputs on Port B
    PORTB = outputPins;
    // The hardware sends a 1.4 microsecond pulse. However we don't necessarily start the pulse
    // at the same time as the hardware would have. This can be as long as 4us.
    __asm__(DELAY_4US);
    PORTB = B00000000;
  }
#ifdef DEBUG
  if (printed == DBG_COUNT) {
    printed = 0;
  }
#endif
}

void loop() {
  // Do nothing. All processing is in the interrupt handler.
}
