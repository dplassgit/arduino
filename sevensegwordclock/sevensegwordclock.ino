// Use "NodeMCU 0.9 (ESP-12)" or "WEMOS mini D1 (clone)" to program (NOT Generic8266)
#include "config.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <time.h>
#include <coredecls.h>                  //   required for settimeofday_cb()

// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html
#include "LEDDisplayDriver.h"

const char* ssid = SECRET_SSID;
const char* pass = SECRET_PWD;

// Timing parameters
time_t present_timestamp;

int progressStatus = 0;

// Display
#define NUM_DIGITS 8
const byte dataPin = D4;
const byte loadPin = D2;
const byte clockPin = D3;

LEDDisplayDriver display(dataPin, clockPin, loadPin, true, NUM_DIGITS);

// Printing to the display.
#define BUFFER_SIZE 200
char dateTimeString[BUFFER_SIZE];


void setup() {
  // make sure the ESP8266 WiFi functions are enabled
  WiFi.mode(WIFI_STA);                  // use only the WiFi 'station' mode

  Serial.begin(115200);

  Serial.println("Hello 7seg Clock serial d1");
  display.showTextScroll("Connecting D1....  ");

  WiFi.begin(ssid, pass);               // send credentials
  Serial.println("Connecting");
  int dot = 0;
  // wait for connection
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(500);
    display.showText(".", dot++, 1);
  }
  Serial.println("Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  display.showTextScroll("Connected!!!   ");
  delay(1000);

  // implement NTP update of timekeeping (with automatic hourly updates)
  configTime(0, 0, "0.pool.ntp.org");

  // info to convert UNIX time to local time (including automatic DST update)
  setenv("TZ", "EST+5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00", 1);

  // register a callback (execute whenever an NTP update has occurred)
  settimeofday_cb(time_is_set);

  // All sorts of OTA (over-the-air) updates.
  // Code mostly copied from https://randomnerdtutorials.com/esp8266-ota-updates-with-arduino-ide-over-the-air/
  ArduinoOTA.setPort(OTA_PORT);
  ArduinoOTA.setHostname("sevensegwordd1");
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onStart([]() {
    progressStatus = 0;
    display.showText("Start");
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    digitalWrite(LED_BUILTIN, HIGH);
    display.showText("End");
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if (progressStatus == 0) {
      display.showText("Updating");
    }
    digitalWrite(LED_BUILTIN, progressStatus % 2);
    progressStatus++;
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      display.showTextScroll("Auth failed");
      Serial.println("Auth Failed");
    }  else if (error == OTA_BEGIN_ERROR) {
      display.showTextScroll("Begin failed");
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      display.showTextScroll("Connect failed");
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      display.showTextScroll("Receive failed");
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      display.showTextScroll("End failed");
      Serial.println("End Failed");
    }
    delay(2000);
  });
  ArduinoOTA.begin();
}

void loop() {
  //  loop2();
  loopOrig();
  ArduinoOTA.handle();
}

void loopOrig() {
  present_timestamp = time(nullptr);

  // convert the system (UNIX) time to a local date and time in a configurable format
  struct tm *now = localtime(&present_timestamp);      // break down the timestamp

  populateTimeStringAsNumbers(now);
  display.showTextScroll(dateTimeString);
  Serial.println(dateTimeString);

  delay(4000);
  populateTimeStringAsWords(now);
  display.showTextScroll(dateTimeString);
  Serial.println(dateTimeString);

  populateDateAsWords(now);
  display.showTextScroll(dateTimeString);
  Serial.println(dateTimeString);
}

