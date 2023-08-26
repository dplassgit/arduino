#include "config.h"
#include <ESP8266WiFi.h>
#include <time.h>
#include <coredecls.h>                  //   required for settimeofday_cb()

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html
#include "LEDDisplayDriver.h"

const char* ssid = SECRET_SSID;
const char* pass = SECRET_PWD;

// Timing parameters
time_t present_timestamp;

// Display
#define NUM_DIGITS 8
const byte dataPin = D1;
const byte loadPin = D2;
const byte clockPin = D3;

LEDDisplayDriver display(dataPin, clockPin, loadPin, true, NUM_DIGITS);

// Printing to the display.
#define BUFFER_SIZE 100
char dateTimeString[BUFFER_SIZE];


void setup() {
  // make sure the ESP8266 WiFi functions are enabled
  WiFi.mode(WIFI_STA);                  // use only the WiFi 'station' mode

  Serial.begin(115200);

  Serial.println("Hello 7seg Clock serial");
  delay(2000);
  display.showTextScroll("Connecting");

  WiFi.begin(ssid, pass);               // send credentials
  Serial.println("Connecting");
  int dot = 0;
  // wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.showText(".", dot++, 1);
  }
  Serial.println("Connected");
  // implement NTP update of timekeeping (with automatic hourly updates)
  configTime(0, 0, "0.pool.ntp.org");

  // info to convert UNIX time to local time (including automatic DST update)
  setenv("TZ", "EST+5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00", 1);

  // register a callback (execute whenever an NTP update has occurred)
  // DBP: Unclear if this is needed.
  settimeofday_cb(time_is_set);
}

void loop() {
  present_timestamp = time(nullptr);

  // convert the system (UNIX) time to a local date and time in a configurable format
  struct tm *now = localtime(&present_timestamp);      // break down the timestamp

  populateTimeStringAsNumbers(now);
  display.showTextScroll(dateTimeString);
  delay(4000);
  populateTimeStringAsWords(now);
  display.showTextScroll(dateTimeString);
}


// callback routine - arrive here whenever a successful NTP update has occurred
void time_is_set() {
  struct tm *tmp ;                      // NOTE: structure tm is defined in time.h

  char UPDATE_TIME[50];                 // buffer for use by strftime()

  // display time when NTP update occurred
  time_t tnow = time(nullptr);          // get UNIX timestamp
  tmp = localtime(&tnow);               // convert to local time and break down
  strftime(UPDATE_TIME, sizeof(UPDATE_TIME), "%T", tmp);  // extract just the 'time' portion

  Serial.print("\n-------- NTP update at ");
  Serial.print(UPDATE_TIME);
  Serial.println(" --------");
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void populateTimeStringAsNumbers(struct tm * dt) {
  int hour = dt->tm_hour;
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
             dt->tm_min, ampm ? "P" : "A"
            );
}

void populateTimeStringAsWords(struct tm * dt) {
  int hour = dt->tm_hour;
  boolean ampm = hour >= 12;
  if (hour > 12) {
    hour -= 12;
  }
  int minute = dt->tm_min;
  if (minute >= 33) {
    hour++;
  }
  const char *separator = " ";
  const char *hourStr = getHour(hour);
  const char *minuteStr = getMinute(minute);
  char temp[BUFFER_SIZE];

  if (minute >= 58 || minute <= 2 || (minute >= 33 && minute <= 42)) {
    if (hour == 0 || hour == 12 || hour == 24) {
      // don't show "noon o'clock" or "midnight o'clock"
      minuteStr = "";
      separator = "";
    } else {
      // swap; instead of "O'clock Ten" show "Ten O'clock".
      const char *swaptemp; swaptemp = hourStr; hourStr = minuteStr; minuteStr = swaptemp;
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
  // Clear the final destination; copy the temporary string into the right
  // spot, then fix the end-of-string marker.
  memset(dateTimeString, ' ', BUFFER_SIZE);
  strcpy(&dateTimeString[numSpace], temp);
  dateTimeString[numSpace + strlen(temp)] = ' ';
  dateTimeString[len] = 0;
}

const char *getHour(int hour) {
  switch (hour) {
    case 1: case 13: return "One";
    case 2: case 14: return "2";
    case 3: case 15: return "Three";
    case 4: case 16: return "Four";
    case 5: case 17: return "Five";
    case 6: case 18: return "6";
    case 7: case 19: return "Seven";
    case 8: case 20: return "Eight";
    case 9: case 21: return "Nine";
    case 10: case 22: return "Ten";
    case 11: case 23: return "Eleven";
    case 12: return "Noon";
    case 0: case 24: return "12";
    default: return "";
  }
}

const char *getMinute(int minute) {
  switch (minute) {
    case 58: case 59: case 0:  case 1:  case 2: return "O'clocK";
    case 3:  case 4:  case 5:  case 6:  case 7: return "Five Past";
    case 8:  case 9:  case 10: case 11: case 12: return "Ten Past";
    case 13: case 14: case 15: case 16: case 17: return "a Quarter Past";
    case 18: case 19: case 20: case 21: case 22: return "20 Past";
    case 23: case 24: case 25: case 26: case 27: return "25 Past";
    case 28: case 29: case 30: case 31: case 32: return "Half Past";
    case 33: case 34: case 35: case 36: case 37: return "Thirty five";
    case 38: case 39: case 40: case 41: case 42: return "Forty";
    case 43: case 44: case 45: case 46: case 47: return "a Quarter to";
    case 48: case 49: case 50: case 51: case 52: return "Ten to";
    case 53: case 54: case 55: case 56: case 57: return "Five to";
    default: return "";
  }
}
