#include <Arduino.h>
#include "settings.h"
#include <WiFiUdp.h>
#include "osc/OSCMessage.h"

extern SETTINGS_T user_config;
WiFiUDP Udp;

void sendOscMessage(const char *address, const char *message)
{
    IPAddress outIp;
    outIp.fromString(user_config.osc_address);
    int outPort = user_config.osc_port;

    Serial.print("Sending OSC Message - ");
    Serial.print(address);
    Serial.print(" : ");
    Serial.print(message);
    Serial.print(" to ");
    Serial.print(outIp);
    Serial.print(":");
    Serial.println(outPort);

    OSCMessage msg(address);
    msg.add(message);
    Udp.beginPacket(outIp, outPort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
}

void button1Handler(uint8_t pin, uint8_t event, uint8_t count, uint16_t length)
{
    Serial.print("Event : ");
    Serial.print(event);
    Serial.print(" Count : ");
    Serial.print(count);
    Serial.print(" Length: ");
    Serial.print(length);
    Serial.println();

    if (event == 3)
    {
        sendOscMessage(user_config.osc_message1, "ff0000");
    }
}

void button2Handler(uint8_t pin, uint8_t event, uint8_t count, uint16_t length)
{
    Serial.print("Event : ");
    Serial.print(event);
    Serial.print(" Count : ");
    Serial.print(count);
    Serial.print(" Length: ");
    Serial.print(length);
    Serial.println();

    if (event == 3)
    {
        sendOscMessage(user_config.osc_message2, "ff0000");
    }
}

void button3Handler(uint8_t pin, uint8_t event, uint8_t count, uint16_t length)
{
    Serial.print("Event : ");
    Serial.print(event);
    Serial.print(" Count : ");
    Serial.print(count);
    Serial.print(" Length: ");
    Serial.print(length);
    Serial.println();

    if (event == 3)
    {
         sendOscMessage(user_config.osc_message3, "ff0000");
    }
}