void loop2() {
  for (int hr = 1; hr < 24; ++hr) {
    for (int min = 2; min < 60; min += 7) {
      struct tm testNow;
      testNow.tm_hour = hr;
      testNow.tm_min = min;
      populateTimeStringAsNumbers(&testNow);
      Serial.println(dateTimeString);
      display.showTextScroll(dateTimeString);

      populateTimeStringAsWords(&testNow);
      Serial.println(dateTimeString);
      //  display.showTextScroll(dateTimeString);
    }
  }
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
  if (minute >= 43) {
    hour++;
  }
  // Can we replace this with a literal in the format string?
  const char *separator = " ";
  const char *hourStr = getHour(hour);
  const char *minuteStr = getMinute(minute);
  char temp[BUFFER_SIZE];

  snprintf_P(temp,
             countof(temp),
             PSTR("%s%s%s"),
             hourStr
             separator,
             minuteStr,
            );
  // Figure out how many spaces to add to the beginning and end of the string;
  // the string can be up to BUFFER_SIZE long, and have at most NUM_DIGITS spaces
  // at the beginning and end.
  int numSpace = 0;
  int len = strlen(temp);
  // Gemini mathed this for me:
  if (len < BUFFER_SIZE) {
    // Calculate how many +2 steps fit in the remaining buffer
    numSpace = (BUFFER_SIZE - len + 1) / 2;
    
    // Cap it by the maximum allowed digits
    if (numSpace > NUM_DIGITS) {
      numSpace = NUM_DIGITS;
    }
  }

  // Update the final length
  len += (numSpace * 2);

  // Clear the final destination; copy the temporary string into the right
  // spot, then fix the end-of-string marker.
  memset(dateTimeString, ' ', BUFFER_SIZE);
  strcpy(&dateTimeString[numSpace], temp);
  dateTimeString[numSpace + strlen(temp)] = ' ';
  dateTimeString[len] = 0;
}

// Sun Jan 18
void populateDateAsWords(struct tm * dt) {
  const char *separator = " ";

  const char *dayStr = getDay(dt->tm_wday);
  const char *monthStr = getMonth(dt->tm_mon);

  char temp[BUFFER_SIZE];
  snprintf_P(temp,
             countof(temp),
             PSTR("%s%s%s%s%u"),
             dayStr,
             separator,
             monthStr,
             separator,
             dt->tm_mday
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

const char *getDay(int day) {
  switch (day) {
    case 0: return "Sun";
    case 1: return "Mon";
    case 2: return "Tue";
    case 3: return "Wed";
    case 4: return "Thu";
    case 5: return "Fri";
    case 6: return "Sat";
    default: return "";
  }
}

const char *getMonth(int mon) {
  switch (mon) {
    case 0: return "Jan";
    case 1: return "Feb";
    case 2: return "Mar";
    case 3: return "Apr";
    case 4: return "May";
    case 5: return "Jun";
    case 6: return "Jul";
    case 7: return "Aug";
    case 8: return "Sep";
    case 9: return "Oct";
    case 10: return "Nov";
    case 11: return "Dec";
    default: return "";
  }
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
    case 12: case 0: case 24: return "12";
    default: return "";
  }
}

const char *getMinute(int minute) {
  switch (minute) {
    case 0: return "O'clocK";
    case 1: return "oh-one";
    case 2: return "oh-one-ish";
    case 3: return "oh-three";
    case 4: return "oh-four";
    case 5: return "oh-five";
    case 6: return "oh-five-ish";
    case 7: return "oh-seven";
    case 8: return "oh-eight";
    case 9: return "oh-nine";
    case 10: return "ten";
    case 11: return "eleven";
    case 12: return "eleven-ish";
    case 13: return "thirteen";
    case 14: return "fourteen";
    case 15: return "fifteen";
    case 16: return "fifteen-ish";
    case 17: return "seventeen";
    case 18: return "eighteen";
    case 19: return "nineteen";
    case 20: return "20";
    case 21: return "21";
    case 22: return "22";
    case 23: return "23";
    case 24: return "24";
    case 25: return "25";
    case 26: return "26";
    case 27: return "27";
    case 28: return "28";
    case 29: return "29";
    case 30: return "thirty";
    case 31: return "thirty one";
    case 32: return "thirty one-ish";
    case 33: return "thirty three";
    case 34: return "thirty four";
    case 35: return "thirty five";
    case 36: return "thirty five-ish";
    case 37: return "thirty seven";
    case 38: return "thirty eight";
    case 39: return "thirty nine";
    case 40: return "forty";
    case 41: return "forty one";
    case 42: return "forty two";
    case 43: return "forty three";
    case 44: return "forty four";
    case 45: return "forty five";
    case 46: return "forth five-ish";
    case 47: return "forty seven";
    case 48: return "forty eight";
    case 49: return "forty nine";
    case 50: return "fifty";
    case 51: return "fifty one";
    case 52: return "fifty one-ish";
    case 53: return "fifty three";
    case 54: return "fifty four";
    case 55: return "fifty five";
    case 56: return "fifty five-ish";
    case 57: return "fifty seven";
    case 58: return "fifty eight";
    case 59: return "fifty nine";
    default: return ""; 
  }
}