/*
   Fixes the behavior of the XR22-950-3A chip in the Vector Graphic 4 keyboard.

   It listens to the 4 control pins (ABCD) and sends a pulse on the corresponding output pin
   for the broken pins on my XR22-950-3A.
*/

// This MUST be an interrupt-enabled pin.
const int outputEnabledPin = 2;

// NOTE: pin 3 is NC

// Input (control) pins:
const int aPin = 4;
const int bPin = 5;
const int cPin = 6;
const int dPin = 7;

// Output pins
const int y0 = 8; // Arduino pin 8, XR22-950 pin 12 (Y0)
const int y1 = 9; // Arduino pin 9, XR22-950 pin 13 (Y1)
const int y2 = 10; // Arduino pin 10, XR22-950 pin 14 (Y2)
const int y3 = 11; // Arduino pin 16, XR22-950 pin 15 (Y3)
const int y4 = 12; // Arduino pin 14, XR22-950 pin 16 (Y4)

void setup() {
  Serial.begin(9600);
  Serial.println("Hello xr22-950");

  pinMode(outputEnabledPin, INPUT);
  pinMode(aPin, INPUT);
  pinMode(bPin, INPUT);
  pinMode(cPin, INPUT);
  pinMode(dPin, INPUT);

  pinMode(y0, OUTPUT);
  pinMode(y1, OUTPUT);
  pinMode(y2, OUTPUT);
  pinMode(y3, OUTPUT);
  pinMode(y4, OUTPUT);

  digitalWrite(y0, LOW);
  digitalWrite(y1, LOW);
  digitalWrite(y2, LOW);
  digitalWrite(y3, LOW);
  digitalWrite(y4, LOW);

  setupSplintPins();

  attachInterrupt(digitalPinToInterrupt(outputEnabledPin), intHandler, FALLING);
}

int splintPins[16];

void setupSplintPins() {
  for (int i = 0; i < 16; ++i) {
    splintPins[i] = 0;
  }
  // TODO: FIX THESE!
  splintPins[0] = B00000001; // Arduino pin x, XR22-950 Pin 12 (Y0). This mostly works. Restores shift, ctrl, caps lock. Erratic behavior sometimes.
  splintPins[1] = 0; // Arduino pin x, XR22-950 Pin 13 (Y1):
  splintPins[2] = 0; // Arduino pin x, XR22-950 Pin 14 (Y2):
  splintPins[3] = B00000100; // Arduino pin x, XR22-950 Pin 15 (Y3). This didn't work at all. Supposed to restore enter etc.
  splintPins[3] = B00010000; // Arduino pin x, XR22-950 Pin 16 (Y4). This works, restores AZXCV, space
}

volatile int printed = 0; // debugging

// Delay 1.4 microseconds. One nop = 62.5ns, so we use 22 nops (1.375 us)
#define DELAY_14_US	"nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"

void intHandler() {
  // Read pins a,b,c,d at the same time (upper 4 bits, then shift to the bottom nibble)
  int controlPins = (PIND & B11110000) >> 4;

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

  int outputPins = splintPin[controlPins];

  if (outputPins != 0) {
    // TODO: FIXME
    // PORTB = outputPins;
    __asm__(DELAY_14_US);
    // PORTB = B00000000;
  }
}

void loop() {
  // Do nothing. All processing in the interript handler.
}
