#include <Arduino.h>

// These need to be included when using standard Ethernet
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <WiFiUDP.h>
#include <WiFiClient.h>
#include <DebounceEvent.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LedController.hpp>

// AppleMIDI
#define USE_EXT_CALLBACKS 1

#include <AppleMIDI.h>

#include "led.h"
#include "safe_strlen.h"
#include "button_handler.h"
#include "settings.h"
#include "LittleFS.h"
#include "pins.h"
#include "debug.h"

/* Web server */
AsyncWebServer server(80);

/* LED (see led.cpp) */
extern LedController ledController;

/* Fallback SoftAP Config -------------------------------- */
#define MAX_WIFI_TRIES 20
#ifndef APSSID
#define APSSID "HubbaShowNode"
#define APPSK "" // no password
#endif

/* Set these to your desired credentials. */
const char *AP_ssid = APSSID;
const char *AP_password = APPSK;

/* IP Configuration for Soft AP */
IPAddress AP_IP = IPAddress(10, 1, 1, 1);
IPAddress AP_SUBNET = IPAddress(255, 255, 255, 0);
IPAddress AP_GATEWAY = IPAddress(10, 1, 1, 2);

bool AP_enabled = false;

SETTINGS_T user_config = {};

DebounceEvent buttonRed = DebounceEvent(BUTTON_RED, button1Handler, BUTTON_PUSHBUTTON | BUTTON_DEFAULT_HIGH | BUTTON_SET_PULLUP);
DebounceEvent buttonYel = DebounceEvent(BUTTON_YELLOW, button2Handler, BUTTON_PUSHBUTTON | BUTTON_DEFAULT_HIGH | BUTTON_SET_PULLUP);
DebounceEvent buttonGrn = DebounceEvent(BUTTON_GREEN, button3Handler, BUTTON_PUSHBUTTON | BUTTON_DEFAULT_HIGH | BUTTON_SET_PULLUP);

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

struct
{
  byte frame_ls;
  byte frame_ms;
  byte seconds_ls;
  byte seconds_ms;
  byte minutes_ls;
  byte minutes_ms;
  byte hours_ls;
  byte hours_ms;
} timeCode;

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Page Not found");
}

void sendConfigForm(AsyncWebServerRequest *request)
{
  request->send(LittleFS, "/config.html", "text/html", false, configFormProcessor);
}

void startAP()
{
  // possible crashing on insufficient power?
  WiFi.setOutputPower(15.0);

  // WiFi.mode(WIFI_AP_STA);
  DBG(WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET) ? "AP Ready" : "Failed!");

  if (WiFi.softAP(AP_ssid, AP_password))
  {
    DBG("Ready. Access point IP: " + WiFi.softAPIP().toString());

    ledShowString("AP-n0de");
  }
  else
  {
    DBG("Setting up access point failed!");
  }

  // stop any other network calls in the main loop with this flag. It seems if
  // query status over and over again while in softAP mode, the ESP8266 will
  // crash
  AP_enabled = true;
  server.onNotFound(notFound);
  server.begin();
}

void networkInit()
{
  EEPROM.begin(512);
  EEPROM.get(0, user_config);
  DBG(F("\n\nRead EEPROM"));

  // holding down the button at boot will force the device into AP mode and wipe the config.
  if (digitalRead(BUTTON_RED) == LOW)
  {
    memset(&user_config, 0, sizeof(user_config));
    EEPROM.put(0, user_config);
    EEPROM.commit();
    DBG(F("WiFi settings reset"));
    DBG(F("Forcing AP mode"));
    ledShowString("-rEsEt-");
    startAP();
    return;
  }

  if (strncmp(user_config.magic, CONFIG_MAGIC, 4) != 0 || user_config.version != CONFIG_VERSION)
  {
    // no wifi settings stored
    DBG(F("No WiFi settings stored"));
    ledShowString(" no nEt");

    // clear the config
    memset(&user_config, 0, sizeof(user_config));
    startAP();
    return;
  }

  DBG(F("WiFi settings found"));

  DBG("SSID: " + String(user_config.ssid));
  DBG("Password: " + String(user_config.password));
  DBG("OSC Address: " + String(user_config.osc_address));
  DBG("OSC Port: " + String(user_config.osc_port));

  displayDHCP();

  Serial.print(F("Getting IP address..."));
  WiFi.mode(WIFI_STA);
  WiFi.begin(user_config.ssid, user_config.password);

  bool dhcpLEDState = false;
  byte tries = 0;

  while (WiFi.status() != WL_CONNECTED)
  {
    ledController.setChar(0, 6, ' ', dhcpLEDState);
    ledController.setChar(0, 2, 'p', dhcpLEDState);
    delay(1000);
    Serial.print(F("."));
    dhcpLEDState = !dhcpLEDState;

    if (tries++ > MAX_WIFI_TRIES)
    {
      // we have been trying for 10 seconds
      startAP();
      return;
    }
  }

  DBG();
  
  DBG(F("WiFi connected"));

  IPAddress ip = WiFi.localIP();
  DBG("IP address is " + ip.toString());
  ledShowIP(ip);
}

