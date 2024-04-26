#include <IPaddress.h>

#ifndef __LED_H__
#define __LED_H__

extern void ledInit();
extern void ledShowIP(IPAddress ip);
extern void displayDHCP();
extern void ledShowString(const char*);

#endif