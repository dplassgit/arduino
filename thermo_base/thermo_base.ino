#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266mDNS.h>

#include <RF24.h>

#define min(a,b) ((a<b)?a:b)
#define max(a,b) ((a>b)?a:b)

#include "LEDDisplayDriver.h"
#include "config.h"
#include "data.h"

#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServerSecure server(443);

// The certificate is stored in PMEM
static const uint8_t x509[] PROGMEM = {
  0x30, 0x82, 0x01, 0x3d, 0x30, 0x81, 0xe8, 0x02, 0x09, 0x00, 0xfe, 0x56,
  0x46, 0xf2, 0x78, 0xc6, 0x51, 0x17, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86,
  0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x30, 0x26, 0x31,
  0x10, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x07, 0x45, 0x53,
  0x50, 0x38, 0x32, 0x36, 0x36, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55,
  0x04, 0x03, 0x0c, 0x09, 0x31, 0x32, 0x37, 0x2e, 0x30, 0x2e, 0x30, 0x2e,
  0x31, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x37, 0x30, 0x33, 0x31, 0x38, 0x31,
  0x34, 0x34, 0x39, 0x31, 0x38, 0x5a, 0x17, 0x0d, 0x33, 0x30, 0x31, 0x31,
  0x32, 0x35, 0x31, 0x34, 0x34, 0x39, 0x31, 0x38, 0x5a, 0x30, 0x26, 0x31,
  0x10, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x07, 0x45, 0x53,
  0x50, 0x38, 0x32, 0x36, 0x36, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55,
  0x04, 0x03, 0x0c, 0x09, 0x31, 0x32, 0x37, 0x2e, 0x30, 0x2e, 0x30, 0x2e,
  0x31, 0x30, 0x5c, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7,
  0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x4b, 0x00, 0x30, 0x48, 0x02,
  0x41, 0x00, 0xc6, 0x72, 0x6c, 0x12, 0xe1, 0x20, 0x4d, 0x10, 0x0c, 0xf7,
  0x3a, 0x2a, 0x5a, 0x49, 0xe2, 0x2d, 0xc9, 0x7a, 0x63, 0x1d, 0xef, 0xc6,
  0xbb, 0xa3, 0xd6, 0x6f, 0x59, 0xcb, 0xd5, 0xf6, 0xbe, 0x34, 0x83, 0x33,
  0x50, 0x80, 0xec, 0x49, 0x63, 0xbf, 0xee, 0x59, 0x94, 0x67, 0x8b, 0x8d,
  0x81, 0x85, 0x23, 0x24, 0x06, 0x52, 0x76, 0x55, 0x9d, 0x18, 0x09, 0xb3,
  0x3c, 0x10, 0x40, 0x05, 0x01, 0xf3, 0x02, 0x03, 0x01, 0x00, 0x01, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
  0x05, 0x00, 0x03, 0x41, 0x00, 0x69, 0xdc, 0x6c, 0x9b, 0xa7, 0x62, 0x57,
  0x7e, 0x03, 0x01, 0x45, 0xad, 0x9a, 0x83, 0x90, 0x3a, 0xe7, 0xdf, 0xe8,
  0x8f, 0x46, 0x00, 0xd3, 0x5f, 0x2b, 0x0a, 0xde, 0x92, 0x1b, 0xc5, 0x04,
  0xc5, 0xc0, 0x76, 0xf4, 0xf6, 0x08, 0x36, 0x97, 0x27, 0x82, 0xf1, 0x60,
  0x76, 0xc2, 0xcd, 0x67, 0x6c, 0x4b, 0x6c, 0xca, 0xfd, 0x97, 0xfd, 0x33,
  0x9e, 0x12, 0x67, 0x6b, 0x98, 0x7e, 0xd5, 0x80, 0x8f
};