void ledInit()
{
  ledController.activateAllSegments();
  ledController.setIntensity(15);
  ledController.clearMatrix();
}

int isConnected = 0;

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================
void displayTimeCode()
{
  char timeCodeStr[20];

  byte hour = timeCode.hours_ls + ((timeCode.hours_ms & 0x01) << 4);
  byte minute = timeCode.minutes_ms << 4 | timeCode.minutes_ls;
  byte second = timeCode.seconds_ms << 4 | timeCode.seconds_ls;
  byte frame = timeCode.frame_ms << 4 | timeCode.frame_ls;

  // first convert timeCode to a string where the digits are bytes. Respect the nibble!
  //
  // Special case for the hours field (type 6 and 7)
  //
  // Note that the last row in the table above describes a message that is a
  // bit different than the rest. When the high nibble of the second byte of
  // the MIDI quarter frame message is 7, the low nibble bits are organized
  // as follows.
  //
  // XYYZ
  //
  // The bit X is not used. The bit Z is bit 4 (counting from 0) of the
  // "hours" portion of the MIDI time code (which means that "hours" has
  // only 5 bits and cannot exceed 31). The YY bits describe the number of
  // frames per second. These YY bits can have the following meaning.
  snprintf(timeCodeStr, 20, "%02d.%02d.%02d.%02d", hour, minute, second, frame);

  ledShowString(timeCodeStr);
}

String processor(const String &var)
{
  Serial.println(var);
  // FIX ME
  return String("XXX");
}

void dumpFiles()
{

  Serial.println("File list: ");
  Dir dir = LittleFS.openDir("/");
  while (dir.next())
  {
    Serial.println(dir.fileName());
  }
  Serial.println("---");
}

void setup()
{
  DBG_SETUP(115200);

  LittleFS.begin();
  dumpFiles();

  ledInit();
  networkInit();

  // we'll always run a web server if you want to reconfigure.
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { sendConfigForm(request); });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request)
            { 
              handleUpdateConfig(request); 
              request->send(LittleFS, "/done.html", "text/html", false, nullptr);
            });

  server.begin();

  // Anything requiring the network to be up, should go below this conditional.
  if (AP_enabled)
  {
    return;
  }

  DBG("AppleMIDI Name: " + String(AppleMIDI.getName()));
  DBG("AppleMIDI Port: " + String(AppleMIDI.getPort()));

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t &ssrc, const char *name)
                               {
        isConnected++;
        DBG(F("Connected to session"));
        DBG(ssrc);
        DBG(name); 
        ledController.clearMatrix();
        ledShowString("cOnnEcT"); });

  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t &ssrc)
                                  {
        isConnected--;
        DBG_PRINT(F("Disconnected"));
        DBG(ssrc);
        ledController.clearMatrix();
        ledShowString("disconn"); });

  MIDI.begin();

  MIDI.setHandleTimeCodeQuarterFrame([](byte data)
                                     {
    /*
      A quarter frame message contains two bytes. The first byte is F1, the
      Quarter Frame System Common byte. The second byte contains a nibble that
      represents the message number (0 through 7), and a nibble for one of the
      digits of a time field (hours, minutes, seconds or frames). Quarter
      frame messages are transmitted in a sequence of eight messages, thus a
      complete timecode value is specified every two frames.
    */

    byte messageType = data >> 4;
    byte value = data & 0x0F;

    switch (messageType)
    {
    case 0:
      timeCode.frame_ls = value;
      break;
    case 1:
      timeCode.frame_ms = value;
      break;
    case 2:
      timeCode.seconds_ls = value;
      break;
    case 3:
      timeCode.seconds_ms = value;
      break;
    case 4:
      timeCode.minutes_ls = value;
      break;
    case 5:
      timeCode.minutes_ms = value;
      break;
    case 6:
      timeCode.hours_ls = value;
      break;
    case 7:
      timeCode.hours_ms = value;
      displayTimeCode();
      break;
    } });

  // clear timecode
  timeCode.frame_ls = 0;
  timeCode.frame_ms = 0;
  timeCode.seconds_ls = 0;
  timeCode.seconds_ms = 0;
  timeCode.minutes_ls = 0;
  timeCode.minutes_ms = 0;
  timeCode.hours_ls = 0;
  timeCode.hours_ms = 0;
}

void loop()
{
  buttonRed.loop();
  buttonYel.loop();
  buttonGrn.loop();

  if (AP_enabled)
  {
    return;
  }

  // --- calls requiring network access go below this line. ---
  MIDI.read();
}