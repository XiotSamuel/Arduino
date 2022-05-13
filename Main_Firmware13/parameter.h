
uint32_t chipId = 0;

struct toRootReport {
  String reportTime;
  long unsigned int epoch;
  int deviceID;
  long unsigned int luxTime[60];
  float luxData[60];
  long unsigned int pirTime[60];
  int pirData[60];
} TOROOTREPORT;

struct deviceInformation {
  String companyName;
  String siteName;
  String group;
  String deviceID;
  String deviceName;
  long unsigned int bootTime;
} DEVICEINFORMATION;

struct credentials {
  String mesh_prefix;
  String mesh_password;
  uint16_t mesh_port = 5555;
  String wifi_ssid;
  String wifi_password;
}CREDENTIALS;


//Structure for the nodeStatus
struct nodeStatus {
  int lux_meter;
  int pir_counter;
  bool lightONOFF;
  int lightLevel;
  int colour[3];
  int internalTemp;
} NODESTATUS;


//for the temperature sensor reading
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
