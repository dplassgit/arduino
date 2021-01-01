
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <dht.h>


#include "LEDDisplayDriver.h"

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html

#define numberOfDigits 16
const byte dataPin = A4;
const byte clockPin = A5;

LEDDisplayDriver display(dataPin, clockPin, true, numberOfDigits);
DISPLAY_INTR(display)

char dateTimeString[30];

dht DHT;
#define DHT11_PIN 2

ThreeWire myWire(9, 8, 10); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

void setup() {
  Serial.begin(9600);
  Serial.println("Thermo 14 compiled on");

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  getDateTimeString(compiled);
  Serial.println(dateTimeString);

  display.showTextScroll("Hello thermo14 very long should be ok");
  delay(2000);

  Serial.println("hello serial thermo14");
  Rtc.Begin();
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
  getDateTimeString(now);
  Serial.println(dateTimeString);
  if (now < compiled)  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  } else if (now > compiled) {
    Serial.println("RTC is newer than compile time. (this is expected)");
  } else if (now == compiled)  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }
}


void loop() {
  display.showText("Time is");
  delay(2000);
  RtcDateTime now = Rtc.GetDateTime();
  getDateTimeString(now);
  display.showText(dateTimeString);
  delay(2000);

  if (!now.IsValid()) {
    // Common Causes:
    //    1) the battery on the device is low or even missing and the power line was disconnected
    Serial.println("RTC lost confidence in the DateTime!");
    display.showText("CLOCK BAD!?");
  }
  int chk = DHT.read11(DHT11_PIN);
  double c = DHT.temperature;
  if (c == -999) {
    display.showText("Waiting for sensor");
    delay(2000);
    return;
  }
  display.showTextScroll("Temperature (Fahrenheit)");
  delay(2000);
  Serial.println(c);
  double f = (c * 9.0 / 5.0) + 32.0;
  Serial.println(f);
  display.clear();
  display.showNum2Left(f);
  display.showText("degrees F", 3, 16);

  delay(5000);
  display.showTextScroll("Relative humidity");
  delay(2000);
  double rh = DHT.humidity;
  Serial.println(rh);
  display.clear();
  display.showNum2Left(rh); display.showChar(3, '%');

  delay(2000);
}


#define countof(a) (sizeof(a) / sizeof(a[0]))

void getDateTimeString(const RtcDateTime& dt) {
  int hour = dt.Hour();
  boolean pm = hour >= 12;
  if (hour > 12) {
    hour -= 12;
  }
  snprintf_P(dateTimeString,
             countof(dateTimeString),
             // PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             PSTR("%02u:%02u:%02u %s"),
             //dt.Month(),
             //dt.Day(),
             //dt.Year(),
             hour,
             dt.Minute(),
             dt.Second(),
             pm ? "PM" : "AM");
}
