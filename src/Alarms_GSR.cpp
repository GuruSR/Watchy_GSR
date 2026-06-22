#include "esp32-hal-log.h"
#include <cstdint>
#include <Arduino.h>
#include "Globals_GSR.h"
#include "Defines_GSR.h"
#include "Alarms_GSR.h"
#include <cmath>

RTC_DATA_ATTR uint8_t pwmPin;
RTC_DATA_ATTR AlarmID gsrAlarm[8];
RTC_DATA_ATTR Countdown TimerDown;

bool VibeMode;    // Vibe Motor is On=True/Off=False, used for the Haptic and
                  // Alarms.
bool goHaptic;    // True if it is to run.
bool makeNoise;   // Means noise has to be made.
bool serviceBool; // Used for return of a bool from a callback.
uint8_t serviceRequest;    // Holds the DEFINE for the service requested.
uint8_t serviceByte;       // Used for return of a byte from a callback.
unsigned long serviceLong; // Used for return of a long from a callback.
unsigned long serviceUTC;  // UTC Synced to start of current minute.
TaskHandle_t SoundHandle;
BaseType_t SoundRet;
uint16_t pulsePlay; // Play tone (binary)
uint8_t pulsePos;   // What bit it is playing.
uint8_t pulseBit;   // Bits to stop at.
bool pulsing;

AlarmsGSR::AlarmsGSR (){}; // Constructor

/* Public functions */

void
AlarmsGSR::StartCD ()
{
  TimerDown.Secs = TimerDown.MaxSecs;
  TimerDown.Mins = TimerDown.MaxMins;
  TimerDown.Hours = TimerDown.MaxHours;
  AlarmsGSR::requestUTCUpdate ();
  TimerDown.LastUTC = WatchTime.UTC_RAW;
  TimerDown.StopAt = TimerDown.LastUTC + (3600 * TimerDown.Hours)
                     + (60 * TimerDown.Mins) + TimerDown.Secs;
  TimerDown.Repeating = TimerDown.Repeats;
  TimerDown.Active = true;
}

void
AlarmsGSR::StopCD ()
{
  if (TimerDown.ToneLeft > 0)
    {
      TimerDown.ToneLeft = 1;
      TimerDown.Tone = 1;
    }
  TimerDown.Active = false;
  TimerDown.Repeating = false;
}

void
AlarmsGSR::setEditMode (uint8_t i, bool bEdit)
{
  if (i < 8)
    gsrAlarm[i].EditMode = bEdit;
}

uint8_t
AlarmsGSR::getHour (uint8_t i)
{
  if (i < 8)
    return gsrAlarm[i].Hour;
  return 0;
}
void
AlarmsGSR::setHour (uint8_t i, uint8_t newHour)
{
  if (i < 8 && newHour < 24 && gsrAlarm[i].EditMode)
    gsrAlarm[i].Hour = newHour;
}

uint8_t
AlarmsGSR::getMinutes (uint8_t i)
{
  if (i < 8)
    return gsrAlarm[i].Minutes;
  return 0;
}
void
AlarmsGSR::setMinutes (uint8_t i, uint8_t newMinutes)
{
  if (i < 8 && newMinutes < 60 && gsrAlarm[i].EditMode)
    gsrAlarm[i].Minutes = newMinutes;
}

uint16_t
AlarmsGSR::getActive (uint8_t i)
{
  if (i < 8)
    return gsrAlarm[i].Active;
  return 0;
}
void
AlarmsGSR::setActive (uint8_t i, uint16_t newActive)
{
  if (i < 8 && gsrAlarm[i].EditMode)
    gsrAlarm[i].Active = newActive & GSRALARM_ALL;
}

uint8_t
AlarmsGSR::getTimes (uint8_t i)
{
  if (i < 8)
    return gsrAlarm[i].Times;
  return 0;
}
void
AlarmsGSR::setTimes (uint8_t i, uint8_t newTimes)
{
  if (i < 8 && newTimes < 127 && gsrAlarm[i].EditMode)
    gsrAlarm[i].Times = newTimes;
}

