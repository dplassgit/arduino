/*
  Button

  Turns on and off a light emitting diode(LED) connected to LED_BUILTIN,
  when pressing a pushbutton attached to pin 2.

  The circuit:
  - pushbutton attached to pin 2 from +5V
  - 10K resistor attached to pin 2 from ground

  created 2005
  by DojoDave <http://www.0j0.org>
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Button
*/

// constants won't change. They're used here to set pin numbers:
const int buttonPin = 7;     // the number of the pushbutton pin
const int dataPin = 2;

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

void setup() {
  // initialize the LED pin as an output:
  pinMode(LED_BUILTIN, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  Serial.begin(9600);
  Serial.println("Hello buttonblink");
}

int changed = 0;
int shouldRead = 0;
int data = 0;
void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH && !changed) {
    // turn LED on:
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("on");
    shouldRead = 1 - shouldRead;
    changed = 1;
  } else if (buttonState == LOW && changed) {
    // turn LED off:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("off");
    changed = 0;
  }

  if (shouldRead == 1) {
    data = digitalRead(dataPin);
    if (data == HIGH) {
      Serial.println("1");
    } else {
      Serial.println("0");
    }
    delay(1);
  }
}
