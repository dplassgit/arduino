
#include "LEDDisplayDriver.h"

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html

#define NUM_DIGITS 16
const byte dataPin = D0;
const byte clockPin = D1;

LEDDisplayDriver display(dataPin, clockPin, true, NUM_DIGITS);

void setup() {
  Serial.begin(115200);
  Serial.println("Hello ledtest");

  for (int i = 0; i < 255; ++i) {
    if (!isprint(i)) continue;
    display.showChar(i % NUM_DIGITS, i);
    delay(200);
  }
  display.showTextScroll("Tell me something");
}


char displayBuffer[16] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};

void loop() {
  char c = Serial.read();
  if (! isprint(c)) return; // only printable!

  // scroll down display
  for (int i = 0; i < 15; ++i) {
    displayBuffer[i] = displayBuffer[i + 1];
  }
  displayBuffer[15] = c;
  for (int i = 0 ; i < 16; ++i) {
    display.showChar(i, displayBuffer[i]);
  }
  delay(200);
}
