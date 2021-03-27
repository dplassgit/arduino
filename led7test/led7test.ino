#define min(a,b) ((a<b)?a:b)

#include "LEDDisplayDriver.h"

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html

#define NUM_DIGITS 16
const byte dataPin = D1;
const byte clockPin = D2;
const byte loadPin = D3;

LEDDisplayDriver display(dataPin, clockPin, loadPin, true, NUM_DIGITS);

void setup() {
  Serial.begin(9600);
  Serial.println("Hello led7test");
  display.begin();
  display.setBrightness(1);
  //  for (int i = 0; i < 255; ++i) {
  //    if (!isprint(i)) continue;
  //    display.showChar(i % NUM_DIGITS, i);
  //    delay(100);
  //  }
  display.showTextScroll("Tell me something88888888");
  Serial.println("Tell me something");
}


char displayBuffer[16] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
int num = 0;
void numloop() {
  int ones = num % 10;
  int tens = num / 10;
  display.showChar(0, tens + '0');
  display.showChar(1, ones + '0');
  num++;
  if (num == 100) {
    num = 0;
  }
  Serial.println(num);
  digitalWrite(2, num % 2);
  delay(1000);
}

void loop() {
  char c = Serial.read();
  if (! isprint(c)) {
    return; // only printable!
  }

  // scroll down display
  for (int i = 0; i < NUM_DIGITS - 1; ++i) {
    displayBuffer[i] = displayBuffer[i + 1];
  }
  displayBuffer[NUM_DIGITS - 1] = c;
  for (int i = 0 ; i < NUM_DIGITS; ++i) {
    display.showChar(i, displayBuffer[i]);
  }
  Serial.print(c);
  delay(200);
}
