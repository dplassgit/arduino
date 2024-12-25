/*----------------------------------------------------------------------------------*
   A simple clock that reads the time from the internet and shows the time on 4 x 7-segment displays
   Example connection diagram: https://bit.ly/4x7SEG-3x1DOT

   The MIT License
   David Plass 30 Nov 2024

   Based on example code at https://github.com/marcinsaj/FlipDisc
   by Marcin Saj 15 Jan 2023

   A dedicated controller or any Arduino board with a power module is required
   to operate the display:
   1. Dedicated controller - https://bit.ly/AC1-FD
   2. Or any Arduino board + Pulse Shaper Power Supply - https://bit.ly/PSPS-FD
  ----------------------------------------------------------------------------------*/

/* The library <FlipDisc.h> uses SPI to control flip-disc displays.
  The user must remember to connect the display inputs marked:
  - DIN - data in - to the MOSI (SPI) output of the microcontroller,
  - CLK - clock - input of the display to the SCK (SPI).
  The displays are connected in series through the built-in connectors,
  only the first display from the left is connected to the Arduino or a dedicated controller.

  It is very important to connect and declare EN, CH, PL pins.
  The declaration of DIN (MOSI) and CLK (SCK) is not necessary,
  because the SPI.h library handles the SPI hardware pins. */

// Use "NodeMCU 0.9 (ESP-12) to program (NOT Generic8266)
#include <ESP8266WiFi.h>
#include <time.h>
#include <coredecls.h>                  //   required for settimeofday_cb()
#include <FlipDisc.h>   // https://github.com/marcinsaj/FlipDisc 

#include "config.h"
const char* ssid = SECRET_SSID;
const char* pass = SECRET_PWD;

// Pin definitions for the 7-segment clock
#define EN_PIN  D1
#define CH_PIN  D2
#define PL_PIN  D3

// Note, MOSI (DataIn), Clk (SCK) are defaulted to D7 and D5, respectively on the 8266 I have

void setup() {
  Serial.begin(115200);
  Serial.println("Hello 4x7seg-flip-disc-clock-wifi");

  // We're going to blink the LED also, so we can tell that it's alive.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (yes the logic is flopped)

  /* Flip.Pin(); it is the most important function and first to call before everything else.
    The function is used to declare pin functions. Before starting the device, double check
    that the declarations and connection are correct. If the connection of the control outputs
    is incorrect, the display may be physically damaged. */
  Flip.Pin(EN_PIN, CH_PIN, PL_PIN);

  /* Flip.Init(display1, display2, ... display8); it is the second most important function.
    Initialization function for a series of displays. The function has 1 default argument and 7 optional arguments.
    The function also prepares SPI. Correct initialization requires code names of the serially
    connected displays:
    - D7SEG - 7-segment display
    - D2X1 - 2x1 display
    - D3X1 - 3x1 display
    - D1X3 - 1x3 display
    - D1X7 - 1x7 display */
  Flip.Init(D7SEG, D7SEG, D7SEG, D7SEG);

  /* The function is used to set the delay effect between flip discs.
    The default value without calling the function is 0. Can be called multiple times
    anywhere in the code. Recommended delay range: 0 - 100ms, max 255ms */
  Flip.Delay(10);

  /* The function is used to test all declared displays - turn on and off all displays */
  Flip.Test();
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED off (yes the logic is flopped)

  // send credentials
  WiFi.begin(ssid, pass);

  Serial.println("Connecting");
  Flip.Matrix_7Seg(C, O, N, N);
  int dot = 0;
  // wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Flip.Disc_7Seg(1, dot % 23, dot % 2); // last argument can be 0 to turn off
    dot++;
    Serial.print(".");
  }
  Serial.println("Connected");
  Flip.Matrix_7Seg(G, O, O, D);
  delay(1000);

  // implement NTP update of timekeeping (with automatic hourly updates)
  configTime(0, 0, "0.pool.ntp.org");

  // info to convert UNIX time to local time (including automatic DST update)
  setenv("TZ", "EST+5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00", 1);

  time_t last = time(nullptr);

  // convert the system (UNIX) time to a local date and time in a configurable format
  struct tm* last_tm = localtime(&last);      // break down the timestamp
  showTime(last_tm->tm_hour, last_tm->tm_min, last_tm->tm_sec);

  // register a callback (execute whenever an NTP update has occurred)
  settimeofday_cb(timeUpdated);

  digitalWrite(LED_BUILTIN, LOW);
}

int last_hour;
int last_min;
int last_sec;

// callback routine - arrive here whenever a successful NTP update has occurred
void timeUpdated() {
  time_t last = time(nullptr);                 // get UNIX timestamp
  struct tm *last_tm = localtime(&last);    // convert to local time and break down
  showTime(last_tm->tm_hour, last_tm->tm_min, last_tm->tm_sec);

  char UPDATE_TIME[50];                 // buffer for use by strftime()
  strftime(UPDATE_TIME, sizeof(UPDATE_TIME), "%T", last_tm);  // extract just the 'time' portion

  Serial.print("-------- NTP update at ");
  Serial.print(UPDATE_TIME);
  Serial.println(" --------");
}

// Show the time; update the globals with the "last time shown"
void showTime(int hour, int minute, int sec) {
  last_hour = hour;
  last_min = minute;
  last_sec = sec;
  if (hour > 12) {
    hour -= 12;
  }
  int hr10 = hour / 10;
  int hr1 = hour % 10;
  int min10 = minute / 10;
  int min1 = minute % 10;
  Serial.print("Clock is: ");
  Serial.print(hr10);
  Serial.print(hr1);
  Serial.print(":");
  Serial.print(min10);
  Serial.println(min1);

  // Clear first digit if zero.
  if (hr10 == 0) {
    hr10 = CLR;
  }

  // Set each digit of the clock.
  /* Function to control up to eight 7-segment displays.
    The first argument is the default and the others are optional.
    This function allows you to display numbers and symbols
    Flip.Matrix_7Seg(data1,data2,data3,data4,data5,data6,data7,data8); */
  Flip.Matrix_7Seg(hr10, hr1, min10, min1);
  Serial.println("showTime End");
}

// Main loop: get the current time; if it's different than last shown time, show it.
void loop() {
  time_t now_t  = time(nullptr);
  // convert the system (UNIX) time to a local date and time in a configurable format
  struct tm *now = localtime(&now_t);

  /* An example of calling the function to set disc no.19 of the first 7-Segment display */
  /* 0  1  2  3  4
    19           5
    18           6
    17 20 21 22  7
    16           8
    15           9
    14 13 12 11 10 */
  int sec = now->tm_sec;
  if (sec != last_sec) {
    // Flip one disc in bottom row to indicate 10s of seconds.
    for (int i = 0; i < sec / 10; i++) {
      Flip.Disc_7Seg(1, 10 + i, 1); // last argument can be 0 to turn off
    }
    if ((sec % 2) == 0) {
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
    }
    Serial.print("updated sec. was: ");
    Serial.print(last_sec);
    Serial.print(", now: " );
    Serial.println(sec);
  }
  last_sec = sec;

  if (now->tm_hour != last_hour || now->tm_min != last_min) {
    // Hour or minute is different; update the whole clock.
    Serial.print("last_tm: ");
    Serial.print(last_hour);
    Serial.print(":");
    Serial.println(last_min);

    Serial.print("now: ");
    Serial.print(now->tm_hour);
    Serial.print(":");
    Serial.print(now->tm_min);
    Serial.print(":");
    Serial.println(now->tm_sec);

    // this sets last_hour, last_min and last_sec
    showTime(now->tm_hour, now->tm_min, now->tm_sec);
  }
}
