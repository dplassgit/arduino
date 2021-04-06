#include <SoftwareSerial.h>
#include <RF24.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "data.h"

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define BAUD_RATE 57600
SoftwareSerial mySerial(4, 5); // RX, TX

RF24 radio(9, 8);  // CE (disconnected), CSN
#define NRF_INT_PIN 3

// Address through which two modules communicate over the radio.
const byte address[6] = "flori";

// Our data that we will send over serial.
struct Data data;

void setup() {
  data.id = 'H';

  Serial.begin(9600);
  Serial.println("Hello nrf_receiver");

  mySerial.begin(BAUD_RATE);

  radio.begin();
  // set the address etc
  radio.setChannel(0x66);
  radio.setPALevel(RF24_PA_MAX);
  radio.openReadingPipe(1, address);

  // Set module as receiver
  radio.startListening();

  sensors.begin();  // Start up the DS18B library

  attachInterrupt(digitalPinToInterrupt(NRF_INT_PIN), intHandler, FALLING);
}

void intHandler() {
  bool txok, txfail, rxread;
  radio.whatHappened(txok, txfail, rxread);
  if (rxread) {
    if (radio.available()) {
      struct Data remoteData;
      radio.read(&remoteData, RX_DATA_SIZE);
      char temp[32];
      snprintf_P(temp,
                 sizeof(temp),
                 PSTR("%c%c: %d F %d V"),
                 remoteData.counter,
                 remoteData.id,
                 (short)remoteData.tempF,
                 remoteData.voltage);
      Serial.print("Data received ~: ");
      Serial.println(temp);
      setChecksum(&remoteData);
      int sent = mySerial.write((byte*) &remoteData, TX_DATA_SIZE);
      if (sent != TX_DATA_SIZE) {
        Serial.println("Forwarding FAILED");
      }
      // In case there was an issue with the base, this gives it a chance to recover
      delay(500);
      return;
    }
  }
  Serial.println("Not available");
}

void setChecksum(struct Data *remoteData) {
  short checksum = 0;
  byte *raw = (byte *)remoteData;
  for (int i = 0; i < RX_DATA_SIZE; ++i) {
    checksum += raw[i] * 3 * (i + 1);
  }
  Serial.print("Setting checksum to: "); Serial.println(checksum);
  remoteData->checksum = checksum;
}

int counter = 0;

void loop() {
  // Send command to all the sensors for temperature conversion
  sensors.requestTemperatures();

  Serial.print("Local temperature: ");
  float tempC = sensors.getTempCByIndex(0);
  Serial.print(tempC);
  Serial.print("C | ");
  float tempF = DallasTemperature::toFahrenheit(tempC);
  Serial.print(tempF);
  Serial.println("F");

  // Send message to base
  data.tempF = tempF;
  data.counter = 'A' + counter;
  char text[32];
  snprintf_P(text,
             sizeof(text),
             PSTR("%c%c: %d F %d V"),
             data.counter,
             data.id,
             (short)data.tempF,
             data.voltage
            );
  Serial.print("Trying to send ~: "); Serial.println(text);
  setChecksum(&data);
  int sent = mySerial.write((byte*) &data, TX_DATA_SIZE);
  if (sent != TX_DATA_SIZE) {
    Serial.println("Sending failed");
  }
  counter++;
  if (counter == 26) counter = 0;

  delay(5000);
}