uint8_t
AlarmsGSR::getPlaying (uint8_t i)
{
  if (i < 8)
    return gsrAlarm[i].Playing;
  return 0;
}
void
AlarmsGSR::setPlaying (uint8_t i, uint8_t newPlaying)
{
  if (i < 8 && newPlaying < 127 && gsrAlarm[i].EditMode)
    gsrAlarm[i].Playing = newPlaying;
}

uint8_t
AlarmsGSR::getRepeats (uint8_t i)
{
  if (i < 8)
    return gsrAlarm[i].Repeats;
  return 0;
}
void
AlarmsGSR::setRepeats (uint8_t i, uint8_t newRepeats)
{
  if (i < 8 && newRepeats < 127 && gsrAlarm[i].EditMode)
    gsrAlarm[i].Repeats = newRepeats;
}

uint16_t
getPlayPattern (uint8_t i)
{
  if (i < 8)
    return gsrAlarm[i].Pattern & 1022;
  return 0;
}
void
setPlayPattern (uint8_t i, uint16_t pattern)
{
  if (i < 8)
    gsrAlarm[i].Pattern = (pattern & 1022);
}

bool
AlarmsGSR::getCancelled (uint8_t i)
{
  if (i < 8)
    return gsrAlarm[i].Cancelled;
  return 0;
}
void
AlarmsGSR::setCancelled (uint8_t i, bool newCancelled)
{
  if (i < 8 && gsrAlarm[i].EditMode)
    gsrAlarm[i].Cancelled = newCancelled;
}

bool
AlarmsGSR::getTimerDownActive ()
{
  return TimerDown.Active;
}
void
AlarmsGSR::setTimerDownActive (bool inActive)
{
  TimerDown.Active = inActive;
}

bool
AlarmsGSR::getTimerDownRepeats ()
{
  return TimerDown.Repeats;
}
void
AlarmsGSR::setTimerDownRepeats (bool inRepeats)
{
  TimerDown.Repeats = inRepeats;
}

uint8_t
AlarmsGSR::getTimerDownHours ()
{
  return TimerDown.Hours;
}

uint8_t
AlarmsGSR::getTimerDownMinutes ()
{
  return TimerDown.Mins;
}

uint8_t
AlarmsGSR::getTimerDownSeconds ()
{
  return TimerDown.Secs;
}

uint8_t
AlarmsGSR::getTimerDownMaxHours ()
{
  return TimerDown.MaxHours;
}
void
AlarmsGSR::setTimerDownMaxHours (uint8_t inHours)
{
  if (inHours < 24)
    TimerDown.MaxHours = inHours;
}

uint8_t
AlarmsGSR::getTimerDownMaxMinutes ()
{
  return TimerDown.MaxMins;
}
void
AlarmsGSR::setTimerDownMaxMinutes (uint8_t inMins)
{
  if (inMins < 60)
    TimerDown.MaxMins = inMins;
}

uint8_t
AlarmsGSR::getTimerDownMaxSeconds ()
{
  return TimerDown.MaxSecs;
}
void
AlarmsGSR::setTimerDownMaxSeconds (uint8_t inSecs)
{
  if (inSecs < 60)
    TimerDown.MaxSecs = inSecs;
}

uint8_t
AlarmsGSR::getTimerDownToneLeft ()
{
  return TimerDown.ToneLeft;
}

void
AlarmsGSR::setTimerDownToneLeft (uint8_t inLeft)
{
  if (inLeft < 5)
    TimerDown.ToneLeft = inLeft;
}

uint8_t
AlarmsGSR::getTimerDownMaxTones ()
{
  return TimerDown.MaxTones;
}

void
AlarmsGSR::setTimerDownMaxTones (uint8_t inLeft)
{
  if (inLeft < 5)
    TimerDown.MaxTones = inLeft;
}

time_t
AlarmsGSR::getTimerDownStopAt ()
{
  return TimerDown.StopAt;
}

