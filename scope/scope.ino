// Silly little oscilloscope.

// potentiometer wiper (middle terminal) connected to analog pin 3
// outside leads to ground and +5V
int analogPin = A3;
// Pin to trigger
const int triggerPin = 2;

int trigger = 0;
int val = 0;  // variable to store the value read
int shouldRead = 0;
int ones = 0;

void setup() {
  pinMode(triggerPin, INPUT);

  Serial.begin(1000000);
  Serial.println("Hello trigger scope");
}

void loop() {
  trigger = digitalRead(triggerPin);
  if (!shouldRead && trigger == LOW) {
    Serial.println("triggered");
    shouldRead = 1;
    ones = 0;
  }
  if (shouldRead) {
    Serial.print("kb "); Serial.print(trigger); Serial.print(" Analog: ");
  }
  if (shouldRead && trigger == HIGH) {
    // Stop reading after 30 ones in a row.
    ones++;
    if (ones == 300) {
      Serial.println("untriggered");
      shouldRead = 0;
    }
  }
  if (shouldRead) {
    // read the analog port
    // send back to the result as an appropriately formatted string
    val = analogRead(analogPin);  // read the input pin

    // Scale to 80 chars wide, assuming a 10bit A/D
    int width = 80 * (val / 1024.0);
    for (int x = 0; x < width; ++x) {
      Serial.print(" ");
    }
    Serial.print("* ");
    Serial.println(5.0*val/1024);          // debug value
  }
}
