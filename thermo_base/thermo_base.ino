/** FOR UPLOADING: Use NodeMCU 1.0 (ESP-12E Module) */
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h>

#define min(a,b) ((a<b)?(a):(b))
#define max(a,b) ((a>b)?(a):(b))

#include "LEDDisplayDriver.h"
#include "config.h"
#include "data.h"

#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

#define NUM_DIGITS 24
/* For Uno:
  const byte dataPin = A0;
  const byte clockPin = A1;
  const byte loadPin = A2;
  const byte intPin = 2;
*/
/* For ESP8266MOD */
const byte dataPin = D1;
const byte loadPin = D2;
const byte clockPin = D3;

LEDDisplayDriver display(dataPin, clockPin, loadPin, true, NUM_DIGITS);

#define BAUD_RATE 57600
#define SERIAL_RX_PIN D7
SoftwareSerial mySerial(SERIAL_RX_PIN, D8); // RX, TX

#define NUM_REMOTES 8

static const char* NAMES[] PROGMEM =    {"Aaron's", "Basement", "Den", "Florida", "Garage", "Here", "Office", "Our Br"};
static const char* CODES PROGMEM = "ABDFGHOM";
static const float ADJUSTMENT[] = { -1.92, -1.84, -2, -1.84, -1.64, -5.4, -1, -3.32};
struct RemoteMetaData metadata[NUM_REMOTES];

// Port 80. Nyeah.
ESP8266WebServer server(80);


void setup() {
  Serial.begin(115200);
  Serial.println("Hello thermo base");

  reset();

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
  // Allows us to find this at http://base.local
  MDNS.begin("base");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  mySerial.begin(BAUD_RATE);

  server.on("/", HTTP_GET, &handleRoot);
  server.begin();
}

void reset() {
  for (int i = 0; i < NUM_REMOTES; ++i) {
    resetOne(i);
  }
}

void resetOne(int i) {
  memset(&metadata[i], 0, sizeof(struct RemoteMetaData));
  metadata[i].maxTemp = -100;
  metadata[i].minTemp = 212;
}

void serialHandler() {
  if (mySerial.available() >= RX_DATA_SIZE) {
    struct Data data;
    byte *rawData = (byte*)&data;
    for (int i = 0; i < RX_DATA_SIZE; ++i) {
      rawData[i] = mySerial.read();
    }
    // Validate the checksum
    short calcChecksum = getChecksum(&data);
    if (data.checksum != calcChecksum) {
      display.showText("ERROR");
      Serial.print("CHECKSUM MISMATCH; got "); Serial.print(data.checksum);
      Serial.print(" but calculated as "); Serial.println(calcChecksum);
      char tmp[32];
      snprintf_P(tmp,
                 sizeof(tmp),
                 PSTR("%c%c: %d F (%d)"),
                 data.counter,
                 data.id,
                 (short)data.tempF,
                 data.checksum);
      Serial.print("Data was ~:"); Serial.println(tmp);
      // Flush the input
      Serial.print("Flushing: skipping: ");
      while (mySerial.available() > 0) {
        Serial.print(mySerial.read(), HEX); Serial.print(" ");
      }
      Serial.println();
      return;
    }

    int slot = -1;
    char code = data.id;
    for (int i = 0; i < strlen(CODES); ++i) {
      if (code == CODES[i]) {
        slot = i;
        break;
      }
    }
    if (slot == -1) {
      char temp[32];
      snprintf_P(temp,
                 sizeof(temp),
                 PSTR("%c%c: %d F"),
                 data.counter,
                 data.id,
                 (short)data.tempF);
      Serial.print("UNKNOWN SOURCE; Received ~: ");
      Serial.println(temp);
      return;
    }
    snprintf_P(metadata[slot].summary,
               sizeof(metadata[slot].summary),
               PSTR("%c%c: %d F"),
               data.counter,
               data.id,
               (short)data.tempF);
    Serial.print("Received ~: ");
    Serial.print(metadata[slot].summary);

    if (data.tempF == 0 || data.tempF == -1 || data.tempF < -10 || data.tempF > 110) {
      display.showTextScroll("Weird data");
      Serial.println("Skipping weird/invalid data");
      return;
    }
    // data.tempF += ADJUSTMENT[slot];
    metadata[slot].maxTemp = max(metadata[slot].maxTemp, data.tempF);
    metadata[slot].minTemp = min(metadata[slot].minTemp, data.tempF);
    Serial.print(". Min: "); Serial.print(metadata[slot].minTemp);
    Serial.print(", Max: "); Serial.print(metadata[slot].maxTemp);
    Serial.println();
    if (metadata[slot].when != 0) {
      // See if we missed one
      if (!(data.counter == 'A' && metadata[slot].data.counter == 'Z')) {
        // Not wraparound
        if (data.counter != metadata[slot].data.counter) {
          // Not a dupe
          if (data.counter != metadata[slot].data.counter + 1) {
            metadata[slot].missed++;
            Serial.print("Missed "); Serial.print(metadata[slot].missed); Serial.print(" from "); Serial.println(data.id);
          }
        }
      }
    }
    metadata[slot].when = millis();
    memcpy(&(metadata[slot].data), &data, sizeof(struct Data));
  }
}

