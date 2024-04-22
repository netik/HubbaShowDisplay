#include <Arduino.h>

#ifndef _SWITCHHANDLER_H_
#define _SWITCHHANDLER_H_

extern void button1Handler(uint8_t pin, uint8_t event, uint8_t count, uint16_t length);
extern void button2Handler(uint8_t pin, uint8_t event, uint8_t count, uint16_t length);
extern void button3Handler(uint8_t pin, uint8_t event, uint8_t count, uint16_t length);

#endif
