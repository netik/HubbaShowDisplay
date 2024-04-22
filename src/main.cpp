#include <Arduino.h>

// These need to be included when using standard Ethernet
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WiFiUDP.h>
#include <WiFiClient.h>
#include <LedController.hpp>
#include <DebounceEvent.h>

#define USE_EXT_CALLBACKS 1
#include <AppleMIDI.h>

#include "safe_strlen.h"
#include "buttondefs.h"
#include "button_handler.h"
#include "settings.h"

#define DEBUG 1 // SET TO 0 OUT TO REMOVE TRACES

#if DEBUG
#define DBG_SETUP(...) Serial.begin(__VA_ARGS__);
#define DBG_PRINT(...) Serial.print(__VA_ARGS__)
#define DBG_WRITE(...) Serial.write(__VA_ARGS__)
#define DBG(...) Serial.println(__VA_ARGS__)
#else
#define DBG_SETUP(...)
#define DBG_PRINT(...)
#define DBG_WRITE(...)
#define DBG(...)
#endif

/* Pin definitions --------------------------------------- */
#define BUTTON_RED D0    // gpio 16, internal pull up
#define BUTTON_YELLOW D1 // gpio 5, internal pull up
#define BUTTON_GREEN D2  // gpio 4, internal pull up

#define LED_CLK_PIN D5
#define LED_DATA_PIN D6
#define LED_CS_PIN D7

/* LED definitions --------------------------------------- */
#define LED_NUM_SEGMENTS 1
#define LED_DIGITS 7 // 0-based. 7 means 8 digits

/* Web server */
ESP8266WebServer server(80);

/* Fallback SoftAP Config -------------------------------- */
#define MAX_WIFI_TRIES 20
#ifndef APSSID
#define APSSID "HubbaShowNode"
#define APPSK ""
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

LedController ledController(LED_DATA_PIN, LED_CLK_PIN, LED_CS_PIN, LED_NUM_SEGMENTS);

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

void ledShowString(const char *str)
{
  // display a short string on the LED
  int currentDigit = 0;
  for (size_t i = 0; i < safe_strlen(str, 16); i++)
  {
    if (str[i] == '\0')
    {
      break;
    }

    // if the next character is a period, set the decimal point
    ledController.setChar(0, LED_DIGITS - currentDigit, str[i], str[i + 1] == '.');

    // if the next character is a period, skip the next character
    if (str[i + 1] == '.')
    {
      i++;
    }
    currentDigit++;

    if (currentDigit > LED_DIGITS)
    {
      // we are out of space
      break;
    }
  }
}

void ledShowIP()
{
  // display the IP address on the LED
  IPAddress ip = WiFi.localIP();
  ledShowString(ip.toString().c_str());
}

void displayDHCP()
{
  ledController.clearMatrix();
  // display ".dHCp."
  ledController.setChar(0, 6, ' ', true);
  ledController.setChar(0, 5, 'd', false);
  ledController.setChar(0, 4, 'h', false);
  ledController.setChar(0, 3, 'C', false);
  ledController.setChar(0, 2, 'p', true);
}

void handlePortal()
{
  if (server.method() == HTTP_POST)
  {
    DBG(F("Saving settings..."));

    strncpy(user_config.ssid, server.arg("ssid").c_str(), sizeof(user_config.ssid));
    strncpy(user_config.password, server.arg("password").c_str(), sizeof(user_config.password));
    strncpy(user_config.osc_address, server.arg("osc_address").c_str(), sizeof(user_config.osc_address));
    user_config.osc_port = server.arg("osc_port").toInt();

    DBG(user_config.osc_port);

    user_config.ssid[server.arg("ssid").length()] = user_config.password[server.arg("password").length()] = '\0';
    strncpy(user_config.magic, CONFIG_MAGIC, 5);
    user_config.version = CONFIG_VERSION;

    EEPROM.put(0, user_config);
    EEPROM.commit();
    server.send(200, "text/html", F("<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title><style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}</style> </head> <body><main class='form-signin'> <h1>Node Setup</h1> <br/> <p>Your settings have been saved successfully!<br />Please restart the device.</p></main></body></html>"));
  }
  else
  {
    server.send(200, "text/html", F("<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title> <style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{cursor: pointer;border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1{text-align: center}</style> </head> <body><main class='form-signin'> <form action='/' method='post'> <h1 class=''>Node Setup</h1><br/><div class='form-floating'><label>SSID</label><input type='text' class='form-control' name='ssid'> </div><div class='form-floating'><br/><label>Password</label><input type='password' class='form-control' name='password'></div><div class='form-floating'><br/><label>OSC Destination Address</label><input type='text' class='form-control' name='osc_address'></div><div class='form-floating'><br/><label>OSC Destination Port</label><input type='text' class='form-control' name='osc_port'></div><br/><br/><button type='submit'>Save</button><p style='text-align: right'><a href='https://www.mrdiy.ca' style='color: #32C5FF'>mrdiy.ca</a></p></form></main> </body></html>"));
  }
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
  server.on("/", handlePortal);
  server.begin();
}

void networkInit()
{
  EEPROM.begin(512);
  EEPROM.get(0, user_config);
  DBG(F("\n\nRead EEPROM"));

  // holding down the button at boot will force the device into AP mode
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
  DBG("IP address is " + WiFi.localIP().toString());

  ledShowIP();
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

char getSysExStatus(const byte *data, uint16_t length)
{
  if (data[0] == 0xF0 && data[length - 1] == 0xF7)
    return 'F'; // Full SysEx Command
  else if (data[0] == 0xF0 && data[length - 1] != 0xF7)
    return 'S'; // Start of SysEx-Segment
  else if (data[0] != 0xF0 && data[length - 1] != 0xF7)
    return 'M'; // Middle of SysEx-Segment
  else
    return 'E'; // End of SysEx-Segment
}

void OnMidiSysEx(byte *data, unsigned length)
{
  Serial.print(F("SYSEX: ("));
  Serial.print(getSysExStatus(data, length));
  Serial.print(F(", "));
  Serial.print(length);
  Serial.print(F(" bytes) "));
  for (uint16_t i = 0; i < length; i++)
  {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void displayTimeCode()
{
  char timeCodeStr[12];

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
  snprintf(timeCodeStr, 12, "%02d.%02d.%02d.%02d", hour, minute, second, frame);
  ledShowString(timeCodeStr);
}
void setup()
{
  DBG_SETUP(115200);

  ledInit();
  networkInit();

  // calls requiring network access go below this line.
  if (AP_enabled)
  {
    return;
  }

  // we'll always run a webserver if you want to reconfigure.
  server.on("/", handlePortal);
  server.begin();

  DBG("AppleMIDI Name: " + String(AppleMIDI.getName()));
  DBG("AppleMIDI Port: " + String(AppleMIDI.getPort()));

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t &ssrc, const char *name)
                               {
        isConnected++;
        DBG(F("Connected to session"));
        DBG(ssrc);
        DBG(name); 
        ledController.clearMatrix();
        ledShowString("cOnnEcT");
  });

  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t &ssrc) {
        isConnected--;
        DBG_PRINT(F("Disconnected"));
        DBG(ssrc);
        ledController.clearMatrix();
        ledShowString("disconn");
  });
  
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
    }
  });

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

  server.handleClient();

  // calls requiring network access go below this line.
  if (AP_enabled)
  {
    return;
  }

  MIDI.read();
}