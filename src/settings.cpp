#include <Arduino.h>
#include <ESPAsyncWebserver.h>
#include <EEPROM.h>
#include "debug.h"
#include "settings.h"

extern SETTINGS_T user_config;

String configFormProcessor(const String &var)
{

  if (var == "SSID")
    return String(user_config.ssid);

  if (var == "PASSWORD")
    return String(user_config.password);

  if (var == "OSC_ADDRESS")
    return String(user_config.osc_address);

  if (var == "OSC_PORT")
    return String(user_config.osc_port);

  if (var == "OSC_MESSAGE1")
    return String(user_config.osc_message1);

  if (var == "OSC_MESSAGE2")
    return String(user_config.osc_message2);

  if (var == "OSC_MESSAGE3")
    return String(user_config.osc_message3);

  if (var == "OSC_DATA1")
    return String(user_config.osc_data1);

  if (var == "OSC_DATA2")
    return String(user_config.osc_data2);

  if (var == "OSC_DATA3")
    return String(user_config.osc_data3);

  return String();
}


void handleUpdateConfig(AsyncWebServerRequest *request)
{
  DBG(F("Saving settings..."));

  String ssid;
  String password;
  String osc_address;
  String osc_port;
  String osc_message1;
  String osc_message2;
  String osc_message3;
  String osc_data1;
  String osc_data2;
  String osc_data3;

  if (request->hasParam("ssid", true))
  {
    String ssid = request->getParam("ssid", true)->value();
    strncpy(user_config.ssid, ssid.c_str(), sizeof(user_config.ssid));
  }

  if (request->hasParam("password", true))
  {
    String password = request->getParam("password", true)->value();
    strncpy(user_config.password, password.c_str(), sizeof(user_config.password));
  }

  if (request->hasParam("osc_address", true))
  {
    String osc_address = request->getParam("osc_address", true)->value();
    strncpy(user_config.osc_address, osc_address.c_str(), sizeof(user_config.osc_address));
  }

  if (request->hasParam("osc_port", true))
  {
    String osc_port = request->getParam("osc_port", true)->value();
    user_config.osc_port = osc_port.toInt();
  }

  if (request->hasParam("osc_message1", true))
  {
    String osc_message1 = request->getParam("osc_message1", true)->value();
    strncpy(user_config.osc_message1, osc_message1.c_str(), sizeof(user_config.osc_message1));
  }

  if (request->hasParam("osc_message2", true))
  {
    String osc_message2 = request->getParam("osc_message2", true)->value();
    strncpy(user_config.osc_message2, osc_message2.c_str(), sizeof(user_config.osc_message2));
  }

  if (request->hasParam("osc_message3", true))
  {
    String osc_message3 = request->getParam("osc_message3", true)->value();
    strncpy(user_config.osc_message3, osc_message3.c_str(), sizeof(user_config.osc_message3));
  }

  if (request->hasParam("osc_data1", true))
  {
    String osc_data1 = request->getParam("osc_data1", true)->value();
    strncpy(user_config.osc_data1, osc_data1.c_str(), sizeof(user_config.osc_data1));
  }

  if (request->hasParam("osc_data2", true))
  {
    String osc_data2 = request->getParam("osc_data2", true)->value();
    strncpy(user_config.osc_data2, osc_data2.c_str(), sizeof(user_config.osc_data2));
  }

  if (request->hasParam("osc_data3", true))
  {
    String osc_data3 = request->getParam("osc_data3", true)->value();
    strncpy(user_config.osc_data3, osc_data3.c_str(), sizeof(user_config.osc_data3));
  }

  user_config.cstr_terminator = 0;

  DBG(user_config.osc_port);

  strncpy(user_config.magic, CONFIG_MAGIC, 5);
  user_config.version = CONFIG_VERSION;

  EEPROM.put(0, user_config);
  EEPROM.commit();
}