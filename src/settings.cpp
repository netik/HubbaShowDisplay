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

  if (var == "OSC_BUTTON1")
    return String(user_config.osc_button1);

  if (var == "OSC_BUTTON2")
    return String(user_config.osc_button2);

  if (var == "OSC_BUTTON3")
    return String(user_config.osc_button3);

  return String();
}


void handleUpdateConfig(AsyncWebServerRequest *request)
{
  DBG(F("Saving settings..."));

  String ssid;
  String password;
  String osc_address;
  String osc_port;
  String osc_button1;
  String osc_button2;
  String osc_button3;

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

  if (request->hasParam("osc_button1", true))
  {
    String osc_button1 = request->getParam("osc_button1", true)->value();
    strncpy(user_config.osc_button1, osc_button1.c_str(), sizeof(user_config.osc_button1));
  }

  if (request->hasParam("osc_button2", true))
  {
    String osc_button2 = request->getParam("osc_button2", true)->value();
    strncpy(user_config.osc_button2, osc_button2.c_str(), sizeof(user_config.osc_button2));
  }

  if (request->hasParam("osc_button3", true))
  {
    String osc_button3 = request->getParam("osc_button3", true)->value();
    strncpy(user_config.osc_button3, osc_button3.c_str(), sizeof(user_config.osc_button3));
  }

  DBG(user_config.osc_port);

  strncpy(user_config.magic, CONFIG_MAGIC, 5);
  user_config.version = CONFIG_VERSION;

  EEPROM.put(0, user_config);
  EEPROM.commit();
}