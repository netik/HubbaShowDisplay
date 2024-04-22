#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#define CONFIG_VERSION 1      // config struct version. Changing this will force a reconfiguration!
#define CONFIG_MAGIC   "BOOB" // magic number to check if the settings are valid in the EEPROM

typedef struct {
  char magic[5];
  int version;

  char ssid[30];        // wifi ssid
  char password[30];    // wifi password or blank
  
  char osc_address[50]; // osc address for destination calls
  int  osc_port;        // osc port for destination calls

  char cstr_terminator; // make sure the struct is null terminated
} SETTINGS_T;

#endif