/*
   Simple keyboard interface. Triggers when the data pin goes low.
   Records 8 bits at 110 baud.
*/

const int dataPin = 7;      // pin 6 on the KB connector (and pin 6 of the RJ12)
const int resetPin = 9;     // pin 2 on the KB connector (and pin 4 on the RJ12)
const int aPin = 4;
const int bPin = 5;
const int cPin = 6;
const int dPin = 2;
const int fakeColumn = 8;
const int outputEnabledPin = 3;

void setup() {
  pinMode(resetPin, OUTPUT);
  pinMode(dataPin, INPUT);
  pinMode(outputEnabledPin, INPUT);
  pinMode(fakeColumn, OUTPUT);
  pinMode(aPin, INPUT);
  pinMode(bPin, INPUT);
  pinMode(cPin, INPUT);
  pinMode(dPin, INPUT);
  DDRB |= B00000001;// make pin 8 an output.

  Serial.begin(9600);
  Serial.println("Hello keyboard send pin 15");

  digitalWrite(fakeColumn, LOW);
  // Set the reset pin low for 10 ms.
  digitalWrite(resetPin , LOW);
  delay(10);
  digitalWrite(resetPin , HIGH);

  attachInterrupt(digitalPinToInterrupt(outputEnabledPin), intHandler, FALLING);
}

// Microseconds to wait between bits. Corresponds to 110 baud, empirically determined.
const int baudDelay = 3333;
int allData[9];

int printed = 0;
// A B C D
// 0 0 1 0 = A16 (Y4)
// 1 0 1 0 = A17 (Y5)
void intHandler() {
  // read pins a,b,c,d at the same time.
  int allPins = PIND & B01110100;
  int a = (allPins & B00010000) >> 4,
      b = (allPins & B00100000) >> 5,
      c = (allPins & B01000000) >> 6,
      d = (allPins & B00000100) >> 2;
  if (printed == 1000) {
    Serial.print("int "); Serial.print(a); Serial.print(b); Serial.print(c); Serial.println(d);
    if (a == 1 && b == 1 && c == 0 && d == 0) {
      Serial.print("allpins should match:"); Serial.println(allPins, BIN);
    }
    printed = 0;
  } else printed++;

//    if (allPins == B01000000) {
//      // THis works, restores AZXCV, space
//      // pin 16 (Y4) = 0010
//      PORTB = B00000001;
//      // one nop = 62.5ns. need it to be 1.4 microseconds = 22ish
//      __asm__("nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t");
//      PORTB = B00000000;
//    }
//      if (allPins == B00000000) {
//        // This mostly works. Restores shift, ctrl, caps lock. Erratic behavior sometimes.
//        // pin 12 (Y0) = 0000
//        PORTB = B00000001;
//        // one nop = 62.5ns. need it to be 1.4 microseconds = 22ish
//        __asm__("nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t");
//        PORTB = B00000000;
//      }
  if (allPins == B00110000) {
    if (printed == 0) {
          Serial.println("A15");
    }
    // This iddn't work at all. supposed to restore enter etc.
    // pin 15 (Y3) = 1100 (ABCD)
    PORTB = B00000001;
    // one nop = 62.5ns. need it to be 1.4 microseconds = 22ish
    __asm__("nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t");
    PORTB = B00000000;
  }
}

void loop() {
  int data = digitalRead(dataPin);
  if (data == LOW) {
    int index = 0;
    int theKey = 0;

    // Wait half a cycle so that we're sampling in the middle of the bit.
    delayMicroseconds(baudDelay / 2);
    while (index <= 8) {

      data = digitalRead(dataPin);

      allData[index++] = data;
      // Shift right and possibly shift in a 1.
      theKey >>= 1;
      if (data == HIGH) {
        theKey = theKey | 0x80;
      }

      // Wait for the next bit
      delayMicroseconds(baudDelay);
    }

    Serial.print("YOU TYPED: ");
    Serial.print((char)theKey);
    Serial.print(" ("); Serial.print(theKey); Serial.print("=0b");
    for (int i = index - 1; i >= 0; i--) {
      Serial.print(allData[i]);
    }
    Serial.println(")");
  }
}
