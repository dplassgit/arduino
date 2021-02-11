
#include "LEDDisplayDriver.h"

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html

#define NUM_DIGITS 8
const byte dataPin = A0;
const byte clockPin = A1;
const byte loadPin = A2;

LEDDisplayDriver display(dataPin, clockPin, loadPin, true, NUM_DIGITS);

void setup() {
  Serial.begin(9600);
  Serial.println("Hello led7test");

  for (int i = 0; i < 255; ++i) {
    if (!isprint(i)) continue;
    display.showChar(i % NUM_DIGITS, i);
    delay(200);
  }
  display.showTextScroll("88888888");
  Serial.println("Tell me something");
}


char displayBuffer[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};

void loop() {
  char c = Serial.read();
  if (! isprint(c)) return; // only printable!

  // scroll down display
  for (int i = 0; i < 7; ++i) {
    displayBuffer[i] = displayBuffer[i + 1];
  }
  displayBuffer[7] = c;
  for (int i = 0 ; i < 8; ++i) {
    display.showChar(i, displayBuffer[i]);
  }
  Serial.print(c);
  delay(200);
}
