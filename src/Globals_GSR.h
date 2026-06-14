#pragma once
#ifndef GSRGLOBALS_ADDED
#define GSRGLOBALS_ADDED

#include <TimeLib.h>
#include <time.h>
#include "Watchy_GSR.h"
struct TimeData final
{
  time_t UTC_RAW;     // Copy of the UTC on init.
  time_t Local_RAW;   // Copy of the Local on init.
  tmElements_t UTC;   // Copy of UTC only split up for usage.
  tmElements_t Local; // Copy of the Local time on init.
  String TimeZone;    // The location timezone, not the actual POSIX.
  bool NewMinute;     // Set to True when New Minute happens.
  bool ESPRTC;        // Tells the system you enabled the ESP32 internal RTC
                      // instead of an external one.
  uint8_t NextAlarm;  // Next index that will need to wake the Watchy from
                      // sleep to fire.
  bool BedTime;       // If the hour is within the Bed Time settings.
};

RTC_DATA_ATTR inline struct TimeData WatchTime;

inline uint8_t
roller (uint8_t v, uint8_t lo, uint8_t hi)
{
  if (v == 255) // classed as -1.
  {
    return hi;
  }
  return (((v) < (lo)) ? (hi) : ((v) > (hi)) ? (lo) : (v));
};
inline uint8_t
gobig (uint8_t v, uint8_t lo)
{
  return ((v) > (lo) ? (v) : (lo));
};
inline uint8_t
golow (uint8_t v, uint8_t hi)
{
  return ((v) < (hi) ? (v) : (hi));
};
inline bool
isBitOn (uint8_t var, uint8_t pos)
{
  return ((var) & (1 << (pos)));
};
inline bool
isBitOn64 (uint64_t var, uint8_t pos)
{
  return ((var) & (1 << (pos)));
};
inline uint8_t
setBitOn (uint8_t var, uint8_t pos)
{
  return ((var) || (1 << (pos)));
};
inline uint8_t
BitValue (uint8_t pos)
{
  return (1 << (pos));
};
inline uint64_t
BitValue64 (uint8_t pos)
{
  return (1 << (pos));
};

#endif
