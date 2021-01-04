#include <ThreeWire.h>
#include <RtcDS1302.h>

#include "LEDDisplayDriver.h"

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html

#define numberOfDigits 16
const byte dataPin = A4;
const byte clockPin = A5;

#define chargePin 3
#define dischargePin 4
#define resistorValue 99700.0
const byte analogPin = A1;

LEDDisplayDriver display(dataPin, clockPin, true, numberOfDigits);

char dateTimeString[32];

void setup() {
  Serial.begin(9600);
  Serial.println("Captester compiled on");

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  populateTimeString(compiled);
  Serial.println(dateTimeString);

  display.showText("Hello Captester");
  delay(500);
  display.showText(__DATE__);
  delay(500);
  display.showText(__TIME__);
  delay(500);

  Serial.println("Hello serial Captester");
  pinMode(chargePin, OUTPUT);
  pinMode(dischargePin, INPUT);
  digitalWrite(chargePin, LOW);
  Serial.println("after setting the pins");
}


void loop() {
  //display.showText("Discharging");
  Serial.println("Discharging");
  digitalWrite(chargePin, LOW);
  pinMode(dischargePin, OUTPUT);
  digitalWrite(dischargePin, LOW);
  while (analogRead(analogPin) > 0) {}
  pinMode(dischargePin, INPUT);

  Serial.println("Charging");
  //display.showText("Charging");
  digitalWrite(chargePin, HIGH);
  unsigned long startTime = micros();
  while (analogRead(analogPin) < 648) {
  }
  float elapsed = micros() - startTime;
  // display.showText("Elapsed");
  //  display.showNumRight(elapsed);
  Serial.print("elapsed"); Serial.println(elapsed);
  float microFarads = (elapsed / resistorValue);
  Serial.print("uf"); Serial.println(microFarads);
  delay(100);
  if (microFarads >= 1) {
    display.showText("uF:");
    display.showNum2decimals(microFarads, 3, 4);
  } else {
    display.showText("nF:");
    display.showNum2decimals(microFarads * 1000.0, 3, 4);
  }
  delay(5000);
}


#define countof(a) (sizeof(a) / sizeof(a[0]))

void populateTimeString(const RtcDateTime& dt) {
  int hour = dt.Hour();
  boolean ampm = hour >= 12;
  if (hour > 12) {
    hour -= 12;
  }
  snprintf_P(dateTimeString,
             countof(dateTimeString),
             PSTR("%02u:%02u %s"),
             hour,
             dt.Minute(), ampm ? "PM" : "AM"
            );
}
