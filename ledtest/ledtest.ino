
#include "LEDDisplayDriver.h"

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html

const byte numberOfDigits = 16;
const byte dataPin = A4;
const byte clockPin = A5;
const byte loadPin = A2;

LEDDisplayDriver display(dataPin, clockPin, loadPin);

void setup() {
  Serial.begin(9600);
  Serial.println("Hello ledtest");
  //
  //  for (int i = 0; i < 255; ++i) {
  //    if (!isprint(i)) continue;
  //    display.showChar(i % 4, i);
  //    delay(200);
  //  }
  // // display.showDigit(SEG_A | SEG_B | SEG_H | SEG_G | SEG_F, 0);
  //  delay(5000);
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
