/*
   Simple logic analyzer. Triggers when the data pin goes low.
   Records bits until 30  1's in a row are received.
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

  Serial.begin(1000000);
  Serial.println("Hello logic_analyzer");
}

const int baudDelay=3333;
int data = 0;

void loop() {
  data = digitalRead(dataPin);
  if (data == LOW) {
    int numOnes = 0;
    digitalWrite(LED_BUILTIN, HIGH);
//    Serial.print("START: ");
  //  unsigned long started = micros();
    // Serial.print(0); Serial.print(", 0,  ");
    int index = 0;
    int allData[9];
    int theKey=0;
    delayMicroseconds(baudDelay/2);
    while (index <= 8) {
     // unsigned long now = micros();
      data = digitalRead(dataPin);
      //      Serial.print(now - started);
      allData[index++] = data;
      if (data == HIGH) {
        theKey = (theKey>>1)|0x80;
        //Serial.print(", 1,  ");
        //Serial.print(1);
        numOnes++;
      } else {
        theKey = (theKey>>1);
        numOnes = 0;
        //        Serial.print(", 0,  ");
        //Serial.print(0);
      }
      delayMicroseconds(baudDelay);
    }
    Serial.print("TYPED: ");
    Serial.print((char)theKey);
    Serial.print(" (");
    for (int i = 0; i < index; ++i) {
      Serial.print(allData[i]); 
    }
    Serial.println(")");
    digitalWrite(LED_BUILTIN, LOW);
//    Serial.println("END");
  }
}