short getChecksum(struct Data * remoteData) {
  short checksum = 0;
  byte *raw = (byte *)remoteData;
  // Skip the actual checksum bytes
  for (int i = 0; i < RX_DATA_SIZE - 2; ++i) {
    checksum += raw[i] * 3 * (i + 1);
  }
  return checksum;
}

/** IP to String? */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

void handleRoot() {
  Serial.println("Handling /");
  if (server.hasArg("reset")) {
    String arg = server.arg("reset");
    Serial.println("resetting " + arg);
    if (arg.length() > 0) {
      long id = arg.toInt();
      resetOne(id);
    }
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return;
  }
  long now = millis();
  char buffer[500];
  snprintf_P(buffer,
             sizeof(buffer),
             "   Aaron: %s at %d. Min %d Max %d\nBasement: %s at %d. Min %d Max %d\n     Den: %s at %d. Min %d Max %d\n Florida: %s at %d. Min %d Max %d\n  Garage: %s at %d. Min %d Max %d\n    Here: %s at %d. Min %d Max %d\n  Office: %s at %d. Min %d Max %d\n  Our Br: %s at %d. Min %d Max %d\n",
             metadata[0].summary, (now - metadata[0].when) / 1000, (int)metadata[0].minTemp, (int)metadata[0].maxTemp,
             metadata[1].summary, (now - metadata[1].when) / 1000, (int)metadata[1].minTemp, (int)metadata[1].maxTemp,
             metadata[2].summary, (now - metadata[2].when) / 1000, (int)metadata[2].minTemp, (int)metadata[2].maxTemp,
             metadata[3].summary, (now - metadata[3].when) / 1000, (int)metadata[3].minTemp, (int)metadata[3].maxTemp,
             metadata[4].summary, (now - metadata[4].when) / 1000, (int)metadata[4].minTemp, (int)metadata[4].maxTemp,
             metadata[5].summary, (now - metadata[5].when) / 1000, (int)metadata[5].minTemp, (int)metadata[5].maxTemp,
             metadata[6].summary, (now - metadata[6].when) / 1000, (int)metadata[6].minTemp, (int)metadata[6].maxTemp,
             metadata[7].summary, (now - metadata[7].when) / 1000, (int)metadata[7].minTemp, (int)metadata[7].maxTemp
            );
  Serial.println(buffer);
  server.send(200, "text/plain", buffer);
}

int source = 0;
long lastShown = 0;

void loop() {
  serialHandler();

  server.handleClient();

  long now = millis();
  if (lastShown == 0 || now - lastShown > 4000) {
    // yes this fails for rollover
    lastShown = now;
    long secondsSince = (now - metadata[source].when) / 1000;
    display.showText(NAMES[source], 0, 8);
    if (secondsSince < 9999 && metadata[source].when != 0) {
      if (secondsSince > 60) {
        display.showNum(secondsSince, 8, 8);
      } else {
        display.showText("    ", 12, 4);
      }
      display.showNum1decimal(metadata[source].data.tempF, 8, 3);
      display.showText("F", 11, 1);
      char third[12];
      sprintf(third, "Lo%dHi%d",
              (int)(metadata[source].minTemp + .5),
              (int)(metadata[source].maxTemp + .5));
      display.showText(third, 16, 8);
    } else {
      display.showText("No data", 8, 16);
    }

    source++;
    if (source == NUM_REMOTES) source = 0;

    MDNS.update();
  }
}