bool
AlarmsGSR::firing ()
{
  return Alarming || VibeMode || AlarmsGSR::getTimerDownToneLeft ();
}

uint8_t
AlarmsGSR::alarmsGoing ()
{
  uint8_t i = 0, c = 0;
  for (i = 0; i < 8; i++)
    {
      if (gsrAlarm[i].Times > 0 || gsrAlarm[i].Playing > 0)
        c++;
    }
  return c;
}

bool
AlarmsGSR::soundsFiring ()
{
  AlarmsGSR::updateSound ();
  return (SoundHandle != NULL || VibeMode);
}

void
AlarmsGSR::endSounds ()
{
  if (SoundHandle != NULL)
    {
      vTaskDelete (SoundHandle);
      SoundHandle = NULL;
    }
  if (VibeMode)
    {
      AlarmsGSR::VibeTo (false);
    }
}

void
AlarmsGSR::doHaptic ()
{
  goHaptic = true;
  makeNoise = true;
}

void
AlarmsGSR::makePulses (uint16_t binaryPlay)
{
  bool b = true;
  uint8_t i;
  uint16_t bt = (binaryPlay & 32767);
  for (i = 15; i > 0 && b; i--)
    {
      b = !((1<<i) & bt);
    }
  if (i < 15 && !goHaptic)
    {
      pulsePlay = bt;
      pulsing = true;
      if (i < 15)
        {
          i++;
        }
      pulseBit = i;
      pulsePos = 0;
      if (pulseBit)
      {
        makeNoise = true;
        AlarmsGSR::updateSound ();
      }
    }
}

/* Class code */

void
AlarmsGSR::init (void (*funcIn) (), uint8_t inPin)
{
  byte i;
  serviceCallBack = funcIn;
  pwmPin = inPin;
  for (i = 0; i < 8; i++)
    {
      gsrAlarm[i].Hour = 0;
      gsrAlarm[i].Minutes = 0;
      gsrAlarm[i].Active = 0;
      gsrAlarm[i].Times = 0;
      gsrAlarm[i].Playing = 0;
      gsrAlarm[i].Repeats = 0;
      gsrAlarm[i].Cancelled = false;
      gsrAlarm[i].Pattern = AlarmVBs[i];
    }
  SoundHandle = NULL;
  pulsing = true;
  pulsePlay = 23514;
  pulseBit = 15;
  pulsePos = 0;
  TimerDown.MaxSecs = 0;
  TimerDown.MaxMins = 0;
  TimerDown.MaxHours = 0;
  TimerDown.Secs = 0;
  TimerDown.Mins = 0;
  TimerDown.Hours = 0;
  TimerDown.Active = false;
  TimerDown.Repeats = false;
  TimerDown.MaxTime = 0;
  TimerDown.Tone = 0;
  TimerDown.ToneLeft = 0;
  TimerDown.MaxTones = 0;
  Alarming = false;
  VibeMode = false;
  makeNoise = true;
  AlarmsGSR::updateSound ();
}

void
AlarmsGSR::VibeTo (bool Mode)
{
  if (Mode != VibeMode)
    {
      if (Mode)
        {
          pinMode (pwmPin, OUTPUT);
          digitalWrite (pwmPin, true);
        }
      else
        {
          digitalWrite (pwmPin, false);
          pinMode (pwmPin, INPUT);
        }
      VibeMode = Mode;
    }
}

void
AlarmsGSR::SoundBegin ()
{
  if (SoundHandle == NULL)
    {
      SoundRet = xTaskCreate (AlarmsGSR::SoundAlarms, "AlarmsGSR_Alarming",
                              2048, NULL, (tskIDLE_PRIORITY + 1), &SoundHandle);
    }
}

