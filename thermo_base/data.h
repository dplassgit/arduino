// Data sent to the base
struct Data {
  char counter; // Counter, A-Z
  char id;      // See ID, above
  short voltage;  // mV
  float tempF;
  short checksum;
};

#define RX_DATA_SIZE 10

struct RemoteMetaData {
  struct Data data;
  float minTemp;
  float maxTemp;
  long when;
  short missed;
  char summary[32];
};
