#include <ThreeWire.h>
#include <RtcDS1302.h>

#include "LEDDisplayDriver.h"

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html

#define numberOfDigits 16
const byte dataPin = A4;
const byte clockPin = A5;
const byte loadPin = A2;

LEDDisplayDriver display(dataPin, clockPin, true, numberOfDigits);
DISPLAY_INTR(display)

char dateTimeString[50];

ThreeWire myWire(9, 8, 10); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

void setup() {
  Serial.begin(9600);
  Serial.println("Wordclock compiled on");

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  populateTimeString(compiled);
  Serial.println(dateTimeString);

  display.showTextScroll("Hello Wordclock");
  delay(2000);

  Serial.println("hello serial Wordclock");
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
  populateTimeString(now);
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
  RtcDateTime now = Rtc.GetDateTime();
  if (!now.IsValid()) {
    // Common Causes:
    //    1) the battery on the device is low or even missing and the power line was disconnected
    Serial.println("RTC lost confidence in the DateTime!");
    display.showText("CLOCK BAD!?");
  } else {
    populateTimeString(now);
    display.showText(dateTimeString);
    delay(2000);
    populateTimeString2(now);
    display.showTextScroll(dateTimeString);
  }
  delay(10000);
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

void populateTimeString2(const RtcDateTime& dt) {
  int hour = dt.Hour();
  boolean ampm = hour >= 12;
  if (hour > 12) {
    hour -= 12;
  }
  int minute = dt.Minute();
  const char *separator = " ";
  const char *hourStr = getHour(hour);
  const char *minuteStr = getMinute(minute);
  if (minute >= 33) {
    hourStr = getHour(hour + 1);
  }

  if (minute >= 58 || minute <= 3) {
    if (hour == 0 || hour == 12) {
      // don't show "noon o'clock"
      minuteStr = "";
      separator = "";
    } else {
      // swap; instead of "O'clock Ten" it's "Ten O'clock".
      const char *temp; temp = hourStr; hourStr = minuteStr; minuteStr = temp;
    }
  }
  snprintf_P(dateTimeString,
             countof(dateTimeString),
             PSTR("%s%s%s"),
             minuteStr,
             separator,
             hourStr
            );
}

const char *getHour(int hour) {
  switch (hour) {
    case 1: case 13: return "One";
    case 2: case 14: return "Two";
    case 3: case 15: return "Three";
    case 4: case 16: return "Four";
    case 5: case 17: return "Five";
    case 6: case 18: return "Six";
    case 7: case 19: return "Seven";
    case 8: case 20: return "Eight";
    case 9: case 21: return "Nine";
    case 10: case 22: return "Ten";
    case 11: case 23: return "Eleven";
    case 12: return "Noon";
    case 0: case 24: return "Midnight";
  }
}

const char *getMinute(int minute) {
  switch (minute) {
    case 58: case 59: case 0: case 1: case 2: case 3: return "O'clock";
    case 4: case 5: case 6: case 7: return "Five Past";
    case 8: case 9: case 10: case 11: case 12: case 13: return "Ten Past";
    case 14: case 15: case 16: return "Quarter Past";
    case 17: case 18: case 19: case 20: case 21: case 22: case 23: return "Twenty Past";
    case 24: case 25: case 26: case 27: return "Twenty five Past";
    case 28: case 29: case 30: case 31: case 32: return "Half Past";
    case 33: case 34: case 35: case 36: return "Twenty five to";
    case 37: case 38: case 39: case 40: case 41: case 42: case 43: return "Twenty to";
    case 44: case 45: case 46: return "A Quarter to";
    case 47: case 48: case 49: case 50: case 51: case 52: return "Ten to";
    case 53: case 54: case 55: case 56: case 57: return "Five to";
  }
}
