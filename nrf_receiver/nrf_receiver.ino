#include <SoftwareSerial.h>
#include <RF24.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include "data.h"

#define ONE_WIRE_BUS 2

// Set up a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);
// Number of thermometers
int deviceCount;

#define BAUD_RATE 57600

SoftwareSerial mySerial(4, 5); // RX, TX

RF24 radio(9, 8);  // CE, CSN

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
  Serial.print("Locating DS18Bs...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount);
  Serial.println(" devices.");
  attachInterrupt(digitalPinToInterrupt(NRF_INT_PIN), intHandler, FALLING);
}

void intHandler() {
  if (radio.available()) {
    struct Data remoteData;
    radio.read(&remoteData, sizeof(remoteData));
    char temp[32];
    snprintf_P(temp,
               sizeof(temp),
               PSTR("%c%c: %d F %d V"),
               remoteData.counter,
               remoteData.id,
               (short)remoteData.tempF,
               remoteData.voltage);
    Serial.print("Data received equivalent to: ");
    Serial.println(temp);
    Serial.println("Forwarding");
    mySerial.write((byte*) &remoteData, sizeof(remoteData));
  } else {
    Serial.println("Not available");
  }
}

int counter = 0;

void loop() {
  // Send command to all the sensors for temperature conversion
  sensors.requestTemperatures();

  float sendingTemp;
  // Display temperature from each sensor
  for (int i = 0; i < deviceCount; ++i) {
    Serial.print(i);
    Serial.print(" Temp: ");
    float tempC = sensors.getTempCByIndex(i);
    Serial.print(tempC);
    Serial.print("C  |  ");
    float tempF = DallasTemperature::toFahrenheit(tempC);
    if (i == 0) {
      sendingTemp = tempF;
    }
    Serial.print(tempF);
    Serial.println("F");
  }

  // Send message to base
  data.tempF = sendingTemp;
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
  Serial.print("Trying to send equivalent of: "); Serial.println(text);
  mySerial.write((byte*) &data, sizeof(data));
  counter++;
  if (counter == 26) counter = 0;

  delay(5000);
}
