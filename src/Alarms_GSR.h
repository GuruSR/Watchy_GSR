#pragma once
// Alarms & Sound class for Watchy GSR.
// ALARM flags.
#ifndef GSRALARM_REPEAT
#define GSRALARM_REPEAT 128
#define GSRALARM_ACTIVE 256
#define GSRALARM_NOTRIGGER 511
#define GSRALARM_TRIGGERED 512
#define GSRALARM_DAYS 127
#define GSRALARM_ALL 1023

// Defines for CallBack
#define ALARMSGSR_REQ_UTCUPDATE 1
#define ALARMSGSR_REQ_SECONDS 2
#define ALARMSGSR_REQ_SHOWING 3
#define ALARMSGSR_REQ_DISPUPDATE 4
#define ALARMSGSR_REQ_OTA 5
#define ALARMSGSR_SET_DARKNESS 6
#define ALARMSGSR_REQ_DISPWAKE 7
#define ALARMSGSR_REQ_MENUITEM 8

// Constants
const int AlarmVBs[]
    = { 0x01FE, 0x00CC, 0x01B6, 0x014A, 0x0182, 0x0192, 0x018A, 0x01AA };
const float Reduce[5] = { 1.0, 0.8, 0.6, 0.4, 0.2 };
const uint8_t maxafLength = 10;
const uint8_t maxtdLength = 33;
const uint8_t timerdpause = 2;
const uint8_t alarmPaws = 5;
const uint8_t alarmMS = 30;
const uint8_t timerSecs = 10;

struct Countdown final
{
  bool Active;
  bool Repeats;
  bool Repeating;
  uint8_t Hours;    // Converted to work the same as the CountUp so seconds
                    // can be done.
  uint8_t Mins;
  uint8_t Secs;
  uint8_t MaxHours;
  uint8_t MaxMins;
  uint8_t MaxSecs;
  uint32_t MaxTime; // MaxHours*3600+MaxMins*60+MaxSecs.
  uint8_t Tone;     // 10 to start.
  uint8_t ToneLeft; // 30 to start.
  uint8_t MaxTones; // 0-4 (20-80%) tone duration reduction.
  time_t LastUTC;
  time_t StopAt;
};

struct AlarmID final
{
  uint8_t Hour;
  uint8_t Minutes;
  uint16_t Active; // Bits 0-6 (days of week), 7 Repeat, 8 Active, 9 Tripped
                   // (meaning if it isn't repeating, don't fire).
  uint8_t Times;   // Set to 10 to start, ends when zero.
  uint8_t Playing; // Means the alarm tripped and it is to be played (goes to 0
                   // when it finishes).
  uint8_t Repeats; // 0-4 (20-80%) reduction in repetitions.
  bool Cancelled;  // Alarm was cancelled.
  bool EditMode;   // Means the alarm is being altered by either the user or
                   // code, it won't fire if this is on.
  uint16_t Pattern;
};

class AlarmsGSR
{
public:
  AlarmsGSR ();
  static void SoundBegin ();
  void CalculateTones ();
  void StartCD ();
  void StopCD ();
  void setEditMode (uint8_t i, bool bEdit = false);
  uint8_t getHour (uint8_t i);
  void setHour (uint8_t i, uint8_t newHour);
  uint8_t getMinutes (uint8_t i);
  void setMinutes (uint8_t i, uint8_t newMinutes);
  uint16_t getActive (uint8_t i);
  void setActive (uint8_t i, uint16_t newActive);
  uint8_t getTimes (uint8_t i);
  void setTimes (uint8_t i, uint8_t newTimes);
  uint8_t getPlaying (uint8_t i);
  void setPlaying (uint8_t i, uint8_t newPlaying);
  uint8_t getRepeats (uint8_t i);
  void setRepeats (uint8_t i, uint8_t newRepeats);
  uint16_t getPlayPattern (uint8_t i);
  void setPlayPattern (uint8_t i, uint16_t pattern);
  bool getCancelled (uint8_t i);
  void setCancelled (uint8_t i, bool newCancelled = false);
  bool getTimerDownActive ();
  void setTimerDownActive (bool inActive = false);
  bool getTimerDownRepeats ();
  void setTimerDownRepeats (bool inRepeats = false);
  uint8_t getTimerDownHours ();
  uint8_t getTimerDownMinutes ();
  uint8_t getTimerDownSeconds ();
  uint8_t getTimerDownMaxHours ();
  void setTimerDownMaxHours (uint8_t inHours);
  uint8_t getTimerDownMaxMinutes ();
  void setTimerDownMaxMinutes (uint8_t inMins);
  uint8_t getTimerDownMaxSeconds ();
  void setTimerDownMaxSeconds (uint8_t inSecs);
  uint8_t getTimerDownToneLeft ();
  void setTimerDownToneLeft (uint8_t inLeft);
  uint8_t getTimerDownMaxTones ();
  void setTimerDownMaxTones (uint8_t inLeft);
  time_t getTimerDownStopAt ();
  bool firing ();
  uint8_t alarmsGoing ();
  bool soundsFiring ();
  static void updateSound ();
  void endSounds ();
  void doHaptic ();
  static void makePulses (uint16_t binaryPlay);
  void init (void (*objIn) (), uint8_t inPin);
  uint8_t getCallbackOpcode ();
  bool getCallbackBoolData ();
  uint8_t getCallbackByteData ();
  unsigned long getCallbackLongData ();
  void setCallbackBoolData (bool newData);
  void setCallbackByteData (uint8_t newData);
  void setCallbackLongData (unsigned long newData);
  static void SoundAlarms (void *parameter);

private:
  static void VibeTo (bool Mode);
  void CheckAlarm (int I);
  void UpdateTimerDown ();
  static uint8_t getToneTimes (uint8_t ToneIndex);
  static void requestUTCUpdate ();
  static uint8_t requestUTCSeconds ();
  bool requestShowing ();
  static void requestRefreshScreen ();
  static bool requestOTAStatus ();
  static void setDarknessLast (unsigned long newDark);
  void requestDisplayWake ();
  uint8_t requestMenuItem ();
  static void updateToneTimes ();

private:
  bool Alarming; // Means an alarm has triggered.
};

// extern
RTC_DATA_ATTR static void (*serviceCallBack) ();

#endif
