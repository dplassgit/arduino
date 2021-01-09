#include <ThreeWire.h>
#include <RtcDS1302.h>

#include "LEDDisplayDriver.h"

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html

#define NUM_DIGITS 16
const byte dataPin = A4;
const byte clockPin = A5;

LEDDisplayDriver display(dataPin, clockPin, true, NUM_DIGITS);

#define BUFFER_SIZE 100
char dateTimeString[BUFFER_SIZE];
char temp[BUFFER_SIZE];


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

  Serial.println("Hello serial Wordclock");
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
    Serial.println("RTC is older than compile time! (Updating DateTime)");
    Rtc.SetDateTime(compiled);
    display.showText("RTC is older than compile time! (Updating DateTime)");
    delay(1000);
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
    display.showTextScroll("CLOCK BAD!?");
  } else {
    populateTimeString(now);
    display.showTextScroll(dateTimeString);
    delay(2000);
    populateTimeString2(now);
    for (int i = 0; i < 5; ++i) {
      display.showTextScroll(dateTimeString);
    }
  }
}


#define countof(a) (sizeof(a) / sizeof(a[0]))

void populateTimeString(const RtcDateTime& dt) {
  int hour = dt.Hour();
  boolean ampm = hour >= 12;
  if (hour > 12) {
    hour -= 12;
  }
  if (hour == 0) {
    hour = 12;
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
  if (minute >= 33) {
    hour++;
  }
  const char *separator = " ";
  const char *hourStr = getHour(hour);
  const char *minuteStr = getMinute(minute);

  if (minute >= 58 || minute <= 3) {
    if (hour == 0 || hour == 12 || hour == 24) {
      // don't show "noon o'clock" or "midnight o'clock"
      minuteStr = "";
      separator = "";
    } else {
      // swap; instead of "O'clock Ten" show "Ten O'clock".
      const char *temp; temp = hourStr; hourStr = minuteStr; minuteStr = temp;
    }
  }
  snprintf_P(temp,
             countof(temp),
             PSTR("%s%s%s"),
             minuteStr,
             separator,
             hourStr
            );
  // Figure out how many spaces to add to the beginning and end of the string;
  // the string can be up to BUFFER_SIZE long, and have at most NUM_DIGITS spaces
  // at the beginning and end.
  int numSpace = 0;
  int len = strlen(temp);
  // I should math the shit out of this
  while (len < BUFFER_SIZE && numSpace < NUM_DIGITS) {
    numSpace++;
    len += 2;
  }
  // Serial.print("oldlen: "); Serial.println(strlen(temp));
  // Serial.print("newlen: "); Serial.println(len);
  // Serial.print("numspace: "); Serial.println(numSpace);
  // Clear the final destination; copy the temporary string into the right
  // spot, then fix the end-of-string marker.
  memset(dateTimeString, ' ', BUFFER_SIZE);
  strcpy(&dateTimeString[numSpace], temp);
  dateTimeString[numSpace + strlen(temp)] = ' ';
  dateTimeString[len] = 0;
  Serial.print("old string: "); Serial.println(temp);
  Serial.print("new string: ."); Serial.print(dateTimeString); Serial.println(".");
  // .                Twenty to Four                .
  // .                Quarter to Four                .
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
    case 4: case 5: case 6: case 7: return "Five minutes Past";
    case 8: case 9: case 10: case 11: case 12: case 13: return "Ten Past";
    case 14: case 15: case 16: return "A Quarter Past";
    case 17: case 18: case 19: case 20: case 21: case 22: case 23: return "Twenty minutes Past";
    case 24: case 25: case 26: case 27: return "Twenty five Past";
    case 28: case 29: case 30: case 31: case 32: return "Half Past";
    case 33: case 34: case 35: case 36: return "Twenty five to";
    case 37: case 38: case 39: case 40: case 41: case 42: case 43: return "Twenty minutes to";
    case 44: case 45: case 46: return "A Quarter to";
    case 47: case 48: case 49: case 50: case 51: case 52: return "Ten to";
    case 53: case 54: case 55: case 56: case 57: return "Five minutes to";
  }
}
