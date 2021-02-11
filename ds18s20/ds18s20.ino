#include <OneWire.h>
#include <DallasTemperature.h>

#include <ThreeWire.h>
#include <dht.h>

// Data wire is plugged into digital pin 7 on the Arduino
#define ONE_WIRE_BUS 7

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

dht DHT;
#define DHT11_PIN 2
int deviceCount = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Hello dS18S20");
  sensors.begin();  // Start up the library
  // sensors.setResolution(10);
  Serial.println("Starting");

  Serial.print("Locating devices...");
  Serial.print("Found ");
  deviceCount = sensors.getDeviceCount();
  Serial.print(deviceCount, DEC);
  Serial.println(" devices.");
}


void loop() {
  // Send command to all the sensors for temperature conversion
  sensors.requestTemperatures();

  // Display temperature from each sensor
  for (int i = 0;  i < deviceCount;  i++)
  {
    Serial.print("Sensor ");
    Serial.print(i + 1);
    Serial.print(" : ");
    float tempC = sensors.getTempCByIndex(i);
    Serial.print(tempC);
    Serial.print("C  |  ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
    Serial.println("F");
  }
  //print the temperature in Celsius
  //  Serial.print("DS18 Temperature: ");
  //  Serial.print(sensors.getTempCByIndex(0));
  //  Serial.print("C  |  ");
  //
  //  //print the temperature in Fahrenheit
  //  Serial.print((sensors.getTempCByIndex(0) * 9.0) / 5.0 + 32.0);
  //  Serial.println("F");

  int chk = DHT.read11(DHT11_PIN);
  double c = DHT.temperature;
  if (c == -999) {
    Serial.println("Waiting for sensor");
    delay(1000);
    return;
  }
  Serial.print("DHT11 Temperature: ");
  Serial.print(c);
  Serial.print("C  |  ");
  double f = (c * 9.0 / 5.0) + 32.0;
  Serial.print(f);
  Serial.println("F");

  Serial.print("Relative humidity ");
  double rh = DHT.humidity;
  Serial.println(rh);

  delay(5000);
}