void
AlarmsGSR::CheckAlarm (int I)
{
  uint16_t B = 0, bB = 0;
  bool bA = false;
  if (gsrAlarm[I].EditMode)
    {
      return;
    }
  bB = (1<<WatchTime.Local.Wday);
  B = (GSRALARM_ACTIVE | bB);
  bA = (gsrAlarm[I].Hour == WatchTime.Local.Hour
        && gsrAlarm[I].Minutes == WatchTime.Local.Minute);
  if (!bA && gsrAlarm[I].Times == 0
      && (gsrAlarm[I].Active & GSRALARM_TRIGGERED) != 0)
    {
      gsrAlarm[I].Active &= GSRALARM_NOTRIGGER;
    }
  else if ((gsrAlarm[I].Active & B) == B
           && (gsrAlarm[I].Active & GSRALARM_TRIGGERED) == 0)
    { // Active and Active Day.
      // Check alarm listed to see if it is earlier than the one slated.
      if (gsrAlarm[I].Hour == WatchTime.Local.Hour
          && gsrAlarm[I].Minutes > WatchTime.Local.Minute && !bA)
        {
          if (WatchTime.NextAlarm == 99)
            WatchTime.NextAlarm = I;
          else if (gsrAlarm[I].Minutes < gsrAlarm[WatchTime.NextAlarm].Minutes)
            WatchTime.NextAlarm = I;
        }
      if (bA && gsrAlarm[I].Times == 0)
        {
          gsrAlarm[I].Times = 255;
          gsrAlarm[I].Playing = maxafLength;
          AlarmsGSR::setDarknessLast (millis ());
          makeNoise = true;
          AlarmsGSR::requestDisplayWake (); // Fix Display.
          gsrAlarm[I].Active |= GSRALARM_TRIGGERED;
          if ((gsrAlarm[I].Active & GSRALARM_REPEAT) == 0)
            {
              gsrAlarm[I].Active &= (GSRALARM_ALL - bB);
              if ((gsrAlarm[I].Active & GSRALARM_DAYS) == 0)
                {
                  gsrAlarm[I].Active
                      ^= GSRALARM_ACTIVE; // Turn it off, not repeating.
                }
            }
        }
    }
}

// Counts the active (255) alarms/timers and after 3, sets them to lower values.
void
AlarmsGSR::CalculateTones ()
{
  uint8_t i, t;
  uint16_t c;
  WatchTime.NextAlarm = 99;
  for (i = 0; i < 8; i++)
    {
      CheckAlarm (i);
      if (gsrAlarm[i].Times == 255)
        gsrAlarm[i].Times = maxafLength * Reduce[gsrAlarm[i].Repeats];
    }
  UpdateTimerDown ();
  if (TimerDown.ToneLeft == 255)
    {
      t = timerSecs * Reduce[TimerDown.MaxTones];
      c = (3600 * TimerDown.Hours)
        + (60 * TimerDown.Mins) + TimerDown.Secs;
      if (c < t && TimerDown.Repeating)
        {
          if (c > 1)
            {
              c--;
            }
        }
      else
        c = t;
      i = c;
      TimerDown.ToneLeft = i;
    }
  Alarming = AlarmsGSR::alarmsGoing ();
  AlarmsGSR::updateSound ();
}

uint8_t
AlarmsGSR::getToneTimes (uint8_t ToneIndex)
{
  AlarmsGSR::updateSound ();
  if (ToneIndex > 7)
    return TimerDown.ToneLeft;
  return gsrAlarm[ToneIndex].Times;
}

