#pragma once

#ifdef SerialMon

static void AM_DBG_SETUP(unsigned long baud) {
  SerialMon.begin(baud);
  while (!SerialMon);
}

template <typename T>
static void AM_DBG_PLAIN(T last) {
  SerialMon.println(last);
}

template <typename T, typename... Args>
static void AM_DBG_PLAIN(T head, Args... tail) {
  SerialMon.print(head);
  SerialMon.print(' ');
  AM_DBG_PLAIN(tail...);
}

template <typename... Args>
static void AM_DBG(Args... args) {
  AM_DBG_PLAIN(args...);
}

#else
#define AM_DBG_SETUP(...)
#define AM_DBG_PLAIN(...)
#define AM_DBG(...)
#endif
