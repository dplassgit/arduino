/*
   Simple logic analyzer. Triggers when the data pin goes low.
   Records bits until 30  1's in a row are received.
*/

const int dataPin = 2;      // corresponds to pin 6 on the KB connector (and pin 6 of the RJ12)
const int resetPin = 5;     // corresponds to pin 2 on the KB connector (and pin 4 on the RJ12)

int changed = 0;

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

int data = 0;
int inKey = 0;  // are we reading a key?
int numOnes = 0;  // the # of 1s in a row

void loop() {
  data = digitalRead(dataPin);
  if (data == LOW && !inKey) {
    inKey = 1; // we're recording until numones==30
    numOnes = 0;
    Serial.println("START");
    Serial.println("0");
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (inKey) {
    if (data == HIGH) {
      Serial.println("1");
      numOnes++;
      if (numOnes == 30) {
        inKey = 0;
        numOnes = 0;
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("END");
        delay(500);
      }
    } else {
      numOnes = 0;
      Serial.println("0");
    }
  }
}