// And so is the key.  These could also be in DRAM
static const uint8_t rsakey[] PROGMEM = {
  0x30, 0x82, 0x01, 0x3a, 0x02, 0x01, 0x00, 0x02, 0x41, 0x00, 0xc6, 0x72,
  0x6c, 0x12, 0xe1, 0x20, 0x4d, 0x10, 0x0c, 0xf7, 0x3a, 0x2a, 0x5a, 0x49,
  0xe2, 0x2d, 0xc9, 0x7a, 0x63, 0x1d, 0xef, 0xc6, 0xbb, 0xa3, 0xd6, 0x6f,
  0x59, 0xcb, 0xd5, 0xf6, 0xbe, 0x34, 0x83, 0x33, 0x50, 0x80, 0xec, 0x49,
  0x63, 0xbf, 0xee, 0x59, 0x94, 0x67, 0x8b, 0x8d, 0x81, 0x85, 0x23, 0x24,
  0x06, 0x52, 0x76, 0x55, 0x9d, 0x18, 0x09, 0xb3, 0x3c, 0x10, 0x40, 0x05,
  0x01, 0xf3, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02, 0x40, 0x35, 0x0b, 0x74,
  0xd3, 0xff, 0x15, 0x51, 0x44, 0x0f, 0x13, 0x2e, 0x9b, 0x0f, 0x93, 0x5c,
  0x3f, 0xfc, 0xf1, 0x17, 0xf9, 0x72, 0x94, 0x5e, 0xa7, 0xc6, 0xb3, 0xf0,
  0xfe, 0xc9, 0x6c, 0xb1, 0x1e, 0x83, 0xb3, 0xc6, 0x45, 0x3a, 0x25, 0x60,
  0x7c, 0x3d, 0x92, 0x7d, 0x53, 0xec, 0x49, 0x8d, 0xb5, 0x45, 0x10, 0x99,
  0x9b, 0xc6, 0x22, 0x3a, 0x68, 0xc7, 0x13, 0x4e, 0xb6, 0x04, 0x61, 0x21,
  0x01, 0x02, 0x21, 0x00, 0xea, 0x8c, 0x21, 0xd4, 0x7f, 0x3f, 0xb6, 0x91,
  0xfa, 0xf8, 0xb9, 0x2d, 0xcb, 0x36, 0x36, 0x02, 0x5f, 0xf0, 0x0c, 0x6e,
  0x87, 0xaa, 0x5c, 0x14, 0xf6, 0x56, 0x8e, 0x12, 0x92, 0x25, 0xde, 0xb3,
  0x02, 0x21, 0x00, 0xd8, 0x99, 0x01, 0xf1, 0x04, 0x0b, 0x98, 0xa3, 0x71,
  0x56, 0x1d, 0xea, 0x6f, 0x45, 0xd1, 0x36, 0x70, 0x76, 0x8b, 0xab, 0x69,
  0x30, 0x58, 0x9c, 0xe0, 0x45, 0x97, 0xe7, 0xb6, 0xb5, 0xef, 0xc1, 0x02,
  0x21, 0x00, 0xa2, 0x01, 0x06, 0xc0, 0xf2, 0xdf, 0xbc, 0x28, 0x1a, 0xb4,
  0xbf, 0x9b, 0x5c, 0xd8, 0x65, 0xf7, 0xbf, 0xf2, 0x5b, 0x73, 0xe0, 0xeb,
  0x0f, 0xcd, 0x3e, 0xd5, 0x4c, 0x2e, 0x91, 0x99, 0xec, 0xb7, 0x02, 0x20,
  0x4b, 0x9d, 0x46, 0xd7, 0x3c, 0x01, 0x4c, 0x5d, 0x2a, 0xb0, 0xd4, 0xaa,
  0xc6, 0x03, 0xca, 0xa0, 0xc5, 0xac, 0x2c, 0xe0, 0x3f, 0x4d, 0x98, 0x71,
  0xd3, 0xbd, 0x97, 0xe5, 0x55, 0x9c, 0xb8, 0x41, 0x02, 0x20, 0x02, 0x42,
  0x9f, 0xd1, 0x06, 0x35, 0x3b, 0x42, 0xf5, 0x64, 0xaf, 0x6d, 0xbf, 0xcd,
  0x2c, 0x3a, 0xcd, 0x0a, 0x9a, 0x4d, 0x7c, 0xad, 0x29, 0xd6, 0x36, 0x57,
  0xd5, 0xdf, 0x34, 0xeb, 0x26, 0x03
};

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

// Create an RF24 object
/* Uno:
  RF24 radio(9, 8);  // CE, CSN
*/
// ESP8266
RF24 radio(D0, D8);  // CE, CSN

// address through which two modules communicate.
const byte address[6] = "flori";

#define NUM_REMOTES 7

// 0=basement, 1=aaron, 2=garage, 3=Office, 4=Florida, 5=here, 6=unknown
char text[NUM_REMOTES][32] = {0};
struct Data remoteData[NUM_REMOTES];
long when[NUM_REMOTES];
int missed[NUM_REMOTES];