void
AlarmsGSR::UpdateTimerDown ()
{
  uint16_t T;
  bool B
      = (TimerDown.Active && AlarmsGSR::requestMenuItem () == GSR_MENU_TIMEDN);
  if (TimerDown.Active)
    {
      AlarmsGSR::requestUTCUpdate ();
      T = (TimerDown.StopAt - WatchTime.UTC_RAW);
      if (T > -1)
        {
          TimerDown.Hours = (T / 3600);
          T = T - (TimerDown.Hours * 3600);
          TimerDown.Mins = (T / 60);
          if (TimerDown.Secs != (T % 60) && B && AlarmsGSR::requestShowing ())
            AlarmsGSR::requestRefreshScreen ();
          TimerDown.Secs = (T % 60);
        }
      else
        {
          TimerDown.Hours = 0;
          if (TimerDown.Secs != 0 && B && AlarmsGSR::requestShowing ())
            AlarmsGSR::requestRefreshScreen ();
          TimerDown.Mins = 0;
          TimerDown.Secs = 0;
        }
      if (TimerDown.Hours == 0 && TimerDown.Mins == 0 && TimerDown.Secs == 0)
        {
          TimerDown.Tone = maxtdLength;
          TimerDown.ToneLeft = 255;
          TimerDown.Active = false;
          makeNoise = true;
          if (TimerDown.Repeating)
            StartCD ();
          AlarmsGSR::setDarknessLast (millis ());
          AlarmsGSR::requestDisplayWake (); // Fix Display.
        }
    }
}

void
AlarmsGSR::updateSound ()
{
  if (makeNoise)
    {
      // AlarmsGSR::
      SoundBegin ();
    }
  makeNoise = false;
}

/* Private Callback Functions */

void
AlarmsGSR::requestUTCUpdate ()
{
  serviceRequest = (uint8_t)ALARMSGSR_REQ_UTCUPDATE;
  if (serviceCallBack)
    serviceCallBack ();
}

uint8_t
AlarmsGSR::requestUTCSeconds ()
{
  serviceByte = 0;
  serviceRequest = (uint8_t)ALARMSGSR_REQ_SECONDS;
  if (serviceCallBack)
    serviceCallBack ();
  return serviceByte;
}

bool
AlarmsGSR::requestShowing ()
{
  serviceBool = false;
  serviceRequest = (uint8_t)ALARMSGSR_REQ_SHOWING;
  if (serviceCallBack)
    serviceCallBack ();
  return serviceBool;
}

void
AlarmsGSR::requestRefreshScreen ()
{
  serviceRequest = (uint8_t)ALARMSGSR_REQ_DISPUPDATE;
  if (serviceCallBack)
    serviceCallBack ();
}

bool
AlarmsGSR::requestOTAStatus ()
{
  serviceBool = false;
  serviceRequest = (uint8_t)ALARMSGSR_REQ_OTA;
  if (serviceCallBack)
    serviceCallBack ();
  return serviceBool;
}

void
AlarmsGSR::setDarknessLast (unsigned long newDark)
{
  serviceRequest = (uint8_t)ALARMSGSR_SET_DARKNESS;
  serviceLong = newDark;
  if (serviceCallBack)
    serviceCallBack ();
}

void
AlarmsGSR::requestDisplayWake ()
{
  serviceRequest = (uint8_t)ALARMSGSR_REQ_DISPWAKE;
  if (serviceCallBack)
    serviceCallBack ();
}

uint8_t
AlarmsGSR::requestMenuItem ()
{
  serviceByte = 255; // Error.
  serviceRequest = (uint8_t)ALARMSGSR_REQ_MENUITEM;
  if (serviceCallBack)
    serviceCallBack ();
  return serviceByte;
}

/* Public Callback Functions */

uint8_t
AlarmsGSR::getCallbackOpcode ()
{
  return serviceRequest;
}

bool
AlarmsGSR::getCallbackBoolData ()
{
  return serviceBool;
}
uint8_t
AlarmsGSR::getCallbackByteData ()
{
  return serviceByte;
}
unsigned long
AlarmsGSR::getCallbackLongData ()
{
  return serviceLong;
}

void
AlarmsGSR::setCallbackBoolData (bool newData)
{
  serviceBool = newData;
}
void
AlarmsGSR::setCallbackByteData (uint8_t newData)
{
  serviceByte = newData;
}
void
AlarmsGSR::setCallbackLongData (unsigned long newData)
{
  serviceLong = newData;
}

/* Functions for tasks */

