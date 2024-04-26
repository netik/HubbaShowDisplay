#pragma once
#include <Arduino.h>

// we're using NODE_DEBUG here instead of DEBUG as that 
// triggers a lot of problems in the Apple Midi library :()
#define NODE_DEBUG 1

#if NODE_DEBUG
#define DBG_SETUP(...) Serial.begin(__VA_ARGS__)
#define DBG_PRINT(...) Serial.print(__VA_ARGS__)
#define DBG_WRITE(...) Serial.write(__VA_ARGS__)
#define DBG(...) Serial.println(__VA_ARGS__)
#else
#define DBG_SETUP(...)
#define DBG_PRINT(...)
#define DBG_WRITE(...)
#define DBG(...)
#endif

#define AM_DBG_SETUP(...)
#define AM_DBG_PLAIN(...)
#define AM_DBG(...)
