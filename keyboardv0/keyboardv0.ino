/*
   Simple keyboard interface. Triggers when the data pin goes low.
   Records 8 bits at 110 baud.
*/

const int dataPin = 2;      // pin 6 on the KB connector (and pin 6 of the RJ12)
const int resetPin = 5;     // pin 2 on the KB connector (and pin 4 on the RJ12)
const int fakeColumn = 8;

void setup() {
  pinMode(resetPin, OUTPUT);
  pinMode(dataPin, INPUT);
  pinMode(fakeColumn, OUTPUT);
  DDRB |= B00000001;// make pin 8 an output.

  Serial.begin(9600);
  Serial.println("Hello keyboard v0");

  digitalWrite(fakeColumn, LOW);
  // Set the reset pin low for 10 ms.
  digitalWrite(resetPin , LOW);
  delay(10);
  digitalWrite(resetPin , HIGH);
}

// Microseconds to wait between bits. Corresponds to 110 baud, empirically determined.
const int baudDelay = 3333;
int allData[9];

void loop() {
  int data = digitalRead(dataPin);
  if (data == LOW) {
    int index = 0;
    int theKey = 0;

    PORTB = B00000001;
    //digitalWrite(fakeColumn, HIGH);
    // one nop = 62.5ns. need it to be 1.4 microseconds = 22ish
    __asm__("nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t");
    // delayMicroseconds(2);
    //digitalWrite(fakeColumn, LOW);
    PORTB = B00000000;
 
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