void
AlarmsGSR::SoundAlarms (void *parameter)
{
  bool WaitForNext, Pulse, Used, goDisp, isActive;
  unsigned long M;
  uint8_t AlarmIndex = 0, i = 0, Alarms = 0, aSleep = 0;
  VibeMode = false;
  Pulse = false;
  goDisp = false;
  Used = true;
  vTaskDelay (2 / portTICK_PERIOD_MS);
  AlarmsGSR::requestUTCUpdate ();
  i = AlarmsGSR::requestUTCSeconds ();
  serviceUTC = millis() - (i * 1000UL);
  WaitForNext = true;
  isActive = true;
  // Here, do the alarm buzzings by way of which one is running.
  while (isActive)
    {
      M = millis () + alarmMS; // Amount of time extra.
      Used = false;
      isActive = false;
      AlarmsGSR::updateToneTimes ();
      //!AlarmsGSR::requestOTAStatus () && 
      if (aSleep == 0)
        {
          if (WaitForNext)
            {
              WaitForNext = false;
              AlarmIndex = roller (AlarmIndex + 1, 0, 8);
              for (i = 0; i < 8
                && gsrAlarm[AlarmIndex].Times
                   + gsrAlarm[AlarmIndex].Playing == 0; i++)
                {
                  AlarmIndex = roller (AlarmIndex + 1, 0, 8);
                }
            }
          if (AlarmIndex == 8)
            {
              if (TimerDown.ToneLeft > 0)
                {
                  if (TimerDown.Tone == 0)
                    {
                      WaitForNext = true;
                      TimerDown.ToneLeft--;
                      if (TimerDown.ToneLeft > 0)
                        {
                          //Used = true;
                          TimerDown.Tone = maxtdLength;
                        }
                      else
                        goDisp = true;
                    }
                  if (TimerDown.Tone > 0)
                    {
                      Pulse = (((TimerDown.Tone / 2) & 1) != 0
                        && TimerDown.Tone > timerdpause);
                      TimerDown.Tone--;
                      if (TimerDown.Tone == 0 && TimerDown.ToneLeft == 0)
                        goDisp = true;
                      Used = true;
                      //if (!Pulse && TimerDown.Tone > 0)
                      //  TimerDown.Tone--;
                      AlarmsGSR::VibeTo (Pulse); // Turns Vibe on or off
                                                 // depending on bit state.
                      if (Pulse)
                        {
                          AlarmsGSR::setDarknessLast (M);
                        }
                    }
                }
              else
                WaitForNext = true;
            }
          else
            {
              if (!aSleep)
                {
                  if (gsrAlarm[AlarmIndex].Cancelled)
                    {
                      gsrAlarm[AlarmIndex].Times = 0;
                      gsrAlarm[AlarmIndex].Playing = 0;
                      gsrAlarm[AlarmIndex].Cancelled = false;
                      AlarmsGSR::VibeTo (false);
                      goDisp = true;
                    }
                  else
                    {
                      if (gsrAlarm[AlarmIndex].Playing > 0)
                        {
                          gsrAlarm[AlarmIndex].Playing--;
                          Used = true;
                          aSleep = alarmPaws;
                          if (!gsrAlarm[AlarmIndex].Pattern)
                            {
                              gsrAlarm[AlarmIndex].Pattern = AlarmVBs[AlarmIndex];
                            }
                          Pulse = ((gsrAlarm[AlarmIndex].Pattern
                                    & 1<<gsrAlarm[AlarmIndex].Playing) != 0);
                          AlarmsGSR::VibeTo (Pulse); // Turns Vibe on or off
                                                    // depending on bit state.
                          if (Pulse)
                            {
                              AlarmsGSR::setDarknessLast (M);
                              goHaptic = false;
                            }
                        }
                      if (gsrAlarm[AlarmIndex].Playing == 0)
                        {
                          WaitForNext = true;
                          if (gsrAlarm[AlarmIndex].Times > 0)
                            {
                              gsrAlarm[AlarmIndex].Times--;
                              if (gsrAlarm[AlarmIndex].Times > 0)
                                {
                                  AlarmsGSR::setDarknessLast (M);
                                  gsrAlarm[AlarmIndex].Playing = maxafLength;
                                  Used = true;
                                }
                              else
                                {
                                  goDisp = true;
                                }
                            }
                          else
                            {
                              goDisp = true;
                            }
                        }
                    }
                }
            }
        }
      if (Used)
        {
          goHaptic = false;
          pulsePlay = 0;
          pulsePos = 0;
          pulseBit = 0;
          pulsing = false;
        }
      else if (pulsing && aSleep == 0)
        {
          if (pulsePos < pulseBit)
            {
              Pulse = (pulsePlay & (1<<pulsePos));
              aSleep = alarmPaws;
              AlarmsGSR::VibeTo (Pulse);
              goHaptic = false;
              pulsePos++;
            } else {
              pulsing = false;
              aSleep = alarmPaws;
              AlarmsGSR::VibeTo (pulsing);
            }
        }
      if (goDisp)
        {
          goDisp = false;
          AlarmsGSR::requestRefreshScreen ();
        }
      if (aSleep)
        aSleep--;
      isActive = (aSleep > 0 || Used);
      if (!isActive)
        {
          isActive = TimerDown.ToneLeft > 0 || pulsing;
          for (i = 0; i < 8 && !isActive; i++)
            {
              if (gsrAlarm[i].Times > 0 || gsrAlarm[i].Playing > 0)
                {
                  isActive = true;
                }
            }
        }
      if (millis () < M)
        {
          if (goHaptic)
            {
              AlarmsGSR::VibeTo (goHaptic);
              goHaptic = false;
            }
          M -= millis ();
          vTaskDelay (M / portTICK_PERIOD_MS);
        }
      goHaptic = false;
    }
  if (VibeMode)
    {
      AlarmsGSR::VibeTo (false);
    }
  SoundHandle = NULL;
  vTaskDelete (NULL);
}

