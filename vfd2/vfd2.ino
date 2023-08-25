#include <SPI_VFD.h>

//uint8_t data, uint8_t clock, uint8_t strobe,
SPI_VFD vfd(3, 6, 4);
byte heart[8] = {
  B00000,
  B01010,
  B11111,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000
};

const int numRows = 2;
const int numCols = 40;

void setup() {

  // create a new character
  vfd.createChar(0, heart);

  // set up the VFD's number of columns and rows:
  vfd.begin(numCols, numRows);

  // Print a message to the VFD.
  vfd.setCursor(0, 0);
  vfd.print("Hello World ");
  vfd.write(0);
  vfd.write(0);
  delay(5000);
  vfd.setCursor(0, 1);
  vfd.print("Please wait 5 seconds...");
  vfd.blink();
  delay(5000);
  vfd.noBlink();
  vfd.clear();
}

void loop() {
  all();
}

void all() {
  letters();
  delay(5000);
  showtime();
  delay(5000);
  autoscroll();
  delay(5000);
  vfd.clear();
}

void letters() {
  vfd.cursor();

  for (int thisLetter = 0; thisLetter <= 255; thisLetter++) {
    int thisCol = thisLetter % numCols;
    int thisRow = thisLetter / numCols;
    thisRow %= 2;
    vfd.setCursor(thisCol, thisRow);
    // print the letter:
    vfd.write(thisLetter);
    if ((thisLetter % 80) == 79) {
      vfd.blink();
      delay(5000);
      vfd.noBlink();
    } else {
      delay(200);
    }
  }
  vfd.noCursor();
}


void showtime() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  vfd.setCursor(0, 1);
  // print the number of seconds since reset:
  vfd.print(millis() / 1000);
}

void cursor() {
  // Turn off the cursor:
  vfd.noCursor();
  delay(500);
  // Turn on the cursor:
  vfd.cursor();
  delay(500);
}

void autoscroll() {
  // set the cursor to (0,0):
  vfd.setCursor(0, 0);
  // print from 0 to 9:
  for (int thisChar = 0; thisChar < 10; thisChar++) {
    vfd.print(thisChar);
    delay(500);
  }

  // set the cursor to (20,1):
  vfd.setCursor(40, 1);
  // set the display to automatically scroll:
  vfd.autoscroll();
  // print from 0 to 9:
  for (int thisChar = 0; thisChar < 10; thisChar++) {
    vfd.print(thisChar);
    delay(500);
  }
  // turn off automatic scrolling
  vfd.noAutoscroll();
}
