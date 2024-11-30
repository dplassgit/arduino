/*----------------------------------------------------------------------------------*
   A simple clock that reads from a DS1302 RTC and shows the time on 4 x 7-segment displays
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

#include <FlipDisc.h>   // https://github.com/marcinsaj/FlipDisc 
#include <RtcDS1302.h>

ThreeWire myWire(2, 3, 4); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

// Standard pin declaration for Arduino Uno and PSPS module
#define EN_PIN  10
#define CH_PIN  8
#define PL_PIN  9

// Note, MOSI (DataIn), Clk (SCK) are defaulted to 11 and 13, respectively.

RtcDateTime last;

void setup()
{
  Serial.begin(57600);

  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

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

  Flip.Matrix_7Seg(I, N, I, T);
  delay(500);
  Flip.Matrix_7Seg(R, T, C, CLR);
  delay(500);

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if (!Rtc.IsDateTimeValid())
  {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC lost confidence in the DateTime!");
    Flip.Matrix_7Seg(S, E, T, CLR);
    delay(1000);
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
    Flip.Matrix_7Seg(R, T, W, P);
    delay(1000);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
    Flip.Matrix_7Seg(S, T, R, T);
    delay(1000);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time! (Updating DateTime)");
    Rtc.SetDateTime(compiled);
    Flip.Matrix_7Seg(R, T, U, P);
    delay(1000);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  last = Rtc.GetDateTime();
  showTime(last);
}

void showTime(RtcDateTime now) {
  printDateTime(now);
  Serial.println();

  int hour = now.Hour();
  if (hour > 12) {
    hour -= 12;
  }
  int hr10 = hour / 10;
  int hr1 = hour % 10;
  int min10 = now.Minute() / 10;
  int min1 = now.Minute() % 10;
  Serial.print("Clock is: "); Serial.print(hr10); Serial.print(hr1); Serial.print(":"); Serial.print(min10); Serial.println(min1);

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
}

void loop()
{
  RtcDateTime now = Rtc.GetDateTime();
  if (now.Hour() != last.Hour() || now.Minute() != last.Minute()) {
    // Hour or minute is different; update the whole clock.
    showTime(now);
    last = now;
  } else if (now.Second() != last.Second()) {
    /* An example of calling the function to set disc no.19 of the first 7-Segment display */
    /* 0  1  2  3  4
      19           5
      18           6
      17 20 21 22  7
      16           8
      15           9
      14 13 12 11 10 */
    // Flip one disc in the leftmost column to indicate 10s of seconds
    for (int i = 0; i <= now.Second() / 10; i++) {
      Flip.Disc_7Seg(1, 14 + i, 1); // last argument can be 0 to turn off
    }
  }

  /* 7-Segment displays allow the display of numbers and symbols.
      Symbols can be displayed using their code name or number e.g. 37/DEG - "°" Degree symbol
      The full list of symbols can be found in the FlipDisc.h library repository https://github.com/marcinsaj/FlipDisc
      Code names for symbols:
      - 0-9
      - 1/VLR  - " |" - Vertical line - right
      - 8/ALL  - Set all discs
      - 10/CLR - Clear display
      - 11/A
      - 12/B
      - 13/C
      - 14/D
      - 15/E
      - 16/F
      - 17/G
      - 18/H
      - 19/I
      - 20/J
      - 21/K
      - 22/L
      - 23/M
      - 24/N
      - 25/O
      - 26/P
      - 27/Q
      - 28/R
      - 29/S
      - 30/T
      - 31/U
      - 32/V
      - 33/W
      - 34/X
      - 35/Y
      - 36/Z
      - 37/DEG - "°"  - Degree symbol
      - 37/PFH - "%"  - Percent first half symbol
      - 38/PSH - "%"  - Percent second half symbol
      - 39/HLU - "¯"  - Horizontal line - upper
      - 40/HLM - "-"  - Horizontal line - middle
      - 41/HLL - "_"  - Horizontal line - lower
      - 42/HLT - "="  - Horizontal line - upper & lower
      - 43/HLA - "≡"  - All three lines
      - 40/MIN - "-"  - Minus symbol
      - 44/VLL - "| " - Vertical line - left
      - 45/VLA - "||" - All Vertical lines */
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
  char datestring[26];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(datestring);
}
