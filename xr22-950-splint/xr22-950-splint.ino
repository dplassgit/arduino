/*
 * Fixes the behavior of the XR22-950-3A chip in the Vector Graphic 4 keyboard.
 *
 * It listens to the 4 control pins (ABCD) and sends a pulse on the corresponding output pin 
 * for the broken pins on my XR22-950-3A.
*/

// This MUST be an interrupt-enabled pin.
const int outputEnabledPin = 3;

// Input (control) pins:
const int aPin = 4;
const int bPin = 5;
const int cPin = 6;
const int dPin = 7;

// Output pins
const int y0 = 8;
const int y1 = 9;
const int y2 = 10;
const int y3 = 11;
const int y4 = 12;


void setup() {
  pinMode(outputEnabledPin, INPUT);
  pinMode(aPin, INPUT);
  pinMode(bPin, INPUT);
  pinMode(cPin, INPUT);
  pinMode(dPin, INPUT);

  // Make pins 8-12 outputs
  DDRB |= B00011111;

  Serial.begin(9600);
  Serial.println("Hello xr22-950");

  // TODO: make this a single line? e.g., DATAB = 0;?
  digitalWrite(y0, LOW);
  digitalWrite(y1, LOW);
  digitalWrite(y2, LOW);
  digitalWrite(y3, LOW);
  digitalWrite(y4, LOW);

  attachInterrupt(digitalPinToInterrupt(outputEnabledPin), intHandler, FALLING);
}


volatile int printed = 0;

// Delay 1.4 microseconds. One nop = 62.5ns, so we use 22 nops (1.375 us)
#define DELAY_14_US	"nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"

// D C B A
// TODO: define the other pins.
// 0 1 0 0 = A16 (Y4)
// 0 1 0 1 = A17 (Y5)
void intHandler() {
  // Read pins a,b,c,d at the same time.
  int allPins = (PIND & B11110000) >> 4;
  int a = allPins & B00000001,
      b = (allPins & B00000010) >> 1,
      c = (allPins & B00000100) >> 2,
      d = (allPins & B00001000) >> 3;

  if (printed == 1000) {
    // Debugging
    Serial.print("ABCD="); Serial.print(a); Serial.print(b); Serial.print(c); Serial.println(d);
    printed = 0;
  } else {
		printed++;
  }

  // We *could* use a lookup table for this.
  // TODO: FINISH THE LOGIC.
	int outputPins = 0;
	if (allPins == B00000000) {
    // Pin 12 (Y0)
    // This mostly works. Restores shift, ctrl, caps lock. Erratic behavior sometimes.
    outputPins = = B00000001;
	} else if (allPins == B00000011) {
   	// Pin 15 (Y3)
		// This didn't work at all. Supposed to restore enter etc.
		outputPins = B00000100;
  } else if (allPins == B00001000) {
  	// Pin 16 (Y4)
		// This works, restores AZXCV, space
    outputPins = B00010000;
  }

	if (outputPins != 0) {
		PORTB = outputPins;
    __asm__(DELAY_14_US);
    PORTB = B00000000;
  }
}

void loop() {
  // Do nothing. All processing in the interript handler.
}