void
AlarmsGSR::updateToneTimes ()
{
  uint8_t i = 0, t = 0, h = 9, k = 0;
  uint16_t c = 0, s = 0, f = 0;
  // i=index, t=temp times, h=highest alarm times index, c=times count.
  // s=alarm pulses once, d=countdown timer left, f=temp value.
  // alarms last xx seconds, timer lasts xx seconds.
  unsigned long o = 600 - ((millis () - serviceUTC) / 100UL);
  unsigned long d = (3600 * TimerDown.Hours)
                    + (60 * TimerDown.Mins) + TimerDown.Secs;
  if (o > 2)
    {
      o -= 2;
    }
  if (o > 2)
    {
      o -= 2;
    }
  f = ceil ((alarmMS * (1 + alarmPaws) * maxafLength) / 1000); // Pulse time.
  for (i = 0; i < 8; i++)
    {
      t = gsrAlarm[i].Times + (gsrAlarm[i].Playing > 0 ? 1 : 0);
      if (t > 0)
        {
          if (h == 9)
            h = i;
          else if ((gsrAlarm[h].Times + (gsrAlarm[h].Playing > 0 ? 1 : 0)) < t)
            h = i;
          c += t * (10 * f);
          s += f;
        }
    }
  if (TimerDown.ToneLeft)
    {
      i = TimerDown.ToneLeft;
      k = i + (TimerDown.Tone > 0 ? 1 : 0);
      if (TimerDown.Repeating)
        {
          if (s > d)
            {
              i = 1;
              TimerDown.ToneLeft = i; // Allow only 1.
            }
          if (k >= d && i > 0)
            {
              i--;
              TimerDown.ToneLeft = i;
            }
        }
      if (i * timerSecs > o)
        {
          i = o / timerSecs;
        }
      c += (i * timerSecs); //Seconds for timer pulse.
    }
  if (h != 9)
    {
      if (c > o)  // seconds left.
        {
          t = gsrAlarm[h].Times;
          if (t > 0)
            {
              t--;
              gsrAlarm[h].Times = t;
            }
        }
    }
}