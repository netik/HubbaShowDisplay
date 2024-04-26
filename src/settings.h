#include <ESPAsyncWebServer.h>

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#define CONFIG_VERSION 4      // config struct version. Changing this will force a reconfiguration!
#define CONFIG_MAGIC   "BOOB" // magic number to check if the settings are valid in the EEPROM

typedef struct {
  char magic[5];
  int version;

  char ssid[30];        // wifi ssid
  char password[30];    // wifi password or blank
  
  char osc_address[50]; // osc address for destination calls
  int  osc_port;        // osc port for destination calls

  char osc_message1[30]; // osc command sent when button pushed
  char osc_message2[30]; // osc command sent when button pushed
  char osc_message3[30]; // osc command sent when button pushed

  char osc_data1[30]; // osc command sent when button pushed
  char osc_data2[30]; // osc command sent when button pushed
  char osc_data3[30]; // osc command sent when button pushed

  char cstr_terminator; // make sure the struct is null terminated
} SETTINGS_T;

extern void initConfig();
extern String configFormProcessor(const String &var);
extern void handleUpdateConfig(AsyncWebServerRequest *request);

#endif