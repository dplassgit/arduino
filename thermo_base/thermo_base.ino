
#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a>b)?a:b)

//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "LEDDisplayDriver.h"

#define NUM_DIGITS 16
/* For Uno:
  const byte dataPin = A0;
  const byte clockPin = A1;
  const byte loadPin = A2;
  const byte intPin = 2;
*/
/* For ESP8266MOD */
const byte dataPin = D1;
const byte clockPin = D2;
const byte loadPin = D3;
const byte intPin = D4;

LEDDisplayDriver display(dataPin, clockPin, loadPin, true, NUM_DIGITS);

//create an RF24 object
/* Uno:
RF24 radio(9, 8);  // CE, CSN
*/
// ESP8266
RF24 radio(D0, D8);  // CE, CSN

// address through which two modules communicate.
const byte address[6] = "flori";

// 0=basement, 1=aaron, 2=garage
char text[3][32] = {0};
long when[3];

void setup() {
  Serial.begin(115200);
  Serial.println("Hello thermo base");

  radio.begin();
  radio.setChannel(0x66);
  radio.setPALevel(RF24_PA_MAX);
  // set the address
  radio.openReadingPipe(0, address);

  //Set module as receiver
  radio.startListening();

  display.begin();
  display.setBrightness(1);
  display.showTextScroll("LISTENING");
  strcpy(text[0], PSTR("no data"));
  strcpy(text[1], PSTR("no data"));
  strcpy(text[2], PSTR("no data"));
  attachInterrupt(digitalPinToInterrupt(intPin), intHandler, FALLING);
}

ICACHE_RAM_ATTR void intHandler() {
  // Read the data if available in buffer
  if (radio.available()) {
    int slot = 0;
    char temp[32];
    radio.read(&temp, sizeof(temp));
    Serial.print("Data received: ");
    Serial.println(temp);
    if (temp[1] == 'b') {
      slot = 0;
    } else if (temp[1] == 'A') {
      slot = 1;
    } else if (temp[1] == 'G' || temp[1] == 'g') {
      slot = 2;
    }
    char *dest = &(text[slot][0]);
    strcpy(dest, &temp[0]);
    when[slot] = millis();
  } else {
    display.showText("Not available");
    Serial.println("Not available");
  }
}

int source = 0;
void loop() {
  if (when[source] != 0) {
    display.showText(text[source], 0, 8);
    display.showNum(when[source], 8, 8);
  } else {
    display.showText("NO DATA", 0, 8);
    display.showNum(source, 8, 8);
  }
  source++;
  if (source == 3) source = 0;
  delay(4000);
}
