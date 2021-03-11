//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(9, 8);  // CE, CSN

//address through which two modules communicate.
const byte address[6] = "flori";

void setup() {
  Serial.begin(9600);
  Serial.println("Hello receiver");

  radio.begin();

  //set the address
  radio.openReadingPipe(0, address);

  //Set module as receiver
  radio.startListening();

  attachInterrupt(digitalPinToInterrupt(2), intHandler, FALLING);
}

void intHandler() {
  //Read the data if available in buffer
  if (radio.available()) {
    char text[32] = {0};
    radio.read(&text, sizeof(text));
    Serial.println(text);
  } else {
    Serial.println("Not available");
  }
}


void loop() {
}
