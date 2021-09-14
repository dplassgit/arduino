#include <RF24.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "data.h"

#define countof(a) (sizeof(a) / sizeof(a[0]))

// A=Aaron's room
// b=basement
// D=Den
// g or G=garage
// F=Florida room
// H=Here (Kitchen)
// M=Master br
// O=Office
// D=Den
#define ID 'D' 

// Default/average:
#define VCC_FACTOR 1008000L

#define MAX_TRIES 4

// Data wire is plugged into digital pin 2 on the pcbs
#define ONE_WIRE_BUS 2

// Set up a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);
// Number of thermometers
int deviceCount;

// NRF24L01+
RF24 radio(9, 8);  // CE, CSN

// Address through which two modules communicate.
const byte address[6] = "flori";


struct Data data;

void setup() {
  data.id = ID;
  Serial.begin(9600);
  Serial.println("Hello nrf_sender");

  radio.begin();
  radio.setChannel(0x66);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(15, 15);

  // Set this module as transmitter
  radio.stopListening();

  // Set the address
  radio.openWritingPipe(address);

  sensors.begin();  // Start up the DS18B library

  Serial.print("Locating DS18Bs...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount);
  Serial.println(" devices.");
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

  // Send message to receiver
  data.tempF = sendingTemp;
  short vcc = (short) readVcc();
  data.voltage = vcc;
  data.counter = 'A' + counter;

  char text[32];
  snprintf_P(text,
             countof(text),
             PSTR("%c%c: %d F %d V"),
             data.counter,
             data.id,
             (short)data.tempF,
             data.voltage
            );
  Serial.print("Trying to send equivalent of: "); Serial.println(text);

  int tries = 0;
  for (; tries < MAX_TRIES && !radio.write(&data, sizeof(data)); ++tries ) {
    Serial.println("Not connected...");
    delay((tries + 1) * 300);
  }
  if (tries == MAX_TRIES) {
    Serial.println("Gave up...");
  } else {
    Serial.println("Sent!");
  }
  counter++;
  if (counter == 26) counter = 0;

  delay(5000);
}


long readVcc() {
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

  // Wait for Vref to settle
  delay(10);

  ADCSRA |= _BV(ADSC);
  // Convert
  while (bit_is_set(ADCSRA, ADSC));

  // Read 1.1V reference against AVcc
  long result = ADCL;
  result |= ADCH << 8;

  // Back-calculate AVcc in mV
  result = VCC_FACTOR / result;
  return result;
}
