//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <ThreeWire.h>
// #include <dht.h>

// Data wire is plugged into digital pin 7 on the Arduino
#define ONE_WIRE_BUS 7

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

//create an RF24 object
RF24 radio(9, 8);  // CE, CSN

//address through which two modules communicate.
const byte address[6] = "flori";

int deviceCount;
void setup() {
  Serial.begin(9600);
  Serial.println("Hello nrf_sender");
  radio.begin();
  radio.setChannel(0x66);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(15, 15);

  //set the address
  radio.openWritingPipe(address);

  //Set module as transmitter
  radio.stopListening();
  sensors.begin();  // Start up the library

  Serial.print("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" devices.");
}

int counter = 0;

#define countof(a) (sizeof(a) / sizeof(a[0]))

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

  // Send message to receiver
  char text[32];// = " Hello World";
  snprintf_P(text,
             countof(text),
             // CHANGE ME:
             PSTR(" A: %d F"),
             (int)sendingTemp
            );
  text[0] = 'A' + counter;
  Serial.print("Trying to send "); Serial.println(text);
  if (!radio.write(&text, sizeof(text))) {
    Serial.println("Not connected?");
  }
  Serial.println("Sent, waiting 5 secs");
  counter++;
  if (counter == 26) counter = 0;

  delay(5000);
}
