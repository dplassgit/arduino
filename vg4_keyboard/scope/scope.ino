#include <Arduino_DebugUtils.h>

// Silly little oscilloscope.

int analogPin = A3; // potentiometer wiper (middle terminal) connected to analog pin 3
// outside leads to ground and +5V
int val = 0;  // variable to store the value read
void setup() {
  Serial.begin(9600);
  Serial.println("hi");
}

void loop() {
  // read the analog port
  // send back to the result as an appropriately formatted string
  val = analogRead(analogPin);  // read the input pin

  // Scale to 80 chars wide, assuming a 10bit A/D
  int width=80*(val/1024.0);
  for (int x = 0; x < width; ++x) {
    Serial.print(" ");
  }
  Serial.println(val);          // debug value
}
