#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <ThreeWire.h>

#undef BASEMENT
#undef AARON
#undef GARAGE

#define AARON

#if defined(AARON) || defined(GARAGE)
// Data wire is plugged into digital pin 7 on the Arduino
#define ONE_WIRE_BUS 7
#else
// Data wire is plugged into digital pin 7 on the Basement pcb
#define ONE_WIRE_BUS 2
#endif

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

  int vcc = (int) readVcc();

  // Send message to receiver
  char text[32];
  const char *pattern;
#ifdef GARAGE
  pattern = PSTR(" G: %d F %d V");
#endif
#ifdef BASEMENT
  pattern = PSTR(" b: %d F %d V");
#endif
#ifdef AARON
  pattern = PSTR(" A: %d F %d V");
#endif
  snprintf_P(text,
             countof(text),
             pattern,
             (int)sendingTemp,
             vcc
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

long readVcc() {
  long result; // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);

  delay(2);
  // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);
  // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;

#ifdef BASEMENT
  // Accurate to within 10mv on the basement Nano
  result = 1008000L / result;
#endif
#ifdef GARAGE
  // Accurate to within 10mv on the garage Nano
  result = 1005000L / result;
#endif
#ifdef AARON
  // This is accurate to within 10mv on the Aaron Nano
  result = 1020000L / result;
#endif
  // Back-calculate AVcc in mV
  return result;
}
