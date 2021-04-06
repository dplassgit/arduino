// Data sent to the base. NOTE: Field order is significant.
// Also, on the Nano, an int is 2 bytes but on the ESP8266 it's 4 bytes, so we use short.
struct Data {
  char counter; // Counter, A-Z
  char id;      // One-letter ID of this sender. Second slot for historical reasons.
  short voltage;  // mV
  float tempF;
  short checksum;
};

#define RX_DATA_SIZE 8
#define TX_DATA_SIZE 10
