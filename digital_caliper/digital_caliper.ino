/**
   Digital caliper -> VFD
*/

#include <SPI_VFD.h>

//uint8_t data, uint8_t clock, uint8_t strobe,
SPI_VFD vfd(3, 6, 4);

const int numRows = 2;
const int numCols = 40;

#define DC_CLK 11
#define DC_DATA 10

void setup() {
  Serial.begin(115200);

  // set up the VFD's number of columns and rows:
  vfd.begin(numCols, numRows);

  // Print a message to the VFD.
  vfd.setCursor(0, 0);
  vfd.print("Hello Digital Caliper");
  vfd.setCursor(0, 1);
  vfd.print("Please wait 5 seconds...");
  vfd.blink();
  delay(5000);
  vfd.noBlink();
  vfd.clear();
}

int bit_array[25];        // For storing the data bit. bit_array[0] = data bit 1 (LSB), bit_array[23] = data bit 24 (MSB).
float last = -100000.0;

void loop() {
  long value = 0, power = 0;
  int inches = 0;
  while (digitalRead(DC_CLK) == LOW) {}
  // it went high, now wait for it to fall.
  for (int i = 1; i <= 24; ++i) {
    while (digitalRead(DC_CLK) == HIGH) {}
    // it fell, read the data.
    int data = 1 - digitalRead(DC_DATA);
    bit_array[i] = data;
    if (i > 1 && i < 21) {
      // shift data "power" bits to the left, add to value
      if (data) {
        value = value + (1L << power);
      }
      power++;
    }
    if (i == 21 && data == 1) {
      // sign
      value = value * -1;
    }
    if (i == 24) {
      inches = data;
    }
    // wait for clock to go low again
    while (digitalRead(DC_CLK) == LOW) {}
  }
  for (int i = 1; i <= 24; ++i) {
    //    Serial.print(i); Serial.print(":"); Serial.println(bit_array[i]);
  }
  float actual = value;
  if (inches == 1) {
    actual = actual / 1000.0;
  } else {
    actual = actual / 50.0;
  }
  if (actual != last) {
    last = actual;
    vfd.clear();
    vfd.setCursor(0, 0);
    vfd.print(actual, 2 + inches);
    vfd.setCursor(9, 0);
    if (inches == 1) {
      vfd.print("in");
    } else {
      vfd.print("mm");
    }
  }
  delay(100);
}
