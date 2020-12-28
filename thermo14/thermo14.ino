
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <dht.h>


#include "LEDDisplayDriver.h"

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html

dht DHT;
const byte DHT11_PIN = 2;

ThreeWire myWire(9, 8, 10); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

const byte numberOfDigits = 4;
const byte dataPin = A4;
const byte clockPin = A5;
const byte loadPin = A2;

LEDDisplayDriver display(dataPin, clockPin, loadPin);

void setup() {
  Serial.begin(9600);
  Serial.println("Thermo 14");
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if (!Rtc.IsDateTimeValid()) {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC lost confidence in the DateTime! Setting to compiled.");
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected()) {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning()) {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  Serial.println("RTC says datetime is:");
  printDateTime(now);
  Serial.println();
  if (now < compiled)  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  } else if (now > compiled) {
    Serial.println("RTC is newer than compile time. (this is expected)");
  } else if (now == compiled)  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }
}


char displayBuffer[4] = {' ', ' ', ' ', ' '};
char dateTimeString[20];

void loop() {
  RtcDateTime now = Rtc.GetDateTime();
  display.showTextScroll("Date/time is...    ");
  printDateTime(now);
  Serial.println();
  display.showTextScroll(dateTimeString);
  delay(500);

  if (!now.IsValid()) {
    // Common Causes:
    //    1) the battery on the device is low or even missing and the power line was disconnected
    Serial.println("RTC lost confidence in the DateTime!");
  }
  int chk = DHT.read11(DHT11_PIN);
  double c = DHT.temperature;
  if (c == -999) {
    display.showTextScroll("Waiting for sensor......");
    delay(5000);
    return;
  }
  display.showTextScroll("Temperature is   ");
  delay(300);
  Serial.println(c);
  display.clear();
  double f = (c * 9.0 / 5.0) + 32.0;
  display.showNum(f);
  Serial.println(f);

  delay(5000);
  display.showTextScroll("RH is   ");
  double rh = DHT.humidity;
  Serial.println(rh);
  display.clear();
  display.showNum(rh);

  delay(5000);
}


#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt) {
  snprintf_P(dateTimeString,
             countof(dateTimeString),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(dateTimeString);
}
