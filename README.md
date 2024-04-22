# HubbaShowDisplay
An ESP8266/Arduino hardware project which can send OSC messages, receive and display Midi Time Code over Apple MIDI (WiFI). 

This is built on the NodeMCU / Ai-Thinker ESP8266 12-E Board, which you can buy on Amazon. There are so many clones of this but they all work. 
https://www.amazon.com/HiLetgo-Internet-Development-Wireless-Micropython/dp/B010O1G1ES

It also requires a 8 segment display based on the MAX 7219. 

A suitable one is here:
https://www.amazon.com/MAX7219-8-Digital-7-Segment-Display-Raspberry/dp/B086GKV958

Schematics are coming soon, I need to draw them up. Right now the connections between the ESP8266 and display are as follows:

Three buttons, connected to ground and the following pins:

#define BUTTON_RED D0    // gpio 16, internal pull up
#define BUTTON_YELLOW D1 // gpio 5, internal pull up
#define BUTTON_GREEN D2  // gpio 4, internal pull up

LED display connects to VCC (3.3v) Ground, these pins:

#define LED_CLK_PIN D5
#define LED_DATA_PIN D6
#define LED_CS_PIN D7


