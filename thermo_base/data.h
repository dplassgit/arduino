// Data sent to the base
struct Data {
  char counter; // Counter, A-Z
  char id;      // See ID, above
  short voltage;  // mV
  float tempF;
  short checksum;
};

#define RX_DATA_SIZE 10
#define SUMMARY_SIZE 32

struct RemoteMetaData {
  struct Data data;
  float minTemp;
  float maxTemp;
  long when;
  short missed;
  char summary[SUMMARY_SIZE];
};
