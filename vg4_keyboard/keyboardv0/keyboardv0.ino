/*
   Simple keyboard interface. Triggers when the data pin goes low.
   Records 8 bits at 110 baud.
*/

const int dataPin = 2;      // pin 6 on the KB connector (and pin 6 of the RJ12)
const int resetPin = 5;     // pin 2 on the KB connector (and pin 4 on the RJ12)

void setup() {
  pinMode(resetPin, OUTPUT);
  pinMode(dataPin, INPUT);

  // Set the reset pin low for 10 ms.
  digitalWrite(resetPin , LOW);
  delay(10);
  digitalWrite(resetPin , HIGH);

  Serial.begin(9600);
  Serial.println("Hello keyboardv0");
}

// Microseconds to wait between bits. Corresponds to 110 baud.
const int baudDelay = 3333;
int allData[9];

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