void setup() {
  Serial.begin(115200);
  Serial.println("Hello thermo base");

  display.begin();
  display.setBrightness(1);
  display.showText("Connecting");
  int dot = 0;
  WiFi.begin(ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    display.showText(".", dot++, 1);
  }
  MDNS.begin("base");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  radio.begin();
  radio.setChannel(0x66);
  radio.setPALevel(RF24_PA_MAX);

  // set the address
  radio.openReadingPipe(1, address);

  //Set module as receiver
  radio.startListening();

  attachInterrupt(digitalPinToInterrupt(intPin), intHandler, FALLING);

  server.getServer().setServerKeyAndCert_P(rsakey, sizeof(rsakey), x509, sizeof(x509));
  server.on("/", handleRoot);
  server.begin();
  for (int i = 0; i < NUM_REMOTES; ++i) {
    missed[i] = 0;
    when[i] = 0;

  }
}

ICACHE_RAM_ATTR void intHandler() {
  // Read the data if available in buffer
  if (radio.available()) {
    struct Data data;
    radio.read(&data, 8);

    char temp[32];
    snprintf_P(temp,
               sizeof(temp),
               PSTR("%c%c: %d F %d V"),
               data.counter,
               data.id,
               (short)data.tempF,
               data.voltage
              );
    Serial.print("Data received equivalent to: ");
    Serial.println(temp);

    int slot = 0;
    char code = data.id;
    switch (code) {
      case 'b':
      case 'B':
        slot = 0;
        break;
      case 'a':
      case 'A':
        slot = 1;
        break;
      case 'g':
      case 'G':
        slot = 2;
        break;
      case 'o':
      case 'O':
        slot = 3;
        break;
      case 'f':
      case 'F':
        slot = 4;
        break;
      case 'h':
      case 'H':
        slot = 5;
        break;
      default:
        slot = 6;
        break;
    }
    if (when[slot] != 0) {
      // see if we missed one
      if ((data.counter != text[slot][0] && data.counter != text[slot][0] + 1) || (data.counter == 'A' && text[slot][0] != 'Z')) {
        missed[slot]++;
        Serial.print("Missed "); Serial.print(missed[slot]); Serial.print(" from "); Serial.println(temp[1]);
      }
    }
    char *dest = &(text[slot][0]);
    strcpy(dest, &temp[0]);

    memcpy(&(remoteData[slot]), &data, 8);
    when[slot] = millis();
  } else {
    display.showText("Not available");
    Serial.println("Not available");
  }
}


void handleRoot() {
  Serial.println("Handling /");
  long now = millis();
  char buffer[200];
  sprintf(buffer, "Basement: %s at %d\nAaron: %s at %d\nGarage: %s at %d\nOffice: %s at %d\nFlorida: %s at %d\nHere: %s at %d",
          text[0], (now - when[0]) / 1000,
          text[1], (now - when[1]) / 1000,
          text[2], (now - when[2]) / 1000,
          text[3], (now - when[3]) / 1000,
          text[4], (now - when[4]) / 1000,
          text[5], (now - when[5]) / 1000);
  Serial.println(buffer);
  server.send(200, "text/plain", buffer);
}

int source = 0;
long lastShown = 0;

void loop() {
  long now = millis();
  if (lastShown == 0 || now - lastShown > 4000) {
    // yes this fails for rollover
    lastShown = now;
    long secondsSince = (now - when[source]) / 1000;

    switch (source) {
      case 0:
        display.showText("BASEMENT", 0, 8);
        break;
      case 1:
        display.showText("AARON's", 0, 8);
        break;
      case 2:
        display.showText("GARAGE", 0, 8);
        break;
      case 3:
        display.showText("OFFICE", 0, 8);
        break;
      case 4:
        display.showText("FLORIDA", 0, 8);
        break;
      case 5:
        display.showText("HERE", 0, 8);
        break;
      default:
        display.showText("OTHER", 0, 8);
        break;
    }
    if (secondsSince < 9999 && when[source] != 0) {
      display.showNum(secondsSince, 8, 8);
      display.showNum1decimal(remoteData[source].tempF, 8, 3);
      display.showText("F", 11, 1);
    } else {
      display.showText("NO DATA", 8, 8);
    }

    source++;
    if (source == NUM_REMOTES) source = 0;

    MDNS.update();
  }

  server.handleClient();
}
