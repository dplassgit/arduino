/*
   Simple keyboard interface. Triggers when the data pin goes low.
   Records 8 bits at 110 baud.
*/

const int dataPin = 2;      // corresponds to pin 6 on the KB connector (and pin 6 of the RJ12)
const int resetPin = 5;     // corresponds to pin 2 on the KB connector (and pin 4 on the RJ12)

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(dataPin, INPUT);

  // Set the reset pin low for 10 ms.
  digitalWrite(resetPin , LOW);
  delay(10);
  digitalWrite(resetPin , HIGH);

  Serial.begin(9600);
  Serial.println("Hello logic_analyzer");
}

// Microseconds to wait between bits
const int baudDelay = 3333;

void loop() {
  int data = digitalRead(dataPin);
  if (data == LOW) {
    digitalWrite(LED_BUILTIN, HIGH);
    //    Serial.print("START: ");
    int index = 0;
    int allData[9];
    int theKey = 0;
    // Wait half a cycle so that we're sampling in the middle of the bit.
    delayMicroseconds(baudDelay / 2);
    while (index <= 8) {
      data = digitalRead(dataPin);
      allData[index++] = data;
      if (data == HIGH) {
        theKey = (theKey >> 1) | 0x80;
      } else {
        theKey = (theKey >> 1);
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
    digitalWrite(LED_BUILTIN, LOW);
  }
}
