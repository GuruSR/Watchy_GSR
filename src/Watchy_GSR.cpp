#include "Watchy_GSR.h"
/* Watchy_GSR by GuruSR (https://github.com/GuruSR/Watchy_GSR)
 * Version 1.4.8, June 14, 2026 -  See Wiki for Version History.
 *
 * This watch base is a library for the ability to create Watchy firmware
 * watchfaces to your liking.
 *
 *
 * This watch base is designed to specifically work with the entire version
 * line up for Watchy hardware.
 *
 * MIT License
 *
 * Copyright (c) 2026 GuruSR
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

const unsigned char *alarmIcons[16] = { Tone_1,  Tone_2,  Tone_3,  Tone_4,
                                        Tone_5,  Tone_6,  Tone_7,  Tone_8,
                                        Tone_1L, Tone_2L, Tone_3L, Tone_4L,
                                        Tone_5L, Tone_6L, Tone_7L, Tone_8L };

// Specific defines to this Watchy face.
#define GSR_GName "GSR"
#define GSR_GSettings "GSR-Options"
#define GSR_GTZ "GSR-TZ"
#define GSR_STP "Steps"
#define GSR_YSTP "YSteps"
// Protected
RTC_DATA_ATTR struct GSRWireless final
{
  bool Powered;      // Radio is on.
  bool Requested;    // Request WiFi.
  bool Working;      // Working on getting WiFi.
  bool Results;      // Results of WiFi, found an AP?
  bool Prepped;      // Used to denote the WiFi AP ID is being prepped.
  uint8_t Index;     // 10 = built-in, roll backwards to 0.
  uint8_t Requests;  // WiFi Connect requests.
  uint8_t Slow;      // Slows down initial connection to avoid brownouts.
  IPAddress LocalIP; // Address gotten when connection happens.
  struct APInfo
  {
    char APID[33];
    char PASS[65];
    uint8_t TPWRIndex;
  } AP[10];           // Using APID to avoid internal confusion with SSID.
  unsigned long Last; // Used with millis() to maintain sanity.
  bool Tried;         // Tried to connect at least once.
  wifi_power_t TransmitPower;
  wifi_event_id_t WiFiEventID;
} GSRWiFi;
RTC_DATA_ATTR struct CPUWork final
{
  uint32_t Freq;
  bool Locked;
} CPUSet;
RTC_DATA_ATTR struct Stepping final
{
  uint8_t Hour;
  uint8_t Minutes;
  bool Reset;
  uint32_t Yesterday;
  uint32_t Cached; // When a reboot happens,this is added.
  time_t Stored;
} Steps;
RTC_DATA_ATTR struct Optional final
{
  bool TwentyFour; // If the face shows 24 hour or Am/Pm.
  bool LightMode;  // Light/Dark mode.
  bool Feedback;   // Haptic Feedback on buttons.
  bool Border;     // True to set the border to black/white.
  bool Lefty;      // Swaps the buttons to the other side.
  bool Swapped;    // Menu and Back buttons swap ends (vertically).
  bool Orientated; // Set to false to not bother which way the buttons are.
  uint8_t Turbo;   // 0-10 seconds.
  uint8_t MasterRepeats; // Done for ease, will be in the Alarms menu.
  uint8_t SleepStyle; // 0==Disabled, 1==Always, 2==Sleeping, 3==DT, 4==DT Only,
                      // no tilt
  uint8_t SleepMode;  // Turns screen off (black, won't show any screen unless a
                      // button is pressed)
  uint8_t SleepStart; // Hour when you go to bed.
  uint8_t SleepEnd;   // Hour when you wake up.
  uint8_t Performance; // Performance style, "Turbo", "Normal", "Battery Saving"
  bool NeedsSaving; // NVS code to tell it things have been updated, so save to
                    // NVS.
  bool BedTimeOrientation; // Make Buttons only work while Watch is in normal
                           // orientation.
  uint8_t WatchFaceStyle;  // Using the Style values from Defines_GSR.
  uint8_t LanguageID;      // The LanguageID.
  uint8_t Game;
  uint8_t GameCount; // Counts the Games installed.
  bool GameStatus;   // Game needs attention.
  bool NTPAuto;      // Automatically Do NTP start and finish.
  bool ntpAtCharge;  // Automatically do NTP during charging phase, default is
                     // true.
  bool GDEY0154D67;  // True if using the newer GDEY0154D67 screen as apposed to
                     // the GDEH0154D67 screen.
} Options;
RTC_DATA_ATTR struct DesignStyles final
{
  uint8_t Count;
  struct DesignWiFiStyle final
  {
    const unsigned char *WiFiOFF;
    const unsigned char *WiFiON;
    uint8_t width;
    uint8_t height;
    bool useMyWiFi;
  } DesignWiFi[GSR_MaxStyles];     // Hold the WiFi icons setup.
  uint8_t Options[GSR_MaxStyles];  // Contains all the Watchface selections.
  WatchyGSR *AddOn[GSR_MaxStyles]; // Holds the class to call when the Functions
                                   // are present.
  char Style[32 * GSR_MaxStyles];
} WatchStyles;

RTC_DATA_ATTR struct WatchyBootAddOns final
{
  uint8_t Count; // Used at boot time to record Addons to be called.
  WatchyGSR *Inits[GSR_MaxStyles]; // Holds the class to call for "Watch Faces"
                                   // addition.
} BootAddOns;

RTC_DATA_ATTR struct MenuUse final
{
  int8_t Style; // GSR_MENU_INNORMAL or GSR_MENU_INOPTIONS
  int8_t Item;  // What Menu Item is being viewed.
  int8_t
      SubItem; // Used for menus that have sub items, like alarms and Sync Time.
  int8_t SubSubItem; // Used mostly in the alarm to offset choice.
} Menu;

RTC_DATA_ATTR int GuiMode;
RTC_DATA_ATTR String
    WatchyStatus; // Used for the indicator in the bottom left, so when it
                  // changes, it asks for a screen refresh, if not, it doesn't.
RTC_DATA_ATTR String
    WatchyOStatus; // Original status kept when WiFi or BT until disconnected.
RTC_DATA_ATTR int BasicWatchStyles;
RTC_DATA_ATTR bool DefaultWatchStyles; // States that the original 2 Watch
                                       // Styles are to be added.
RTC_DATA_ATTR float HWVer;
RTC_DATA_ATTR volatile bool KeyIRQ; // Used to stop repeats.
RTC_DATA_ATTR uint8_t GSR_MENU_PIN = 0;
RTC_DATA_ATTR uint8_t GSR_BACK_PIN = 0;
RTC_DATA_ATTR uint8_t GSR_UP_PIN = 0;
RTC_DATA_ATTR uint8_t GSR_DOWN_PIN = 0;
RTC_DATA_ATTR uint8_t GSR_PIN_ADC = 0;
RTC_DATA_ATTR uint8_t GSR_PIN_STAT = 255;
RTC_DATA_ATTR uint8_t GSR_PIN_SCL = 255;
RTC_DATA_ATTR uint8_t GSR_PIN_SDA = 255;
RTC_DATA_ATTR uint8_t GSR_PIN_ACC_INT1 = 0;
RTC_DATA_ATTR uint8_t GSR_PIN_ACC_INT2 = 0;
RTC_DATA_ATTR uint8_t GSR_PIN_VIB_PWM = 0;
RTC_DATA_ATTR uint8_t GSR_PIN_USB_DET = 255;
RTC_DATA_ATTR uint8_t GSR_PIN_SCK = 255;
RTC_DATA_ATTR uint8_t GSR_PIN_MISO = 255;
RTC_DATA_ATTR uint8_t GSR_PIN_MOSI = 255;
RTC_DATA_ATTR uint8_t GSR_PIN_SS = 255;
RTC_DATA_ATTR uint8_t GSR_PIN_RTC = 255;
RTC_DATA_ATTR uint64_t GSR_BTN_MASK;
RTC_DATA_ATTR uint64_t GSR_MENU_MASK;
RTC_DATA_ATTR uint64_t GSR_BACK_MASK;
RTC_DATA_ATTR uint64_t GSR_UP_MASK;
RTC_DATA_ATTR uint64_t GSR_DOWN_MASK;
RTC_DATA_ATTR uint64_t GSR_MASK_ACC_INT;
RTC_DATA_ATTR uint64_t GSR_ESP_WAKEUP = 0; // 1 for v1-2, 0 for v3.

RTC_DATA_ATTR struct CountUp final
{
  bool Active;
  time_t SetAt;
  time_t StopAt;
} TimerUp;

RTC_DATA_ATTR struct BatteryUse final
{
  float Read;       // div 100 from Last, since Last is much larger than normal.
  int8_t Direction; // -1 for draining, 1 for charging.
  int8_t DarkDirection; // Direction copy for Options.SleepMode.
  int8_t Level;         // Can go to +3 or -2 to determine direction of battery.
  int8_t State;         // 0=not visible, 1= showing chargeme, 2= showing
                        // reallychargeme, 3=showing charging.
  int8_t DarkState;     // Dark state of above.
  float MinLevel;       // Lowest level before the indicator comes on.
  float
      LowLevel; // The battery is about to get too low for the RTC to function.
  float LowError; // This is a decimal 0.00-0.10 value user changable that
                  // determines the battery drop off voltage from stock values.
  float RadioLevel; // WiFi and BT need this battery level to not brownout.
  time_t LastState; // Used to marshall the battery indicator for 10 second
                    // intervals to avoid Active Mode abuse.
  float Start;      // Used at the start of Init to catch brownouts.
  uint32_t ADCPin;  // Here for static use.
  float Divider;    // Divider for battery.
  float Divider2;   // For formatted battery.
  bool Charge;      // V3 charge indicator.
  bool floatInit;   // Moved to provide V3 time to "flatten".
  struct
  {
    time_t Stamp;  // Timestamp for the current Voltage below.
    float Voltage; // Voltage of that current Stamp.
  } ReadFloat[5];
  int8_t FloatBottom; // Used to indicate the oldest entry of the ReadFloat.
  int8_t FloatTop; // Used to indicate the "last" entry of the ReadFloat (it is
                   // the one stored in last)
} Battery;

RTC_DATA_ATTR struct NTPUse final
{
  uint8_t State; // State = 0=Off, 1=Start WiFi, 2=Wait for WiFi, TZ, send NTP
                 // request, etc, Finish.  See function ProcessNTP();
  uint8_t Wait;  // Counts up to 3 minutes, then fails.
  uint8_t Pause; // How many 50ms to pause for.
  unsigned long tWait; // Used to Only wait X seconds for a response.
  time_t Last;         // Last time it worked.
  bool TimeZone;       // Update Timezone during ProcessNTP.
  bool UpdateUTC;      // Update UTC during ProcessNTP.
  bool driftSync;      // Set to true when Automatic Drift Calculation is on.
  time_t driftSyncNOW; // Time after which the Auto Drift Calculation is
                       // finished, if no internet, add 3600 seconds and repeat.
  bool AutoSync;       // AutoSync time using the 3 values below.
  uint8_t SyncHour;
  uint8_t SyncMins;
  uint8_t SyncDays;
  time_t TravelTemp; // Value used in offset of time while changing the time to
                     // match, later used to determine drift more closely.
  char fromWeb[64];  // NTP from OTA Website, overrides all values.
} NTPData;

RTC_DATA_ATTR struct GoneDark final
{
  bool Went;
  unsigned long Last;
  bool Woke;
  unsigned long Tilt; // Used to track Tilt, if upright count for 1 second.
} Darkness;           // Whether or not the screen is darkened.

RTC_DATA_ATTR struct dispUpdate final
{
  bool Full;
  bool Drawn;
  bool Init;
  bool Tapped;
  /* Display setup for menu item filtering */
  struct HighInfo
  {
    uint64_t Bits; /* Binary values for each character */
    bool Active;
  } Highlighting;
  struct IndexInfo
  {
    uint8_t Offset;
    uint8_t Width; /* In characters */
    bool Active;
  } Indexing;
  struct DrawingInfo
  {
    uint8_t Offset;
    uint8_t Width;
  } MenuData;
} Updates;

RTC_DATA_ATTR struct WeatherGSR final
{
  bool Ready;    /* Weather read and ready */
  bool Metric;   /* SetWeatherScale is set here. */
  uint8_t State; /* Like NTP State 0 = idle, 1 = asking for WiFi, 2 = asking for
                    code, 3 = got code or error. */
  uint8_t Wait;  /* Counts up to 3 minutes, then fails. */
  uint8_t Pause; /* How many 50ms to pause for. */
  time_t goneStale; /* If time goes past this + Interval * 300, the weather data
                       is STALE */
  struct Weatherinfo
  {
    uint16_t ID; /* ID from weather info */
    struct TempsGSR
    {
      float FeelsLike; /* These are set to the Kelvin, when the function call
                          (SetWeatherScale(Metric is true/Imperial is false)) */
      float Current; /* is done, the functions for these will return the proper
                        values based on the selected scale. */
    } Temperature;
    uint8_t Humidity;    /* "humidity" from weather info */
    uint8_t Clouds;      /* "clouds" from weather info */
    uint16_t Pressure;   /* "pressure" from weather info */
    time_t SunRise;      /* "sunrise" from weather info */
    time_t SunSet;       /* "sunset" from weather info */
    uint32_t Visibility; /* "visibility" from weather info */
    float WindSpeed;     /* "wind" "speed" from weather info */
    int WindDirection;   /* "wind" "deg" from weather info */
    float WindGust;      /* "wind" "gust" from weather info */
  } Weather;
  time_t LastCall;   /* Last call to open-meteo.com + Honor ## minute delay */
  double LastLat;    /* Last Latitude  ^^^ */
  double LastLon;    /* Last Longitude ^^^ */
  double StaticLat;  /* Static Latitude  from OTA Website */
  double StaticLon;  /* Static Longitude from OTA Website */
  bool UseStaticPOS; /* Use the above 2 values */
  uint8_t Interval;  /* Interval between checks * 5 */
} WeatherData;       /* COD 'error' means something is amiss. */

RTC_DATA_ATTR struct webGSR final
{
  uint8_t Ready;      /* Data is ready for reading */
  int Response;       /* Code response from HTTP */
  String webURL;      /* NOT PERSISTENT! */
  uint8_t secTimeout; /* URL Timeout IN SECONDS */
  String Data;        /* Data retrieved from HTTP Get NOT PERSISTENT! */
} GSRWebData;

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> WatchyGSR::display (
    GxEPD2_154_D67 (WatchyGSR::getDispCS (), WatchyGSR::getDispDC (),
                    WatchyGSR::getDispRES (), WatchyGSR::getDispBSY ()));
SmallNTP SNTP;
SmallRTC SRTC;
AlarmsGSR Alarms; // Why do these need to be done twice?!?  Retarded.
RTC_DATA_ATTR Designing Design;
// RTC_DATA_ATTR TimeData WatchTime;
#ifdef STABLEBMA_H_INCLUDED
RTC_DATA_ATTR StableBMA SBMA;
#endif
LocaleGSR LGSR;
RTC_DATA_ATTR WatchyGSR *MonitorTo;
RTC_DATA_ATTR Olson2POSIX OP; // Tz code.
SPIClass WatchyGSR::hspi (HSPI);

volatile uint8_t Button;
volatile uint8_t
    Missed; // Button not in menu, not used, so can be used by override.
volatile bool KeysCheckOn; // Used to validate the loop during use.

WiFiClient WiFiC; // GSRWebGet
HTTPClient HTTP;  // Tz
WebServer server (80);
unsigned long OTATimer;
bool WatchyAPOn; // States Watchy's AP is on for connection.  Puts in Active
                 // Mode until back happens.
bool ActiveMode; // Moved so it can be checked.
bool Sensitive;  // Loop code is sensitive, like OTAUpdate, TimeTest
bool OTAUpdate;  // Internet based OTA Update.
bool OTAEnd;     // Means somewhere, it wants this to end, so end it.
bool OTAAsked; // OTA Website/Update asked for WiFi (this gets rid of the loop).
int OTATry;    // Tries to connect to WiFi.
bool DoHaptic; // Want it to happen after screen update.
bool UpdateDisp;  // Display needs to be updated.
bool AlarmsOn;    // Moved for CPU.
bool Rebooted;    // Used in DisplayInit to force a full initial on power up.
time_t TurboTime; // Moved here for less work.
bool AllowHaptic; // For the Task to tell the main thread it can't use the
                  // Haptic feedback due to alarm/timer.
bool WfNM;        // Wait for New Minute.
String GSRLogOutput;
unsigned long LastButton, OTAFail, OTAOff, LastHelp, lastDispDraw;
RTC_DATA_ATTR TaskHandle_t GSRHandle;
RTC_DATA_ATTR BaseType_t GSRRet;
RTC_DATA_ATTR TaskHandle_t KeysHandle;
RTC_DATA_ATTR BaseType_t KeysRet;
RTC_DATA_ATTR bool Started = false; //, goSound = false;
RTC_DATA_ATTR GSRCPUInfo Watchy_Chip_Info;
RTC_DATA_ATTR volatile int LastWebError;

WatchyGSR::WatchyGSR ()
{
  if (!Started)
    {
      Started = true;
      initZeros ();
    }
} // constructor

// Boot Override
void
WatchyGSR::InsertDefaults ()
{
}
void
WatchyGSR::AllowDefaultWatchStyles (bool Allow)
{
  DefaultWatchStyles = Allow;
}
void
WatchyGSR::InsertPost ()
{
}
bool
WatchyGSR::OverrideBitmap ()
{
  return false;
}
bool
WatchyGSR::OverrideSleepBitmap ()
{
  return false;
}

// Boot Face, Game, App
uint8_t
WatchyGSR::AddWatchStyle (String StyleName, WatchyGSR *AddOn, bool IsGame,
                          bool isAPP)
{
  int I, O;
  O = WatchStyles.Count;
  if (O >= GSR_MaxStyles || StyleName.length () > 30)
    return 255; // Full / too long..
  for (I = 0; I < O; I++)
    if (String (WatchStyles.Style[I * 32]) == StyleName)
      return 255; // Error, alrady there.

  strncpy (&WatchStyles.Style[O * 32], StyleName.c_str (), 32);
  WatchStyles.Options[O] = (isAPP ? GSR_APP : (IsGame ? GSR_GAM : 0));
  WatchStyles.AddOn[O] = AddOn;
  WatchStyles.Count++;
  if (IsGame)
    Options.GameCount++;
  return O;
}
void
WatchyGSR::InsertAddWatchStyles ()
{
}
void
WatchyGSR::OverrideDefaultMenu (bool Override)
{
  WatchStyles.Options[Options.WatchFaceStyle]
      = (WatchStyles.Options[Options.WatchFaceStyle] & ~GSR_MOV)
        | (Override ? GSR_MOV : 0);
}
void
WatchyGSR::WantWeather (bool Wanting)
{
  WatchStyles.Options[Options.WatchFaceStyle]
      = (WatchStyles.Options[Options.WatchFaceStyle] & ~GSR_iWW)
        | (Wanting ? GSR_iWW : 0);
}
void
WatchyGSR::NoStatus (bool NoStats)
{
  WatchStyles.Options[Options.WatchFaceStyle]
      = (WatchStyles.Options[Options.WatchFaceStyle] & ~GSR_NOS)
        | (NoStats ? GSR_NOS : 0);
}
void
WatchyGSR::NoAlarmStatus (bool NoAlarmStats)
{
  WatchStyles.Options[Options.WatchFaceStyle]
      = (WatchStyles.Options[Options.WatchFaceStyle] & ~GSR_NOA)
        | (NoAlarmStats ? GSR_NOA : 0);
}
void
WatchyGSR::UseMyWiFiIcon (const unsigned char *cWiFiOFF,
                          const unsigned char *cWiFiON, uint8_t width,
                          uint8_t height)
{
  if (cWiFiOFF != nullptr && cWiFiON != nullptr)
    {
      WatchStyles.DesignWiFi[Options.WatchFaceStyle].WiFiOFF = cWiFiOFF;
      WatchStyles.DesignWiFi[Options.WatchFaceStyle].WiFiON = cWiFiON;
      WatchStyles.DesignWiFi[Options.WatchFaceStyle].width = width;
      WatchStyles.DesignWiFi[Options.WatchFaceStyle].height = height;
      WatchStyles.DesignWiFi[Options.WatchFaceStyle].useMyWiFi = true;
    }
}

// Face, Game, App
uint8_t
WatchyGSR::CurrentStyleID ()
{
  if (GuiMode == GSR_GAMEON)
    return Options.Game;
  else
    return Options.WatchFaceStyle;
}
uint8_t
WatchyGSR::CurrentGameID ()
{
  return Options.Game;
}
void
WatchyGSR::drawTime (uint8_t Flags)
{
  String O;
  bool PM = false;
  O = MakeTime (WatchTime.Local.Hour | (Flags & 96), WatchTime.Local.Minute,
                PM);
  display.setFont (Design.Face.TimeFont);
  setFontColor (Design.Face.TimeColor);

  drawData (O, Design.Face.TimeLeft, Design.Face.Time, Design.Face.TimeStyle,
            Design.Face.Gutter, ((Flags & 32) ? false : true), PM);
}
void
WatchyGSR::drawDay ()
{
  String O = LGSR.GetFormatID (Options.LanguageID, 0);
  O.replace ("{W}", LGSR.GetWeekday (Options.LanguageID, WatchTime.Local.Wday));
  setFontFor (O, Design.Face.DayFont, Design.Face.DayFontSmall,
              Design.Face.DayFontSmaller, Design.Face.DayGutter);
  setFontColor (Design.Face.DayColor);
  drawData (O, Design.Face.DayLeft, Design.Face.Day, Design.Face.DayStyle,
            Design.Face.DayGutter);
}
void
WatchyGSR::drawDate (bool Short)
{
  String O = LGSR.GetFormatID (Options.LanguageID, 1);
  O.replace (
      "{M}",
      (Short ? LGSR.GetShortMonth (Options.LanguageID, WatchTime.Local.Month)
             : LGSR.GetMonth (Options.LanguageID, WatchTime.Local.Month)));
  O.replace ("{D}", String (WatchTime.Local.Day));
  setFontFor (O, Design.Face.DateFont, Design.Face.DateFontSmall,
              Design.Face.DateFontSmaller, Design.Face.DateGutter);
  setFontColor (Design.Face.DateColor);
  drawData (O, Design.Face.DateLeft, Design.Face.Date, Design.Face.DateStyle,
            Design.Face.DateGutter);
}
void
WatchyGSR::drawYear ()
{
  display.setFont (Design.Face.YearFont);
  setFontColor (Design.Face.YearColor);
  drawData (String (WatchTime.Local.Year + SRTC.getLocalYearOffset ()),
            Design.Face.YearLeft, Design.Face.Year, Design.Face.YearStyle,
            Design.Face.Gutter); // 1900
}
void
WatchyGSR::drawWeather (bool Status)
{
  uint8_t Style = Options.WatchFaceStyle;
  if (!WeatherData.Ready)
    return;
  if (DefaultWatchStyles)
    {
      if (Style > BasicWatchStyles && Style != 255)
        {
          if (WatchStyles.AddOn[Style] == nullptr)
            InsertDrawWeather (Style, Status);
          else
            WatchStyles.AddOn[Style]->InsertDrawWeather (Style, Status);
          return;
        }
    }
  else if (WatchStyles.Count > 0 && BasicWatchStyles == -1)
    {
      if (WatchStyles.AddOn[Style] == nullptr)
        InsertDrawWeather (Style, Status);
      else
        WatchStyles.AddOn[Style]->InsertDrawWeather (Style, Status);
      return;
    }
  InsertDrawWeather (Style, Status);
}
void
WatchyGSR::drawChargeMe (bool Dark)
{
  // Shows Battery Direction indicators.
  bool B = Dark;
  uint16_t C = (Dark ? GxEPD_WHITE : ForeColor ());
  Battery.Read = (rawBatteryVoltage ()) / 100;
  if (Design.Status.BatteryInverted && !B)
    C = BackColor ();
  if (Battery.Direction == 1)
    {
      display.drawBitmap (Design.Status.BATTx, Design.Status.BATTy, Charging,
                          40, 17, C); // Show Battery charging bitmap.
    }
  else if (Battery.Read <= getLowBattery ()
           && (GSR_PIN_RTC == 255 || Battery.Read > 3.1999f))
    { // At <3.2v, the display won't work (V3 workaround).
      display.drawBitmap (
          Design.Status.BATTx, Design.Status.BATTy,
          (Battery.Read < getLowBattery (true) ? ChargeMeBad : ChargeMe), 40,
          17, C); // Show Battery needs charging bitmap.
    }
}
void
WatchyGSR::drawStatus (bool Dark)
{
  uint8_t X = Design.Status.WIFIx;
  uint8_t Y = Design.Status.WIFIy - 16;
  uint8_t i = 0;
  bool a = (Alarms.alarmsGoing () > 4); // Smaller icons if > 4.
  bool w = WatchyStatus.startsWith (WiFiTXT);
  bool m = WatchStyles.DesignWiFi[Options.WatchFaceStyle].useMyWiFi;
  bool aOk = !GetNoAlarmStatus (); // Alarms can go.
  uint16_t C = (Dark ? GxEPD_WHITE
                     : (Design.Status.Inverted ? BackColor () : ForeColor ()));
  bool Ok = true, bOnce = false;
  display.setFont (&Bronova_Regular13pt7b);
  display.setTextColor (C);
  if (WatchyStatus > "" && !Dark)
    {
      if (w && !m)
        {
          Ok = false;
          display.drawBitmap (X, Design.Status.WIFIy - 18, iWiFi, 19, 19, C);
          if (WatchyStatus.length () > 4)
            {
              display.setCursor (X + 17, Design.Status.WIFIy);
              display.print (WatchyStatus.substring (4));
            }
        }
      else if (WatchyStatus == "TZ")
        {
          display.drawBitmap (X, Design.Status.WIFIy - 18, iTZ, 19, 19, C);
          Ok = false;
        }
      else if (WatchyStatus == "NTP")
        {
          display.drawBitmap (X, Design.Status.WIFIy - 18, iSync, 19, 19, C);
          Ok = false;
        }
      else if (WatchyStatus == "GL")
        {
          display.drawBitmap (X, Design.Status.WIFIy - 18, iGeo, 19, 19, C);
          Ok = false;
        }
      else if (WatchyStatus == "WE")
        {
          display.drawBitmap (X, Design.Status.WIFIy - 18, iWeather, 19, 19, C);
          Ok = false;
        }
      else if (WatchyStatus == "ESP")
        {
          display.drawBitmap (X, Design.Status.WIFIy - 18, iSync, 19, 19, C);
          Ok = false;
        }
      else if (WatchyStatus == "NC")
        {
          display.drawBitmap (X, Design.Status.WIFIy - 18, iNoClock, 19, 19, C);
          Ok = false;
        }
      else if (Alarms.firing ())
        Ok = true;
      else if (!w)
        {
          display.setCursor (X, Design.Status.WIFIy);
          display.print (WatchyStatus);
          Ok = false;
        }
    }

  if (Ok)
    {
      if (aOk)
        {
          for (i = 0; i < 8; i++)
            {
              if (Alarms.getTimes (i) > 0 && !Dark)
                {
                  if (a)
                    {
                      display.drawBitmap (X, Y, alarmIcons[i], 4, 6, C);
                      X += 6;
                    }
                  else
                    {
                      display.drawBitmap (X, Y, alarmIcons[i + 8], 12, 14, C);
                      X += 13;
                    }
                  Ok = false;
                }
              if (X - Design.Status.WIFIx > 23 && a && !bOnce)
                {
                  bOnce = true;
                  X = Design.Status.WIFIx;
                  Y += 8;
                }
            }
        }
      if (aOk && Alarms.getTimerDownToneLeft () > 0 && !Dark)
        {
          if (a)
            X = Design.Status.WIFIx
                + 52; // Force the countdown to the 5th spot.
          display.drawBitmap (X, Design.Status.WIFIy - 16, Tone_C, 12, 14, C);
          X += 13;
          Ok = false;
        }
      if (Ok && Options.GameStatus)
        {
          display.drawBitmap (X, Design.Status.WIFIy - 18, GameIcon, 19, 18, C);
          Ok = false;
        }
      if (Ok && m && !Dark)
        {
          display.drawBitmap (
              X,
              Design.Status.WIFIy
                  - WatchStyles.DesignWiFi[Options.WatchFaceStyle].height,
              (HasIPAddress ()
                   ? WatchStyles.DesignWiFi[Options.WatchFaceStyle].WiFiON
                   : WatchStyles.DesignWiFi[Options.WatchFaceStyle].WiFiOFF),
              WatchStyles.DesignWiFi[Options.WatchFaceStyle].width,
              WatchStyles.DesignWiFi[Options.WatchFaceStyle].height, C);
          Ok = false;
        }
      if (Ok && !Dark && WeatherData.Ready && GetWantWeather ())
        drawWeather (true);
    }
}
bool
WatchyGSR::IsBatteryHidden ()
{
  return (Battery.State == 0);
}
bool
WatchyGSR::IsDark ()
{
  return Darkness.Went;
}
bool
WatchyGSR::IsAM ()
{
  return (!Options.TwentyFour && WatchTime.Local.Hour < 12);
}
bool
WatchyGSR::IsPM ()
{
  return (!Options.TwentyFour && WatchTime.Local.Hour > 11);
}
bool
WatchyGSR::Is24HourMode ()
{
  return Options.TwentyFour;
}
bool
WatchyGSR::IsLightMode ()
{
  return Options.LightMode;
}
bool
WatchyGSR::isLeapYear (bool useLocal)
{
  if (useLocal)
    !((WatchTime.Local.Year + SRTC.getLocalYearOffset ()) % 4);
  return !((WatchTime.UTC.Year + SRTC.getLocalYearOffset ()) % 4);
}
String
WatchyGSR::MakeTime (int Hour, int Minutes, bool &Alarm)
{ // Use variable with Alarm, if set to False on the way in, returns PM
  // indication.
  int H;
  String AP = "";
  H = (Hour & 31);
  if (!Options.TwentyFour)
    {
      if (H > 11)
        {
          AP = " " + LGSR.GetID (Options.LanguageID, 114);
          if (!Alarm)
            {
              Alarm = true; // Tell the clock to use the PM indicator.
            }
        }
      else
        {
          AP = " " + LGSR.GetID (Options.LanguageID, 115);
        }
      if (H > 12)
        {
          H -= 12;
        }
      else if (H == 0)
        {
          H = 12;
        }
    }
  return (((Hour & 64) && H < 10) ? " " : "") + String (H)
         + (Minutes < 10 ? ":0" : ":") + String (Minutes)
         + ((Hour & 32) ? "" : AP);
}
String
WatchyGSR::MakeHour (uint8_t Hour)
{
  int H;
  H = Hour;
  if (!Options.TwentyFour)
    {
      if (H > 12)
        {
          H -= 12;
        }
      if (H == 0)
        {
          H = 12;
        }
    }
  return String (H);
}
String
WatchyGSR::MakeMinutes (uint8_t Minutes)
{
  return (Minutes < 10 ? "0" : "") + String (Minutes);
}
uint16_t
WatchyGSR::ForeColor ()
{
  return (Options.LightMode ? GxEPD_BLACK : GxEPD_WHITE);
}
uint16_t
WatchyGSR::BackColor ()
{
  return (Options.LightMode ? GxEPD_WHITE : GxEPD_BLACK);
}
void
WatchyGSR::InsertOnMinute ()
{
}
void
WatchyGSR::InsertInitWatchStyle (uint8_t StyleID)
{
}
void
WatchyGSR::InsertDrawWatchStyle (uint8_t StyleID)
{
}
void
WatchyGSR::InsertDrawOverlay (uint8_t StyleID)
{
}
void
WatchyGSR::InsertDrawWeather (uint8_t StyleID, bool Status)
{
}
bool
WatchyGSR::InsertNeedAwake (bool GoingAsleep)
{
  return false;
}
void
WatchyGSR::GameStatus (bool NeedAttention)
{
  if (NeedAttention != Options.GameStatus)
    UpdateScreen ();
  Options.GameStatus = NeedAttention;
}
bool
WatchyGSR::GameStatusOn ()
{
  return Options.GameStatus;
}
void
WatchyGSR::ShowDefaultMenu ()
{
  if (GetMenuOverride () && (GuiMode == GSR_WATCHON || GuiMode == GSR_GAMEON))
    {
      GuiMode = GSR_MENUON;
      DoHaptic = true;
      UpdateDisp = true;
      SetTurbo ();
    }
}
String
WatchyGSR::CurrentSteps (bool Yesterday)
{
  String S = YesterdaySteps ();
  return MakeSteps (CurrentStepCount ())
         + ((S > "" && Yesterday) ? " (" + S + ")" : "");
}
#ifdef STABLEBMA_H_INCLUDED
uint32_t
WatchyGSR::CurrentStepCount ()
{
  return SBMA.getCounter () + Steps.Cached;
}
#else
uint32_t
WatchyGSR::CurrentStepCount ()
{
  return Steps.Cached;
}
#endif
String
WatchyGSR::YesterdaySteps ()
{
  if (Steps.Yesterday > 0)
    return MakeSteps (Steps.Yesterday);
  return "";
}
uint32_t
WatchyGSR::YesterdayStepCount ()
{
  return Steps.Yesterday;
}
bool
WatchyGSR::BMAAvailable ()
{
#ifdef STABLEBMA_H_INCLUDED
  return true;
#else
  return false;
#endif
}
float
WatchyGSR::BMATemperature (bool Metric)
{
#ifdef STABLEBMA_H_INCLUDED
  if (BMAAvailable ())
    return (Metric ? SBMA.readTemperature () : SBMA.readTemperatureF ());
#endif
  return -1;
}
String
WatchyGSR::CurrentWatchStyle ()
{
  uint8_t X;
  char O[32];
  memcpy (&O[0], &WatchStyles.Style[Options.WatchFaceStyle * 32], 32);
  return String (O);
}
String
WatchyGSR::CurrentGameStyle ()
{
  uint8_t X;
  char O[32];
  if (Options.Game != 255)
    memcpy (&O[0], &WatchStyles.Style[Options.Game * 32], 32);
  return String (O);
}
String
WatchyGSR::CountdownTimer ()
{
  if (CountdownTimerActive ())
    UpdateUTC ();
  return String (CountdownTimerActive () ? Alarms.getTimerDownHours ()
                                         : Alarms.getTimerDownMaxHours ())
         + ":"
         + MakeMinutes (CountdownTimerActive ()
                            ? Alarms.getTimerDownMinutes ()
                            : Alarms.getTimerDownMaxMinutes ())
         + ":"
         + MakeMinutes (CountdownTimerActive ()
                            ? Alarms.getTimerDownSeconds ()
                            : Alarms.getTimerDownMaxSeconds ());
}
String
WatchyGSR::CountdownTimerState ()
{
  return LGSR.GetID (Options.LanguageID, (CountdownTimerActive () ? 68 : 69));
}
bool
WatchyGSR::CountdownTimerActive ()
{
  return Alarms.getTimerDownActive ();
}
String
WatchyGSR::ElapsedTimer ()
{
  unsigned long U;
  int16_t H, M, S;
  if (TimerUp.Active)
    {
      UpdateUTC ();
      U = (WatchTime.UTC_RAW - TimerUp.SetAt);
    }
  else
    U = (TimerUp.StopAt - TimerUp.SetAt);
  H = U / 3600;
  M = (U - (H * 3600)) / 60;
  S = U % 60;
  return String (H) + ":" + MakeMinutes (M) + ":" + MakeMinutes (S);
}
String
WatchyGSR::ElapsedTimerState ()
{
  return LGSR.GetID (Options.LanguageID, (TimerUp.Active ? 68 : 69));
}
bool
WatchyGSR::ElapsedTimerActive ()
{
  return TimerUp.Active;
}
bool
WatchyGSR::GetWantWeather ()
{
  return (WatchStyles.Options[Options.WatchFaceStyle] & GSR_iWW);
}
bool
WatchyGSR::GetNoStatus ()
{
  return (WatchStyles.Options[Options.WatchFaceStyle] & GSR_NOS);
}
bool
WatchyGSR::GetNoAlarmStatus ()
{
  return (WatchStyles.Options[Options.WatchFaceStyle] & GSR_NOA);
}
bool
WatchyGSR::GetMenuOverride ()
{
  return (WatchStyles.Options[Options.WatchFaceStyle] & GSR_MOV);
}
void
WatchyGSR::ShowGame ()
{
  if (Options.Game != 255)
    {
      Menu.SubItem = 0;
      GuiMode = GSR_GAMEON;
      UpdateDisp = true;
    }
}
void
WatchyGSR::HideGame ()
{
  if (GuiMode = GSR_GAMEON)
    {
      Menu.SubItem = 0;
      GuiMode = GSR_WATCHON;
      UpdateDisp = true;
    }
}
bool
WatchyGSR::SafeToDraw ()
{
  return (!(
      OTAUpdate || WatchyAPOn
      || (Menu.Item == GSR_MENU_TOFF && Menu.SubItem > 1 && Menu.SubItem < 7)));
}
bool
WatchyGSR::NoMenu ()
{
  return (GuiMode == GSR_WATCHON || GuiMode == GSR_GAMEON);
};
bool
WatchyGSR::InGame ()
{
  return (GuiMode == GSR_GAMEON);
}
void
WatchyGSR::setFontColor (uint16_t Color)
{
  bool B = (Color == GxEPD_BLACK || Color == GxEPD_WHITE);
  if (!B && Color == GSR_AutoFore)
    Color = ForeColor ();
  display.setTextColor (Color);
}

// Interaction
bool
WatchyGSR::InsertHandlePressed (uint8_t SwitchNumber, bool &Haptic,
                                bool &Refresh)
{
  return false;
}
uint8_t
WatchyGSR::getButtonPins ()
{
  return getSWValue ((digitalRead (GSR_MENU_PIN) == GSR_ESP_WAKEUP),
                     (digitalRead (GSR_BACK_PIN) == GSR_ESP_WAKEUP),
                     (digitalRead (GSR_UP_PIN) == GSR_ESP_WAKEUP),
                     (digitalRead (GSR_DOWN_PIN) == GSR_ESP_WAKEUP));
}

// Power
bool
WatchyGSR::getBatteryCharging ()
{
  return (Battery.Direction == 1);
}
float
WatchyGSR::getBatteryVoltage ()
{
  return WatchyGSR::rawBatteryVoltage (true);
}
float
WatchyGSR::getBatteryVoltagePercent ()
{
  float bL = getLowBattery ();
  float bH = 4.2f - bL;
  float bC = getBatteryVoltage () - bL;
  bL = (bC / bH) * 100.0f;
  bL = (bL > 100.0f ? 100.0f : (bL > 0.0f ? bL : 0.0f));
  return bL;
}
float
WatchyGSR::getLowBattery (bool useCritical)
{
  return getRealLowBattery (useCritical);
}
float
WatchyGSR::getLowBatteryRadio ()
{
  return getRealLowBattery (false, true);
}

// Radio
void
WatchyGSR::AskForWiFi ()
{
  if (rawBatteryVoltage (true) > getLowBatteryRadio ())
    {
      SetAskWiFi (Watchy_Chip_Info.HasWiFi);
      GSRWiFi.Requests++;
    }
}
bool
WatchyGSR::GetAskWiFi ()
{
  return (GuiMode != GSR_GAMEON
              ? (WatchStyles.Options[Options.WatchFaceStyle] & GSR_AFW)
              : false);
}
void
WatchyGSR::InsertWiFi ()
{
}
void
WatchyGSR::InsertWiFiEnding ()
{
}
void
WatchyGSR::endWiFi ()
{
  if (!GetAskWiFi () && (GSRWiFi.Powered || GSRWiFi.Requests > 0))
    GSRWiFi.Requests = 0;
  if (GSRWiFi.Requests > 0)
    GSRWiFi.Requests--;
  if (GSRWiFi.Powered && GSRWiFi.Requests == 0)
    {
      WiFi.disconnect ();
      if (GSRWiFi.WiFiEventID)
        WiFi.removeEvent (GSRWiFi.WiFiEventID);
      GSRWiFi.WiFiEventID = 0;
      WiFi.mode (WIFI_OFF);
      GSRWiFi.Requests = 0;
      GSRWiFi.Requested = false;
      GSRWiFi.Working = false;
      GSRWiFi.Results = false;
      GSRWiFi.Index = 0;
      GSRWiFi.Powered = false;
      GSRWiFi.Tried = false;
      GSRWiFi.Prepped = false;
      SetAskWiFi (false);
      if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr)
        InsertWiFiEnding ();
      else
        WatchStyles.AddOn[Options.WatchFaceStyle]->InsertWiFiEnding ();
      WatchyOStatus = "";
      setStatus ("");
    }
}

// Clock
time_t
WatchyGSR::getISO8601 (String inTime)
{
  struct tm tmout;
  String tmp;
  uint8_t ind;
  tmp = inTime.charAt (ind);
  if (tmp == "[")
    ind++;
  tmp = inTime.charAt (ind);
  if (tmp == " ")
    ind++;
  if (ind)
    {
      tmp = inTime.substring (ind);
    }
  else
    {
      tmp = inTime;
    }
  strptime (tmp.c_str (), "%Y-%m-%dT%H:%M", &tmout);
  return tmTOtime_t (tmout);
}
tmElements_t
WatchyGSR::UTCtoLocal (time_t Incoming)
{
  struct tm *TM;
  tmElements_t T;

  OP.setCurrentTimeZone ();
  TM = localtime (&Incoming);
  T.Second = TM->tm_sec;
  T.Minute = TM->tm_min;
  T.Hour = TM->tm_hour;
  T.Wday = TM->tm_wday;
  T.Day = TM->tm_mday;
  T.Month = TM->tm_mon;
  T.Year = TM->tm_year;
  return T;
}
void
WatchyGSR::requestClockUpdate ()
{
  UpdateClock ();
}
void
WatchyGSR::requestUTCUpdate ()
{
  MonitorTo->UpdateUTC ();
}

// Utility
void
WatchyGSR::requestRefreshScreen ()
{
  UpdateDisp = true;
}
void
WatchyGSR::UpdateScreen ()
{
  UpdateDisp |= Showing ();
  Updates.Drawn = true;
}
void
WatchyGSR::requestDisplayWake ()
{
  MonitorTo->DisplayWake (true);
}
bool
WatchyGSR::requestShowing ()
{
  return WatchyGSR::Showing ();
}
bool
WatchyGSR::requestOTAStatus ()
{
  return OTAUpdate;
}
void
WatchyGSR::NeedsSaving ()
{
  Options.NeedsSaving = true;
}
String
WatchyGSR::InsertNTPServer ()
{
  return "pool.ntp.org";
}
void
WatchyGSR::getAngle (uint16_t Angle, uint8_t Width, uint8_t Height,
                     uint8_t &OutX, uint8_t &OutY)
{
  double A, mTan, mY, mX;
  uint8_t cX = Width / 2; /* Thanks to Henny022 on the Watchy Discord for this
                             code, I did neaten it up for readabilty more. */
  uint8_t cY = Height / 2;
  Angle %= 360; // Trim angle (shouldn't ever happen, but hey).
  A = ((2 * M_PI) / 360) * Angle;
  mTan = tan (A);

  mY = (A < M_PI_2 || A > 3 * M_PI_2) ? -cY : cY;
  mX = -mTan * mY;

  if (mX > cX || mX < -cX)
    {
      mX = (A < M_PI) ? cX : -cX;
      mY = -mX / mTan;
    }

  OutX = mX + cX;
  OutY = mY + cY;
  OutX = OutX - ((OutX == Width) ? 1 : 0);
  OutY = OutY - ((OutY == Height) ? 1 : 0);
}
bool
WatchyGSR::filterGeo (double geoValue, bool isLatitude)
{
  return !(isLatitude ? (geoValue > 90.0f || geoValue < -90.0f)
                      : (geoValue > 180.0f || geoValue < -180.0f));
}
bool
WatchyGSR::ChangeWatchFace (bool Up)
{
  byte I = Options.WatchFaceStyle;
  byte C = 0;
  // if WiFiInProgress() return false;  // Avoid WiFi connection.
  while (C < WatchStyles.Count)
    {
      I = roller (I + (Up ? 1 : -1), 0, WatchStyles.Count - 1);
      C++;
      if (!(WatchStyles.Options[I] & GSR_GAM))
        C = WatchStyles.Count;
    }
  if (I != Options.WatchFaceStyle && !(WatchStyles.Options[I] & GSR_GAM))
    {
      WatchFaceEnd ();
      WatchFaceStart (I);
      Options.NeedsSaving = true;
      return true;
    }
  return false;
}
String
WatchyGSR::getCurrentNTPServer ()
{
  return (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr
              ? InsertNTPServer ()
              : WatchStyles.AddOn[Options.WatchFaceStyle]->InsertNTPServer ());
}
String
WatchyGSR::CleanString (String Clean)
{
  Clean.replace ('"', ' ');
  Clean.trim ();
  return Clean;
}

// Weather
bool
WatchyGSR::IsWeatherAvailable ()
{
  return (WatchTime.UTC_RAW
              < (WeatherData.goneStale + WeatherData.Interval * 300)
          && WeatherData.Ready);
}
int
WatchyGSR::GetWeatherTemperature ()
{
  int T = 255;
  if (WeatherData.Ready)
    T = int (WeatherData.Weather.Temperature.Current);
  else
    T = SRTC.temperature ();
  if (T != 255 && !WeatherData.Metric)
    T = (T * 1.8) + 32;
  return T;
}
int
WatchyGSR::GetWeatherTemperatureFeelsLike ()
{
  int T = 255;
  if (WeatherData.Ready)
    T = int (WeatherData.Weather.Temperature.FeelsLike);
  else
    T = SRTC.temperature ();
  if (T != 255 && !WeatherData.Metric)
    T = (T * 1.8) + 32;
  return T;
}
void
WatchyGSR::SetWeatherScale (bool Metric)
{
  WeatherData.Metric = Metric;
}
bool
WatchyGSR::IsMetric ()
{
  return WeatherData.Metric;
}
int
WatchyGSR::GetWeatherID ()
{
  return WeatherData.Weather.ID;
}
uint8_t
WatchyGSR::GetWeatherHumidity ()
{
  return WeatherData.Weather.Humidity;
}
uint8_t
WatchyGSR::GetWeatherClouds ()
{
  return WeatherData.Weather.Clouds;
}
time_t
WatchyGSR::GetWeatherSunRise ()
{
  return WeatherData.Weather.SunRise;
}
time_t
WatchyGSR::GetWeatherSunSet ()
{
  return WeatherData.Weather.SunSet;
}
uint16_t
WatchyGSR::GetWeatherPressure ()
{
  return WeatherData.Weather.Pressure;
}
uint32_t
WatchyGSR::GetWeatherVisibility ()
{
  if (WeatherData.Metric)
    return WeatherData.Weather.Visibility;
  else
    return WeatherData.Weather.Visibility * 0.6214;
}
float
WatchyGSR::GetWeatherWindSpeed ()
{
  if (WeatherData.Metric)
    return WeatherData.Weather.WindSpeed;
  else
    return WeatherData.Weather.WindSpeed * 0.6214;
}
float
WatchyGSR::GetWeatherWindDirection ()
{
  return WeatherData.Weather.WindDirection;
}
float
WatchyGSR::GetWeatherWindGust ()
{
  if (WeatherData.Metric)
    return WeatherData.Weather.WindGust;
  else
    return WeatherData.Weather.WindGust * 0.6214;
}
double
WatchyGSR::getWeatherLatitude (bool useStatic)
{
  return (useStatic & WeatherData.UseStaticPOS) ? WeatherData.StaticLat
                                                : WeatherData.LastLat;
}
double
WatchyGSR::getWeatherLongitude (bool useStatic)
{
  return (useStatic & WeatherData.UseStaticPOS) ? WeatherData.StaticLon
                                                : WeatherData.LastLon;
}

// Web
bool
WatchyGSR::GetWebAvailable ()
{
  return (GSRHandle == NULL);
}
bool
WatchyGSR::GetWebReady ()
{
  return GSRWebData.Ready;
}
int
WatchyGSR::GetWebResponse ()
{
  return GSRWebData.Response;
}
String
WatchyGSR::GetWebData ()
{
  return GSRWebData.Data;
}
bool
WatchyGSR::AskForWeb (String website, uint8_t Timeout)
{
  if (!GetWebAvailable ())
    return false;
  GSRWebData.webURL = website;
  GSRWebData.secTimeout = Timeout;
  GSRRet = xTaskCreate (WatchyGSR::GSRWebGet, "GSRWebGet", 3328, NULL,
                        (tskIDLE_PRIORITY + 1), &GSRHandle);
  return (GSRHandle != NULL);
}

// Override Only
void
WatchyGSR::init (String datetime)
{
  uint64_t wakeupBit;
  bool B, Up, OTAWiFi, userWakeup;
  unsigned long Since, APLoop;
  uint8_t I;
  String S;
  startSetup ();
  esp_sleep_wakeup_cause_t wakeup_reason
      = esp_sleep_get_wakeup_cause (); // get wake up reason
  esp_reset_reason_t reset_reason = esp_reset_reason ();
  wakeupBit = esp_sleep_get_ext1_wakeup_status ();
  NVS.begin ();
  ResetEndOTA ();
  WfNM = false;
  OTAUpdate = false;
  OTAEnd = false;
  OTAAsked = false;
  WatchyAPOn = false;
  AlarmsOn = false;
  Sensitive = false;
  Updates.Drawn = false; // Force it here.
  Updates.Init = true;
  Updates.Tapped = false;
  LastButton = 0;
  Button = 0;
  Missed = 0;
  OTATry = 0;
  GSRWiFi.Slow = 0;
  Darkness.Last = 0;
  Darkness.Tilt = 0;
  Darkness.Woke = false;
  userWakeup = false;
  TurboTime = 0;
  AllowHaptic = true;
  GSRHandle = NULL;
  KeysHandle = NULL;
  KeysCheckOn = false;
  CPUSet.Freq = getCpuFrequencyMhz ();
  SRTC.sysBoot ();

  switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:  // RTC Alarm (V1-V2)
    case ESP_SLEEP_WAKEUP_TIMER: // Internal RTC Alarm
      RefreshCPU (GSR_CPUDEF);
      BrownOutDetect ();
#ifndef SMALL_RTC3_H
      SRTC.clearAlarm ();
#endif
      UpdateUTC ();
      UpdateClock ();
      if (WatchTime.Local.Second == 0)
        WatchTime.NewMinute = true;
      else
        WfNM = (WatchTime.Local.Second > 58);
      adcPins ();
      detectBattery ();
      Battery.Read = rawBatteryVoltage () / 100;
      Alarms.CalculateTones ();
      AlarmsOn = Alarms.firing ();
      if (Updates.Drawn)
        DisplayWake ();
      UpdateDisp = Showing ();
      break;
    case ESP_SLEEP_WAKEUP_EXT1: // button Press, BMA
      adcPins ();
      userWakeup = true;
      break;
    default: // reset
      MonitorTo = this;
      SetupESPValues ();
      adcPins ();
      BrownOutDetect ();
      WatchStyles.Count = 0;
      BasicWatchStyles = -1;
      SRTC.init ();
      Battery.MinLevel = SRTC.getRTCBattery ();
      Battery.LowLevel = SRTC.getRTCBattery (true);
      Battery.RadioLevel = Battery.LowLevel + 0.1;
      HWVer = SRTC.getWatchyHWVer ();
      getPins (HWVer);
      setupDefaults ();
      _bmaConfig ();
      Rebooted = true;
      UpdateUTC ();
      Alarms.init (alarmResponse, GSR_PIN_VIB_PWM);
      Battery.ADCPin = SRTC.getADCPin ();
      Battery.LastState = 0;
      Battery.FloatBottom = 0;
      Battery.FloatTop = 4;
      Battery.Divider = (HWVer == 3.0f ? 7.39750f : 5.0f); // 7.39750 vs 7.826
      Battery.Divider2
          = (HWVer == 3.0f ? 739.750f : 500.0f); // 739.750 vs 782.608
      Battery.State = 0;
      Battery.DarkState = 0;
      Battery.Direction = -1;
      Battery.DarkDirection = 0;
      detectBattery ();
      Battery.Read = rawBatteryVoltage () / 100;
      if (DefaultWatchStyles)
        {
          I = AddWatchStyle ("{%0%}"); // Classic GSR
          BasicWatchStyles = I;
        }
      InsertAddWatchStyles ();
      if (WatchStyles.Count == 0)
        {
          I = AddWatchStyle ("{%0%}"); // Classic GSR
          BasicWatchStyles = I;
          DefaultWatchStyles = true;
        }
      for (I = 0; I < BootAddOns.Count; I++)
        {
          if (BootAddOns.Inits[I] != nullptr)
            BootAddOns.Inits[I]->RegisterWatchFaces ();
        }
      if (OkNVS (GSR_GName))
        B = NVS.getString (GSR_GTZ, S);
      OP.setCurrentPOSIX (S);
      RetrieveSettings ();
      WatchTime.ESPRTC |= (HWVer == 3.0f);
      if (OkNVS (GSR_GName))
        {
          S = "";
          B = NVS.getString (GSR_STP, S);
          if (S.toInt ())
            Steps.Cached = S.toInt ();
          S = "";
          B = NVS.getString (GSR_YSTP, S);
          if (S.toInt ())
            Steps.Yesterday = S.toInt ();
          NVS.erase (GSR_STP);
          NVS.erase (GSR_YSTP);
          NVS.commit ();
        }
      RefreshCPU (GSR_CPUDEF);
      UpdateClock ();
      InsertPost ();
      Updates.Full = true;
#ifndef GxEPD2DarkBorder
      Options.Border = false;
#endif
      if (Options.GameCount > 0)
        {
          if (Options.Game == 255)
            B = ChangeGame ();
          if (Options.Game != 255)
            initGame (Options.Game);
        }
      StartNTP (true);
      WatchFaceStart (Options.WatchFaceStyle);
      DisplayWake (true);
      DoHaptic = true;
#ifndef STABLEBMA_H_INCLUDED
      Menu.Item = GSR_MENU_ALARMS;
#else
      Menu.Item = GSR_MENU_STEPS;
#endif
      break;
    }

  if (userWakeup)
    {
      RefreshCPU (GSR_CPUDEF);
      BrownOutDetect ();
      UpdateUTC ();
      Button = getButtonMaskToID (wakeupBit);
      B = (Options.SleepStyle != 4);
      if (B)
        UpdateDisp |= Showing ();
      if (Darkness.Went && (UpRight ()))
        {
          if (Button == 9 && Options.SleepStyle > 1 && B)
            { // Accelerometer caused this.
              if (Options.SleepMode == 0)
                Options.SleepMode = 2; // Do this to avoid someone accidentally
                                       // not setting this before usage.
              DisplayWake (true);      // Update Screen to new state.
            }
          else if (Button == 10 && !WatchTime.BedTime)
            {                 // Wrist.
              DisplayWake (); // Do this anyways, always.
            }
          else if (Button)
            {
              DisplayWake ();
              LastButton = millis ();
              Button = 0; // Update Screen to new state. if (Options.SleepStyle
                          // > 1 && B)
            }
        }
      if (Darkness.Woke || Button)
        UpdateClock (); // Make sure this is done when buttons are pressed for a
                        // wakeup.
    }

  B = true;
  Up = false;
  if (Darkness.Went && GuiMode != GSR_MENUON)
    {
      B = (Battery.DarkState != Battery.State
           || Battery.DarkDirection != Battery.Direction);
      if (Options.SleepStyle == 4)
        B |= Updates.Tapped;
      else
        B |= (Darkness.Woke || Button);
      // if (!B && Options.Game != 255 && WatchTime.NewMinute) { if
      // (WatchStyles.AddOn[Options.Game] != nullptr)
      // WatchStyles.AddOn[Options.Game]->InsertOnMinute(); }
    }
  B |= Alarms.firing ();

  if (B || Updates.Full || WatchTime.NewMinute || WfNM)
    {

      // Sometimes BMA crashes - simply try to reinitialize bma...

#ifdef STABLEBMA_H_INCLUDED
      if (SBMA.getErrorCode () != 0)
        {
          SBMA.shutDown ();
          SBMA.wakeUp ();
          SBMA.softReset ();
          _bmaConfig ();
        }
#endif

      espPinModes ();
      ActiveMode = true;
      RefreshCPU ((Darkness.Went && GuiMode != GSR_MENUON && WatchTime.BedTime)
                      ? GSR_CPULOW
                      : 0); // Use low CPU usage at night.
      Battery.Start = golow (getBatteryVoltage (), 4.22);
      Battery.Read = Battery.Start;
      KeysStart ();
      while (ActiveMode || UpdateDisp || WfNM)
        {

          Since = millis ();
          ManageTime (); // Handle Time method.
#ifdef STABLEBMA_H_INCLUDED
          Up = SBMA.IsUp (); // SBMA.IsUp();
#endif
          B = DarkWait ();

          if (GSRWiFi.Slow > 0)
            GSRWiFi.Slow--;
          /* Pre-Tilt */

          if (!B && !Up)
            GoDark ();

          /* Tilt */
          // Wrist Tilt delay, keep screen on during this until you put your
          // wrist down.
          if ((Options.SleepStyle == 1
               || (Options.SleepStyle > 2 && Options.SleepStyle != 4))
              && !WatchTime.BedTime)
            {
              if (Darkness.Went && Up && !Darkness.Woke)
                { // Do this when the wrist is UP.
                  if (Darkness.Tilt == 0)
                    Darkness.Tilt = millis ();
                  else if (millis () - Darkness.Tilt > 999)
                    {
                      Darkness.Last = millis ();
                      Darkness.Woke = true;
                      UpdateDisp |= Showing ();
                    }
                }
              if (!Up)
                Darkness.Tilt = 0;
              else
                {
                  if (B)
                    Darkness.Last = millis ();
                  if (Darkness.Tilt != 0 && millis () - Darkness.Tilt > 999
                      && Darkness.Woke)
                    Darkness.Last = millis ();
                }
            }

          // Here, check for button presses and respond, done here to avoid
          // turbo button presses.
          if (Button)
            {
              handleButtonPress (Button);
              Button = 0;
            }

          /* Insert */

          if (currentWiFi () == WL_CONNECTED && NTPData.State == 0
              && WeatherData.State == 0 && !OTAUpdate && !WatchyAPOn
              && !UpdateDisp && GSRWiFi.Slow == 0 && !inBrownOut ())
            {
              if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr)
                InsertWiFi ();
              else
                WatchStyles.AddOn[Options.WatchFaceStyle]->InsertWiFi ();
            }

          /* NTP */

          if (NTPData.State && WeatherData.State < 2 && !WatchyAPOn
              && !OTAUpdate)
            {
              if (GSRWiFi.Slow == 0)
                {
                  if (NTPData.Pause == 0)
                    ProcessNTP ();
                  else
                    NTPData.Pause--;
                }
              if (WatchTime.NewMinute)
                {
                  NTPData.Wait++;
                  UpdateDisp |= Showing ();
                }
            }

          /* Weather */

          if (WeatherData.State && NTPData.State < 2 && !WatchyAPOn
              && !OTAUpdate && GSRWiFi.Slow == 0)
            {

              if (WeatherData.Pause == 0)
                ProcessWeather ();
              else
                WeatherData.Pause--;
              if (WatchTime.NewMinute)
                {
                  WeatherData.Wait++;
                  UpdateDisp |= Showing ();
                }
            }
          if (Battery.Read > getLowBatteryRadio ()
              && WatchTime.UTC_RAW >= WeatherData.LastCall && !WatchTime.BedTime
              && WeatherData.Interval > 0)
            StartWeather ();

          /* Menu Override */

          if (GuiMode == GSR_WATCHON && GetMenuOverride ())
            {
              if (Missed == 0)
                Missed = WatchyGSR::getButtonPins ();
              if ((Missed != 1 && Button) || (Missed == 1 && LastHelp == 0))
                {
                  LastHelp = millis ();
                  SetTurbo ();
                }
              else if (Missed == 1 && millis () - LastHelp > 9999)
                {
                  ShowDefaultMenu ();
                  Missed = 0;
                }
            }
          if (Missed)
            {
              if (GuiMode == GSR_WATCHON)
                {
                  if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr)
                    {
                      if (InsertHandlePressed (Missed, DoHaptic, UpdateDisp))
                        SetTurbo ();
                    }
                  else if (WatchStyles.AddOn[Options.WatchFaceStyle]
                               ->InsertHandlePressed (Missed, DoHaptic,
                                                      UpdateDisp))
                    SetTurbo ();
                }
              else if (GuiMode == GSR_GAMEON && Options.Game != 255)
                {
                  if (WatchStyles.AddOn[Options.Game]->InsertHandlePressed (
                          Missed, DoHaptic, UpdateDisp))
                    SetTurbo ();
                }
              Missed = 0;
              Button = 0;
            }

          // Here, make sure the clock updates on-screen.
          if (WatchTime.NewMinute)
            UpdateDisp |= Showing ();

          /* OTA */

          // OTAEnd code.
          if (OTAEnd)
            {
              if (Menu.Item == GSR_MENU_OTAU)
                {
                  ArduinoOTA.end ();
                  Menu.SubItem = 0;
                }
              else if (Menu.Item == GSR_MENU_OTAM)
                {
                  server.stop ();
                  Menu.SubItem = 0;
                }
              if (WatchyAPOn)
                server.stop ();
              Alarms.endSounds ();
              OTAEnd = false;
              OTAAsked = false;
              endWiFi ();
              if (Menu.Item == GSR_MENU_WIFI && Menu.SubItem > 0)
                Menu.SubItem = 0;
              if (OTAUpdate)
                Menu.SubItem = 0;
              UpdateDisp |= Showing () | WatchyAPOn | OTAUpdate;
              UpdateUTC ();
              UpdateClock ();
              OTAUpdate = false;
              setStatus ("");
              WatchyAPOn = false;
              if (Options.NeedsSaving)
                RecordSettings ();
              Missed = 0;
              Button = 0;
            }

          /* OTA */

          if (WatchyAPOn && IsEndOTA ())
            OTAEnd = true; // Fail if holding back for 10 seconds OR 600 seconds
                           // has passed.
          if (OTAUpdate && !UpdateDisp && GSRWiFi.Slow == 0)
            {
              switch (Menu.SubItem)
                {
                case 1: // Wait for WiFi to connect or fail.
                  if (!GetAskWiFi () && !OTAAsked)
                    {
                      AskForWiFi ();
                      OTAAsked = true;
                    }
                  else if (WiFiStatus () != WL_CONNECTED
                           && currentWiFi () != WL_CONNECT_FAILED)
                    OTATimer = millis ();
                  else if (HasIPAddress ())
                    {
                      OTATimer = millis ();
                      setStatus (
                          WiFiIndicator (GSRWiFi.Index ? GSRWiFi.Index : 24));
                      Menu.SubItem++;
                      UpdateDisp |= Showing ();
                    }
                  else if (!WiFiInProgress ())
                    OTAEnd = true;
                  break;
                case 2: // Setup Arduino OTA and wait for it to either finish or
                        // fail by way of back button held for too long OR 2
                        // minute with no upload.
                  if (Menu.Item == GSR_MENU_OTAU)
                    {
                      ArduinoOTA.setHostname (WiFi_AP_SSID);
                      ArduinoOTA
                          .onStart ([] () {
                            String Type;
                            if (ArduinoOTA.getCommand () == U_FLASH)
                              Type = "sketch";
                            else // U_SPIFFS
                              Type = "filesystem";
                            // NOTE: if updating SPIFFS this would be the place
                            // to unmount SPIFFS using SPIFFS.end()
                          })
                          .onEnd ([] () { OTAEnd = true; })
                          .onProgress (
                              [] (unsigned int progress, unsigned int total) {
                                OTATimer = millis ();
                              })
                          .onError ([] (ota_error_t error) { OTAEnd = true; });
                      RefreshCPU (GSR_CPUMAX);
                      ArduinoOTA.begin ();
                      ArduinoOTA.setPassword ("watchy");
                    }
                  else if (Menu.Item == GSR_MENU_OTAM)
                    WatchyGSR::StartWeb ();
                  Menu.SubItem++;
                  // showWatchFace();
                  break;
                case 3: // Monitor back button and turn WiFi off if it happens,
                        // or if duration is longer than 2 minutes.
                  if (WiFiStatus () == WL_DISCONNECTED || OTAEnd)
                    OTAEnd = true;
                  else if (Menu.Item == GSR_MENU_OTAU)
                    ArduinoOTA.handle ();
                  else if (Menu.Item == GSR_MENU_OTAM)
                    server.handleClient ();
                  if (WatchyGSR::getButtonPins () != 2)
                    OTATimer = millis (); // Not pressing "BACK".
                  if (millis () - OTATimer > 10000 || millis () > OTAFail
                      || IsEndOTA ())
                    OTAEnd = true; // Fail if holding back for 10 seconds OR 600
                                   // seconds has passed.
                }
            }
          if (UpdateDisp)
            showWatchFace (); // partial updates on tick

          /* NON-SENSITIVE */
          // Don't do anything time sensitive while in OTA Update.

          if (!Sensitive)
            {

              /* Battery Detection */

              detectBattery ();

              /* WATCHY AP */

              if (GSRWiFi.Requests == 0 && GSRWiFi.Slow == 0 && WatchyAPOn
                  && !OTAUpdate)
                {
                  switch (Menu.SubItem)
                    {
                    case 0: // The AP is now off.
                      break;
                    case 1: // Turn on AP.
                      if (WiFi.getMode () != WIFI_AP
                          || (millis () - OTATimer > 4000 && OTATry < 3)
                                 && !inBrownOut ())
                        {
                          OTATimer = millis ();
                          OTATry++;
                          WiFi.setHostname (WiFi_AP_SSID);
                          RefreshCPU (GSR_CPUMAX);
                          OTAEnd
                              |= (!WiFi.softAP (WiFi_AP_SSID, WiFi_AP_PSWD, 1,
                                                WiFi_AP_HIDE, WiFi_AP_MAXC));
                          if (!OTAEnd)
                            {
                              UpdateWiFiPower ();
                              GSRWiFi.Slow = 2;
                            }
                        }
                      else if (WiFi.getMode () == WIFI_AP)
                        {
                          WatchyGSR::StartWeb ();
                          Menu.SubItem++;
                          setStatus (String (WiFiTXT) + "AP");
                          APLoop = millis ();
                          GSRWiFi.Slow = 2;
                        }
                      else
                        Menu.SubItem = 0; // Fail, something is amiss.
                      break;
                    case 5: // SoftAP waiting to go off.
                      break;
                    default: // 2 to 5 is here.
                      if (Menu.SubItem > 1)
                        {
                          if (WiFi.getMode () == WIFI_STA)
                            {
                              Menu.SubItem = 0;
                              break;
                            }
                          server.handleClient ();
                          /*if (Server.handleRequests()){
                            Menu.SubItem = 0;
                            break;
                          }*/
                          if (millis () - APLoop > 8000)
                            {
                              Menu.SubItem = roller (Menu.SubItem + 1, 2, 4);
                              UpdateDisp |= Showing ();
                              APLoop = millis ();
                            }
                        }
                    }
                }

              if (Darkness.Went && Options.NeedsSaving)
                RecordSettings ();
              //              if (HWVer == 3.0f) CheckButtons();

              if (!Updates.Init && !Alarms.soundsFiring ()
                  && !(InTurbo () || B))
                DisplaySleep ();
            }
          /* WIFI */

          if (!UpdateDisp && GSRWiFi.Slow == 0)
            processWiFiRequest (); // Process any WiFi requests.

          SRTC.pauseDrift (CountdownTimerActive () || TimerUp.Active);
          Sensitive = (OTAUpdate && Menu.SubItem == 3);
          AlarmsOn = (Alarms.firing () || Alarms.soundsFiring ()
                      || !GetWebAvailable () || TimerAbuse ());
          ActiveMode
              = (InTurbo () || B || NTPData.State || WeatherData.State
                 || AlarmsOn || WatchyAPOn || OTAUpdate || GSRWiFi.Requested
                 || GSRWiFi.Requests || OTAEnd || Button); /* (B && Up) */
          if (Options.Game != 255)
            {
              if (WatchStyles.AddOn[Options.Game] != nullptr)
                ActiveMode |= WatchStyles.AddOn[Options.Game]->InsertNeedAwake (
                    !ActiveMode);
            }
          if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr)
            ActiveMode |= InsertNeedAwake (!ActiveMode);
          else
            ActiveMode
                |= WatchStyles.AddOn[Options.WatchFaceStyle]->InsertNeedAwake (
                    !ActiveMode);
          WatchTime.NewMinute = false;
          RefreshCPU (GSR_CPUDEF);
          Since = 50 - (millis () - Since);
          if (Since <= 50 && Since > 0)
            vTaskDelay (Since / portTICK_PERIOD_MS);
        }
      KeysCheckOn = false;
    }

  BrownOutDetect (true);
  KeysStop ();
  deepSleep ();
}

void
WatchyGSR::StartWeb ()
{
  /*return index page which is stored in basicIndex */
#if __cplusplus < 202002L
#ifdef _MSC_VER
#pragma message(                                                               \
    "msvc goes through this branch even in c++20 mode unless /Zc:__cplusplus is enabled")
#endif
  server.on ("/", HTTP_GET, [=] () {
#else
  server.on ("/", HTTP_GET, [=, this] () {
#endif
    server.sendHeader ("Connection", "close");
    server.send (200, "text/html", getBasicIndex ());
    ResetOTA ();
  });
#if __cplusplus < 202002L
  server.on ("/settings", HTTP_GET, [=] () {
#else
  server.on ("/settings", HTTP_GET, [=, this] () {
#endif
    String S = LGSR.LangString (settingsIndex, true, Options.LanguageID, 5, 6);
    S.replace ("{??}", GetSettings ());
    server.sendHeader ("Connection", "close");
    server.send (200, "text/html", S);
    ResetOTA ();
  });
#if __cplusplus < 202002L
  server.on ("/wifi", HTTP_GET, [=] () {
#else
  server.on ("/wifi", HTTP_GET, [=, this] () {
#endif
    server.sendHeader ("Connection", "close");
    server.send (200, "text/html", buildWiFiAPPage ());
    ResetOTA ();
  });
#if __cplusplus < 202002L
  server.on ("/update", HTTP_GET, [=] () {
#else
  server.on ("/update", HTTP_GET, [=, this] () {
#endif
    if (OTA ())
      {
        server.sendHeader ("Connection", "close");
        server.send (
            200, "text/html",
            LGSR.LangString (updateIndex, true, Options.LanguageID, 8, 13));
        ResetOTA ();
      }
  });
#if __cplusplus < 202002L
  server.on ("/ntp", HTTP_GET, [=] () {
#else
  server.on ("/ntp", HTTP_GET, [=, this] () {
#endif
    String S = LGSR.LangString (ntpIndex, true, Options.LanguageID, 6, 25);
    S.replace ("{%1%}", getNTPfromWeb ());
    S.replace ("{%2%}", getCurrentNTPServer ());
    server.sendHeader ("Connection", "close");
    server.send (200, "text/html", S);
    ResetOTA ();
  });
#if __cplusplus < 202002L
  server.on ("/weather", HTTP_GET, [=] () {
#else
  server.on ("/weather", HTTP_GET, [=, this] () {
#endif
    String S = LGSR.LangString (weatherIndex, true, Options.LanguageID, 6, 23);
    String T = (WeatherData.UseStaticPOS && WeatherData.StaticLat != NOLOC
                    ? makeGeo (String (WeatherData.StaticLat, 6), true)
                    : "");
    S.replace ("{%1%}", T);
    T = (WeatherData.UseStaticPOS && WeatherData.StaticLon != NOLOC
             ? makeGeo (String (WeatherData.StaticLon, 6), false)
             : "");
    S.replace ("{%2%}", T);
    server.sendHeader ("Connection", "close");
    server.send (200, "text/html", S);
    ResetOTA ();
  });
#if __cplusplus < 202002L
  server.on ("/", HTTP_POST, [=] () {
#else
  server.on ("/", HTTP_POST, [=, this] () {
#endif
    server.sendHeader ("Connection", "close");
    server.send (200, "text/html", getBasicIndex ());
    ResetOTA ();
  });
#if __cplusplus < 202002L
  server.on ("/exit", HTTP_GET, [=] () {
#else
  server.on ("/exit", HTTP_GET, [=, this] () {
#endif
    server.sendHeader ("Connection", "close");
    server.send (
        200, "text/html",
        LGSR.LangString (basicIndexDone, true, Options.LanguageID, 1, 21));
    if (WatchyAPOn && Menu.Item == GSR_MENU_WIFI)
      {
        Menu.SubItem = 5;
        UpdateDisp |= Showing ();
      }
    ResetOTA ();
    EndOTA (3);
  });
#if __cplusplus < 202002L
  server.on ("/settings", HTTP_POST, [=] () {
#else
  server.on ("/settings", HTTP_POST, [=, this] () {
#endif
    if (server.argName (0) == "settings")
      {
        StoreSettings (server.arg (0));
        if (Options.WatchFaceStyle > WatchStyles.Count - 1)
          Options.WatchFaceStyle = 0;
        Options.NeedsSaving = true;
        WatchFaceStart (Options.WatchFaceStyle, true);
      }
    server.sendHeader ("Connection", "close");
    server.send (
        200, "text/html",
        LGSR.LangString (settingsDone, true, Options.LanguageID, 5, 19));
    ResetOTA ();
  });
#if __cplusplus < 202002L
  server.on ("/wifi", HTTP_POST, [=] () {
#else
  server.on ("/wifi", HTTP_POST, [=, this] () {
#endif
    uint8_t I = 0;
    while (I < server.args ())
      {
        parseWiFiPageArg (server.argName (I), server.arg (I));
        I++;
      }
    server.sendHeader ("Connection", "close");
    server.send (200, "text/html",
                 LGSR.LangString (wifiDone, true, Options.LanguageID, 14, 19));
    Options.NeedsSaving = true;
    ResetOTA ();
  });
#if __cplusplus < 202002L
  server.on ("/ntp", HTTP_POST, [=] () {
#else
  server.on ("/ntp", HTTP_POST, [=, this] () {
#endif
    String S = "";
    if (server.argName (0) == "ntp")
      S = server.arg (0);
    server.sendHeader ("Connection", "close");
    ResetOTA ();
    Options.NeedsSaving = (S != getNTPfromWeb ());
    setNTPfromWeb (S);
    server.send (200, "text/html",
                 LGSR.LangString (ntpDone, true, Options.LanguageID, 19, 26));
  });
#if __cplusplus < 202002L
  server.on ("/weather", HTTP_POST, [=] () {
#else
  server.on ("/weather", HTTP_POST, [=, this] () {
#endif
    String S = "";
    if (server.argName (0) == "lat")
      S = makeGeo (server.arg (0), true);
    String T = "";
    if (server.argName (1) == "lon")
      T = makeGeo (server.arg (1), false);
    server.sendHeader ("Connection", "close");
    ResetOTA ();
    WeatherData.UseStaticPOS = (T > "" && S > "" && T.toDouble () != NOLOC
                                && S.toDouble () != NOLOC);
    if (WeatherData.UseStaticPOS)
      {
        WeatherData.StaticLat = S.toDouble ();
        WeatherData.StaticLon = T.toDouble ();
      }
    else
      {
        WeatherData.StaticLat = NOLOC;
        WeatherData.StaticLon = NOLOC;
      }
    Options.NeedsSaving = true;
    server.send (
        200, "text/html",
        LGSR.LangString (weatherDone, true, Options.LanguageID, 1, 21));
  });
#if __cplusplus < 202002L
  server.on (
      "/update", HTTP_POST,
      [=] () {
#else
  server.on (
      "/update", HTTP_POST,
      [=, this] () {
#endif
        server.sendHeader ("Connection", "close");
        server.send (200, "text/plain",
                     (Update.hasError ()) ? "Upload Failed."
                                          : "Watchy will reboot!");
        Reboot ();
#if __cplusplus < 202002L
      },
      [=] () {
#else
      },
      [=, this] () {
#endif
        HTTPUpload &upload = server.upload ();
        if (upload.status == UPLOAD_FILE_START)
          {
            ResetOTA ();

            if (!Update.begin (UPDATE_SIZE_UNKNOWN))
              { // start with max available size
                OTAEnd = true;
              }
          }
        else if (upload.status == UPLOAD_FILE_WRITE)
          {
            /* flashing firmware to ESP*/

            if (Update.write (upload.buf, upload.currentSize)
                != upload.currentSize)
              OTAEnd = true;
          }
        else if (upload.status == UPLOAD_FILE_END)
          {
            if (Update.end (true))
              { // true to set the size to the current progress
                OTAEnd = true;
              }
          }
      });
  RefreshCPU (GSR_CPUMAX);
  server.begin ();
}
void
WatchyGSR::handleButtonPress (uint8_t Pressed)
{
  uint8_t I, b;
  uint16_t d;
  int ml, mh;
  int32_t Diff;
  String S = "";
  if (Darkness.Went && Options.SleepStyle == 4 && !Updates.Tapped)
    return; // No buttons unless a tapped happened.
  if (!UpRight ())
    return; // Don't do buttons if not upright.
  if (Pressed < 5 && LastButton > 0 && (millis () - LastButton) < Debounce ())
    return;
  if (Darkness.Went && !Darkness.Woke)
    {
      Darkness.Woke = true;
      Darkness.Last = millis ();
      Darkness.Tilt = Darkness.Last;
      UpdateUTC ();
      UpdateClock ();
      UpdateDisp = true;
      return;
    } // Don't do the button, just exit.
  if ((OTAUpdate) && (Pressed == 3 || Pressed == 4))
    return; // Up/Down don't work in these modes.

  switch (Pressed)
    {
    case 1:
      if (GuiMode != GSR_MENUON && GuiMode != GSR_GAMEON && !GetMenuOverride ())
        { // If MenuOverride is on, it will not let the menu work, meaning,
          // unless it is open already, it won't open.
          GuiMode = GSR_MENUON;
          DoHaptic = true;
          UpdateDisp = true; // Quick Update.
          SetTurbo ();
        }
      else if (GuiMode == GSR_MENUON)
        {
          if (Menu.Item == GSR_MENU_OPTIONS && Menu.SubItem == 0)
            { // Options
              Menu.Item = GSR_MENU_STYL;
              Menu.Style = GSR_MENU_INOPTIONS;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_TRBL && Menu.SubItem == 0)
            { // Troubleshooting.
              Menu.Style = GSR_MENU_INTROUBLE;
              Menu.Item = GSR_MENU_SCRN;
              DoHaptic = true;
              UpdateDisp = true;
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_STEPS)
            { // Steps
              if (Menu.SubItem == 4)
                {
#ifdef STABLEBMA_H_INCLUDED
                  SBMA.resetStepCounter ();
#endif
                  Steps.Cached = 0;
                  Menu.SubItem = 0;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem < 4)
                {
                  Menu.SubItem++;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_ALARMS)
            { // Alarms menu.
              Menu.Style = GSR_MENU_INALARMS;
              Menu.Item = GSR_MENU_ALARM1;
              DoHaptic = true;
              UpdateDisp = true;
              SetTurbo ();
            }
          else if (Menu.Item >= GSR_MENU_ALARM1 && Menu.Item <= GSR_MENU_ALARM8)
            { // Alarms
              if (Menu.SubItem < 5)
                {
                  if (!Menu.SubItem
                      && (Alarms.getPlaying (Menu.Item - GSR_MENU_ALARM1)
                          + Alarms.getTimes (Menu.Item - GSR_MENU_ALARM1)))
                    Alarms.setCancelled (Menu.Item - GSR_MENU_ALARM1, true);
                  if (!Menu.SubItem)
                    Alarms.setEditMode (Menu.Item - GSR_MENU_ALARM1, true);
                  Menu.SubItem++;
                  if (Menu.SubItem == 5)
                    Menu.SubItem
                        += WatchTime.Local.Wday; // Jump ahead to the day.
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem > 4 && Menu.SubItem < 12)
                {
                  d = Alarms.getActive (Menu.Item - GSR_MENU_ALARM1);
                  d ^= Bits[Menu.SubItem - 5]; // Toggle day.
                  Alarms.setActive (Menu.Item - GSR_MENU_ALARM1, d);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 12)
                {
                  d = Alarms.getActive (Menu.Item - GSR_MENU_ALARM1);
                  d ^= GSRALARM_REPEAT; // Toggle repeat.
                  Alarms.setActive (Menu.Item - GSR_MENU_ALARM1, d);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 13)
                {
                  d = Alarms.getActive (Menu.Item - GSR_MENU_ALARM1);
                  d ^= GSRALARM_ACTIVE; // Toggle Active.
                  Alarms.setActive (Menu.Item - GSR_MENU_ALARM1, d);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_TONES)
            { // Tones.
              Options.MasterRepeats = roller (Options.MasterRepeats + 1, 0, 4);
              for (b = 0; b < 8; b++)
                {
                  Alarms.setRepeats (b, Options.MasterRepeats);
                }
              Options.NeedsSaving = true;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_TIMERS)
            { // Timers menu.
              Menu.Style = GSR_MENU_INTIMERS;
              Menu.Item = GSR_MENU_TIMEDN;
              DoHaptic = true;
              UpdateDisp = true;
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_TIMEDN)
            {
              if (Menu.SubItem == 6)
                {
                  if (CountdownTimerActive ())
                    {
                      Alarms.setTimerDownActive ();
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                    }
                  else if ((Alarms.getTimerDownMaxSeconds ()
                            + Alarms.getTimerDownMaxMinutes ()
                            + Alarms.getTimerDownMaxHours ())
                           > 0)
                    {
                      Alarms.StartCD ();
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                    }
                }
              else
                {
                  Menu.SubItem++;
                  if (Alarms.getTimerDownMaxSeconds ()
                              + Alarms.getTimerDownMaxMinutes ()
                              + Alarms.getTimerDownMaxHours ()
                          == 0
                      && Menu.SubItem == 6)
                    Menu.SubItem = 4; // Stop it from being startable.
                  if (CountdownTimerActive ())
                    Menu.SubItem = 6;
                  DoHaptic = true;
                  UpdateDisp = !CountdownTimerActive (); // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_TMEDCF)
            {
              if (Menu.SubItem < 2)
                {
                  Menu.SubItem++;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_TIMEUP)
            {
              if (Menu.SubItem == 0)
                {
                  Menu.SubItem = 1;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else
                {
                  if (TimerUp.Active)
                    {
                      UpdateUTC ();
                      TimerUp.StopAt = WatchTime.UTC_RAW;
                      TimerUp.Active = false;
                    }
                  else
                    {
                      UpdateUTC ();
                      TimerUp.SetAt = WatchTime.UTC_RAW;
                      TimerUp.StopAt = TimerUp.SetAt;
                      TimerUp.Active = true;
                    }
                  DoHaptic = true;
                  UpdateDisp = !TimerUp.Active; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_GAME)
            {
              if (Menu.SubItem == 0)
                {
                  Menu.SubItem++;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else
                {
                  ShowGame ();
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_TPWR
                   && !(GSRWiFi.Requests > 0 || WatchyAPOn))
            {
              I = roller (getTXOffset (GSRWiFi.TransmitPower) + 1, 0, 10);
              GSRWiFi.TransmitPower = RawWiFiTX[I];
              Options.NeedsSaving = true;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_INFO)
            { // Information
              Menu.SubItem = roller (Menu.SubItem + 1, 0, 2);
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_SAVE)
            { // Battery Saver.
              Options.Performance = roller (Options.Performance + 1, 0, 2);
              Options.NeedsSaving = true;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_SYNC)
            { // Sync Time
              if (Menu.SubItem == 4)
                {
                  Options.ntpAtCharge = !Options.ntpAtCharge;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 0 || Menu.SubItem > 4)
                { // Start and Sync Menu
                  if (Menu.SubItem < 16)
                    {
                      Menu.SubItem++;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                    }
                }
              else
                {
                  switch (Menu.SubItem)
                    {
                    case 1:
                      NTPData.UpdateUTC = true;
                      NTPData.TimeZone
                          = (OP.getCurrentPOSIX ()
                             == OP.TZMISSING); // No Timezone, go get one!
                      break;
                    case 2:
                      NTPData.UpdateUTC = false;
                      NTPData.TimeZone = true;
                      break;
                    case 3:
                      NTPData.TimeZone = true;
                      NTPData.UpdateUTC = true;
                      break;
                    }
                  GuiMode = GSR_WATCHON;
                  Menu.Item = GSR_MENU_STYL;
                  Menu.SubItem = 0;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  StartNTP (NTPData.UpdateUTC, NTPData.TimeZone);
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_WEAT)
            { // Weather
              if (Menu.SubItem < 4)
                {
                  if (Menu.SubItem == 0
                      || (WeatherData.Interval == 0 && Menu.SubItem < 2))
                    Menu.SubItem = 2;
                  else
                    Menu.SubItem++;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 4)
                {
                  StartWeather ();
                  Menu.SubItem = 0;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_STYL)
            {                     // Switch Watch Face
              ChangeWatchFace (); /* Defaults to TRUE to go up */
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_LANG)
            { // Language switch.
              I = roller (Options.LanguageID + 1, 0, LGSR.MaxLangID ());
              if (I != Options.LanguageID)
                {
                  Options.LanguageID = I;
                  initWatchFaceStyle (); // If fonts will need to be changed per
                                         // language down the road, I don't
                                         // know.
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_DISP)
            { // Switch Mode
              Options.LightMode = !Options.LightMode;
              Options.NeedsSaving = true;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_SIDE)
            { // Dexterity Mode
              Options.Lefty = !Options.Lefty;
              Options.NeedsSaving = true;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_SWAP)
            { // Swap Menu/Back Buttons
              Options.Swapped = !Options.Swapped;
              Options.NeedsSaving = true;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_BRDR)
            { // Border Mode
#ifdef GxEPD2DarkBorder
              Options.Border = !Options.Border;
              Options.NeedsSaving = true;
              Updates.Init = true;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
#endif
            }
          else if (Menu.Item == GSR_MENU_ORNT)
            { // Watchy Orientation
              Options.Orientated = !Options.Orientated;
              Options.NeedsSaving = true;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_MODE)
            { // Switch Time Mode
              Options.TwentyFour = !Options.TwentyFour;
              Options.NeedsSaving = true;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_FEED)
            { // Feedback.
              Options.Feedback = !Options.Feedback;
              Options.NeedsSaving = true;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_TRBO)
            { // Turbo
              Options.Turbo = roller (Options.Turbo + 1, 0, 10);
              Options.NeedsSaving = true;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_DARK)
            { // Sleep Mode.
              if (Menu.SubItem < 5)
                {
                  Menu.SubItem++;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_SCRN)
            { // Reset Screen
              GuiMode = GSR_WATCHON;
              Updates.Full = true;
              UpdateDisp = true; // Quick Update.
              DoHaptic = true;
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_WIFI && !WatchyAPOn)
            { // Watchy Connect
              Menu.SubItem++;
              WatchyAPOn = true;
              ResetEndOTA ();
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if ((Menu.Item == GSR_MENU_OTAU || Menu.Item == GSR_MENU_OTAM)
                   && !(GSRWiFi.Requests > 0 || WatchyAPOn))
            { // Watchy OTA
              Menu.SubItem++;
              OTAUpdate = true;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_RSET)
            { // Watchy Reboot
              if (Menu.SubItem == 1)
                Reboot ();
              else
                Menu.SubItem++;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_TOFF && NTPData.State == 0)
            { // Drift Management
              /*
               * S Auto H:MM:SS Begin/Calculate/Current/Toggle RTC/Adjust Drift
               * 0 1    2 34 56 7     8         9       10            12
               */
              /* DO SETUP FOR Edit RTC MENU */
              if (Menu.SubItem < 7)
                {
                  Menu.SubItem++;
                  if (Menu.SubItem == 2 && Options.NTPAuto)
                    Menu.SubItem = 7;
                  if (Menu.SubItem == 7)
                    {
                      if (SRTC.checkingDrift (WatchTime.ESPRTC))
                        Menu.SubItem++; // Move to Calculate.
                      else if (SRTC.getDrift (WatchTime.ESPRTC) != 0)
                        Menu.SubItem = 9; // Move to show the value (so you
                                          // don't ruin it).
                    }
                  DoHaptic = true;
                  UpdateDisp = (Menu.SubItem > 6
                                || Menu.SubItem == 1); // Quick Update. */
                  SetTurbo ();
                }
              else if (Menu.SubItem == 7)
                { // Drift Begin
                  if (Options.NTPAuto)
                    {
                      NTPData.driftSync = true;
                      NTPData.driftSyncNOW = 0;
                      StartNTP (true);
                      Menu.SubItem = 0;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                    }
                  else
                    {
                      UpdateUTC ();
                      SRTC.setDrift (0, false, WatchTime.ESPRTC);
                      WatchTime.UTC_RAW += NTPData.TravelTemp;
                      NTPData.TravelTemp = 0;
                      SRTC.doBreakTime (WatchTime.UTC_RAW, WatchTime.UTC);
                      SRTC.beginDrift (WatchTime.UTC, WatchTime.ESPRTC);
                      UpdateUTC ();
                      UpdateClock ();
                      Menu.SubItem = 0;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                    }
                }
              else if (Menu.SubItem == 8 && !Options.NTPAuto)
                { // Drift Finish
                  UpdateUTC ();
                  Diff = 0;
                  WatchTime.UTC_RAW += NTPData.TravelTemp;
                  SRTC.doBreakTime (WatchTime.UTC_RAW, WatchTime.UTC);
                  SRTC.endDrift (WatchTime.UTC, WatchTime.ESPRTC);
                  UpdateUTC ();
                  UpdateClock ();
                  Options.NeedsSaving = true;
                  NTPData.TravelTemp = 0;
                  Menu.SubItem = 0;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 9)
                { // Drift Value pushing Menu to edit.
                  Menu.SubItem = 12;
                  Menu.SubSubItem = 0;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 10 && HWVer != 3.0f)
                {
                  WatchTime.ESPRTC = !WatchTime.ESPRTC;
                  SRTC.useESP32 (WatchTime.ESPRTC);
                  UpdateUTC ();
                  UpdateClock ();
                  Options.NeedsSaving = true;
                  Menu.SubItem = 0;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 12)
                { // Drift Value MENU to advance to next digit.
                  S = String (SRTC.getDrift (WatchTime.ESPRTC));
                  if (Menu.SubSubItem + 1 < S.length ())
                    {
                      Menu.SubSubItem++;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                    }
                }
            }
          else if (Menu.Item == GSR_MENU_BERR && Menu.SubItem == 0)
            { // Battery Error
              Menu.SubItem++;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_UNVS)
            { // USE NVS
              if (Menu.SubItem == 0)
                Menu.SubItem++;
              else if (Menu.SubItem == 1)
                {
                  if (Menu.SubSubItem == 1)
                    {
                      if (OkNVS (GSR_GName))
                        Menu.SubItem == 2; // delete, request
                      else
                        {
                          SetNVS (GSR_GName, true);
                          Menu.SubItem = 0;
                          Menu.SubSubItem = 0;
                        }
                    }
                  else
                    Menu.SubItem = 0; // Keep, don't change.
                }
              else if (Menu.SubItem == 2)
                {
                  NVSEmpty ();
                  Reboot ();
                }
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
        }
      else
        Missed = 1;
      break;
    case 2:
      if (GuiMode == GSR_MENUON)
        { // Back Button [SW2]
          if (Menu.Item == GSR_MENU_STEPS && Menu.SubItem > 0)
            { // Exit for Steps, back to Steps.
              if (Menu.SubItem == 4)
                Menu.SubItem = 2; // Go back to the Hour, so it is the same as
                                  // the Alarms.
              Menu.SubItem--;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item >= GSR_MENU_ALARM1 && Menu.Item <= GSR_MENU_ALARM8
                   && Menu.SubItem > 0)
            {
              if (Menu.SubItem < 5 && Menu.SubItem > 0)
                {
                  Menu.SubItem--;
                  if (!Menu.SubItem)
                    Alarms.setEditMode (Menu.Item - GSR_MENU_ALARM1, false);
                }
              else if (Menu.SubItem > 4)
                {
                  Menu.SubItem = 1;
                }
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_TIMEDN && Menu.SubItem > 0)
            {
              Menu.SubItem--;
              if (CountdownTimerActive ())
                Menu.SubItem = 0;
              DoHaptic = true;
              UpdateDisp = !CountdownTimerActive (); // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_TMEDCF && Menu.SubItem > 0)
            {
              Menu.SubItem--;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_GAME && Menu.SubItem > 0)
            {
              Menu.SubItem--;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_TIMEUP && Menu.SubItem > 0)
            {
              Menu.SubItem = 0;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_GAME && Menu.SubItem > 0)
            {
              Menu.SubItem--;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_DARK && Menu.SubItem > 0)
            { // Sleep Mode.
              Menu.SubItem--;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_SYNC && Menu.SubItem > 0)
            {
              if (Menu.SubItem == 6)
                Menu.SubItem = 5;
              else if (Menu.SubItem > 5 && Menu.SubItem < 10)
                Menu.SubItem = 6;
              else if (Menu.SubItem > 9)
                Menu.SubItem--;
              else
                Menu.SubItem = 0;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_WEAT && Menu.SubItem > 0)
            { // Weather
              Menu.SubItem--;
              if (Menu.SubItem == 1
                  && ((WeatherData.Interval * 300) / 3600) == 0)
                Menu.SubItem--;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_WIFI && Menu.SubItem > 0)
            {
              if (Menu.SubItem < 5)
                {
                  EndOTA (3);
                  Menu.SubItem = 5;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if ((Menu.Item == GSR_MENU_OTAU || Menu.Item == GSR_MENU_OTAM)
                   && Menu.SubItem > 0)
            {
              break; // DO NOTHING!
            }
          else if (Menu.Style == GSR_MENU_INALARMS)
            { // Alarms
              Menu.Style = GSR_MENU_INNORMAL;
              Menu.Item = GSR_MENU_ALARMS;
              DoHaptic = true;
              UpdateDisp = true;
              SetTurbo ();
            }
          else if (Menu.Style == GSR_MENU_INTIMERS)
            { // Timers
              Menu.Style = GSR_MENU_INNORMAL;
              Menu.Item = GSR_MENU_TIMERS;
              DoHaptic = true;
              UpdateDisp = true;
              SetTurbo ();
            }
          else if (Menu.Style == GSR_MENU_INOPTIONS)
            { // Options
              Menu.SubItem = 0;
              Menu.SubSubItem = 0;
              Menu.Item = GSR_MENU_OPTIONS;
              Menu.Style = GSR_MENU_INNORMAL;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Style == GSR_MENU_INTROUBLE && Menu.SubItem == 0)
            { // Troubleshooting.
              Menu.SubItem = 0;
              Menu.SubSubItem = 0;
              Menu.Item = GSR_MENU_TRBL;
              Menu.Style = GSR_MENU_INOPTIONS;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_RSET && Menu.SubItem > 0)
            { // Watchy Reboot
              Menu.SubItem--;
            }
          else if (Menu.Item == GSR_MENU_TOFF && Menu.SubItem > 0)
            { // Drift Travel
              /*
               * S Auto H:MM:SS Begin/Calculate/Current/Toggle RTC/Adjust Drift
               * 0 1    2 34 56 7     8         9       10            12
               */
              /* DO SETUP FOR Edit RTC BACK */
              if (Menu.SubItem == 12)
                { // Drift Value pushing Back to exit.
                  if (Menu.SubSubItem == 0)
                    Menu.SubItem = 9;
                  else
                    Menu.SubSubItem--;
                }
              else if (Menu.SubItem > 6)
                Menu.SubItem = 6;
              else
                Menu.SubItem--;
              if (Menu.SubItem < 7 && Menu.SubItem > 0 && Options.NTPAuto)
                Menu.SubItem = 1;
              UpdateDisp = true; // Quick Update.
              DoHaptic = true;
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_BERR && Menu.SubItem > 0)
            { // Battery Error
              Menu.SubItem--;
              UpdateDisp = true; // Quick Update.
              DoHaptic = true;
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_UNVS)
            { // USE NVS
              Menu.SubItem = 0;
              Menu.SubSubItem = 0;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else
            {
              GuiMode = GSR_WATCHON;
              Menu.SubItem = 0;
              Menu.SubSubItem = 0;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
        }
      else
        Missed = 2; // Missed a SW2.
      break;
    case 3:
      if (GuiMode == GSR_MENUON)
        { // Up Button [SW3]
          // Handle the sideways choices here.
          if (Menu.Item == GSR_MENU_STEPS && Menu.SubItem > 0)
            {
              switch (Menu.SubItem)
                {
                case 1: // Hour
                  Steps.Hour = roller (Steps.Hour + 1, 0, 23);
                  Options.NeedsSaving = true;
                  Steps.Reset = false;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                  break;
                case 2: //  x0 Minutes
                  mh = (Steps.Minutes / 10);
                  ml = Steps.Minutes - (mh * 10);
                  mh = roller (mh + 1, 0, 5);
                  Steps.Minutes = (mh * 10) + ml;
                  Steps.Reset = false;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                  break;
                case 3: //  x0 Minutes
                  mh = (Steps.Minutes / 10);
                  ml = Steps.Minutes - (mh * 10);
                  ml = roller (ml + 1, 0, 9);
                  Steps.Minutes = (mh * 10) + ml;
                  Steps.Reset = false;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item >= GSR_MENU_ALARM1 && Menu.Item <= GSR_MENU_ALARM8
                   && Menu.SubItem > 0)
            {
              if (Menu.SubItem == 1)
                { // Hour
                  Alarms.setHour (
                      Menu.Item - GSR_MENU_ALARM1,
                      roller (Alarms.getHour (Menu.Item - GSR_MENU_ALARM1) + 1,
                              0, 23));
                  d = Alarms.getActive (Menu.Item - GSR_MENU_ALARM1);
                  d &= GSRALARM_NOTRIGGER;
                  Alarms.setActive (Menu.Item - GSR_MENU_ALARM1, d);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 2)
                { //  x0 Minutes
                  b = Alarms.getMinutes (Menu.Item - GSR_MENU_ALARM1);
                  mh = (b / 10);
                  ml = b - (mh * 10);
                  mh = roller (mh + 1, 0, 5);
                  Alarms.setMinutes (Menu.Item - GSR_MENU_ALARM1,
                                     (mh * 10) + ml);
                  d = Alarms.getActive (Menu.Item - GSR_MENU_ALARM1);
                  d &= GSRALARM_NOTRIGGER;
                  Alarms.setActive (Menu.Item - GSR_MENU_ALARM1, d);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 3)
                { //  x0 Minutes
                  b = Alarms.getMinutes (Menu.Item - GSR_MENU_ALARM1);
                  mh = (b / 10);
                  ml = b - (mh * 10);
                  ml = roller (ml + 1, 0, 9);
                  Alarms.setMinutes (Menu.Item - GSR_MENU_ALARM1,
                                     (mh * 10) + ml);
                  b = Alarms.getActive (Menu.Item - GSR_MENU_ALARM1);
                  b &= GSRALARM_NOTRIGGER;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 4)
                { //  Repeats.
                  Alarms.setRepeats (
                      Menu.Item - GSR_MENU_ALARM1,
                      roller (Alarms.getRepeats (Menu.Item - GSR_MENU_ALARM1)
                                  - 1,
                              0, 4));
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem > 4)
                {
                  Menu.SubItem = roller (Menu.SubItem + 1, 5, 13);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_TIMEDN && Menu.SubItem > 0)
            {
              switch (Menu.SubItem)
                {
                case 1: // Hour
                  Alarms.setTimerDownMaxHours (
                      roller (Alarms.getTimerDownMaxHours () + 1, 0, 23));
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                  break;
                case 2: //  x0 Minutes
                  b = Alarms.getTimerDownMaxMinutes ();
                  mh = (b / 10);
                  ml = b - (mh * 10);
                  mh = roller (mh + 1, 0, 5);
                  Alarms.setTimerDownMaxMinutes ((mh * 10) + ml);
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !CountdownTimerActive (); // Quick Update.
                  SetTurbo ();
                  break;
                case 3: //  x0 Minutes
                  b = Alarms.getTimerDownMaxMinutes ();
                  mh = (b / 10);
                  ml = b - (mh * 10);
                  ml = roller (ml + 1, 0, 9);
                  Alarms.setTimerDownMaxMinutes ((mh * 10) + ml);
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !CountdownTimerActive (); // Quick Update.
                  SetTurbo ();
                  break;
                case 4: //  x0 Seconds
                  b = Alarms.getTimerDownMaxSeconds ();
                  mh = (b / 10);
                  ml = b - (mh * 10);
                  mh = roller (mh + 1, 0, 5);
                  Alarms.setTimerDownMaxSeconds ((mh * 10) + ml);
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !CountdownTimerActive (); // Quick Update.
                  SetTurbo ();
                  break;
                case 5: //  x0 Seconds
                  b = Alarms.getTimerDownMaxSeconds ();
                  mh = (b / 10);
                  ml = b - (mh * 10);
                  ml = roller (ml + 1, 0, 9);
                  Alarms.setTimerDownMaxSeconds ((mh * 10) + ml);
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !CountdownTimerActive (); // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_TMEDCF && Menu.SubItem > 0)
            {
              switch (Menu.SubItem)
                {
                case 1: // Repeatable
                  Alarms.setTimerDownRepeats (!Alarms.getTimerDownRepeats ());
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                  break;
                case 2: // Tone Repeats
                  Alarms.setTimerDownMaxTones (
                      roller (Alarms.getTimerDownMaxTones () - 1, 0, 4));
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_GAME && Menu.SubItem > 0)
            {
              if (ChangeGame ())
                {
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_DARK && Menu.SubItem > 0)
            { // Sleep Mode.
              switch (Menu.SubItem)
                {
                case 1: // Style.
#ifdef STABLEBMA_H_INCLUDED
                  Options.SleepStyle = roller (Options.SleepStyle + 1, 0, 4);
#else
                  if (Options.SleepStyle != 2)
                    Options.SleepStyle = 2;
                  else
                    Options.SleepStyle = 0;
#endif
                  Updates.Tapped = true;
                  Options.NeedsSaving = true;
                  break;
                case 2: // SleepMode (0=off, 10 seconds)
                  Options.SleepMode = roller (Options.SleepMode + 1, 1, 10);
                  Options.NeedsSaving = true;
                  break;
                case 3: // SleepStart (hour)
                  if (Options.SleepStart + 1 != Options.SleepEnd)
                    {
                      Options.SleepStart
                          = roller (Options.SleepStart + 1, 0, 23);
                      Options.NeedsSaving = true;
                    }
                  else
                    return;
                  break;
                case 4: // SleepStart (hour)
                  if (Options.SleepEnd + 1 != Options.SleepStart)
                    {
                      Options.SleepEnd = roller (Options.SleepEnd + 1, 0, 23);
                      Options.NeedsSaving = true;
                    }
                  else
                    return;
                  break;
                case 5: // Orientation.
                  Options.BedTimeOrientation = !Options.BedTimeOrientation;
                  Options.NeedsSaving = true;
                }
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_SYNC && Menu.SubItem > 0)
            {
              if (Menu.SubItem > 5)
                {
                  /* On HH:MM
                     6  78 9A */

                  switch (Menu.SubItem)
                    {
                    case 6: // On/Off
                      NTPData.AutoSync = !NTPData.AutoSync;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                      break;
                    case 7: // Hour
                      NTPData.SyncHour = roller (NTPData.SyncHour + 1, 0, 23);
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                      break;
                    case 8: //  x0 Minutes
                      mh = (NTPData.SyncMins / 10);
                      ml = NTPData.SyncMins - (mh * 10);
                      mh = roller (mh + 1, 0, 5);
                      NTPData.SyncMins = (mh * 10) + ml;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                      break;
                    case 9: //  x0 Minutes
                      mh = (NTPData.SyncMins / 10);
                      ml = NTPData.SyncMins - (mh * 10);
                      ml = roller (ml + 1, 0, 9);
                      NTPData.SyncMins = (mh * 10) + ml;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                      break;
                    case 10: // Sunday.
                    case 11: // Monday.
                    case 12: // Tuesday.
                    case 13: // Wednesday.
                    case 14: // Thursday.
                    case 15: // Friday.
                    case 16: // Saturday.
                      I = NTPData.SyncDays ^ Bits[Menu.SubItem - 10];
                      if (I)
                        {
                          NTPData.SyncDays = I;
                          Options.NeedsSaving = true;
                          DoHaptic = true;
                          UpdateDisp = true; // Quick Update.
                          SetTurbo ();
                        }
                    }
                }
              else
                {
                  Menu.SubItem = roller (Menu.SubItem - 1, 1, 5);
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_WEAT && Menu.SubItem > 0)
            { // Weather
              if (Menu.SubItem == 3)
                SetWeatherScale (!IsMetric ());
              else if (Menu.SubItem < 3)
                {
                  I = (Menu.SubItem == 1 ? 12 : 1);
                  WeatherData.Interval
                      = roller (WeatherData.Interval + I, 0, 144);
                  WeatherData.LastCall
                      = WatchTime.UTC_RAW - (WatchTime.UTC_RAW % 60)
                        + (WeatherData.Interval * 300); // 15 minutes.
                }
              if (Menu.SubItem < 4)
                {
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_WIFI && Menu.SubItem > 0)
            {
              // Do nothing!
            }
          else if (Menu.Item == GSR_MENU_TOFF && Menu.SubItem > 0)
            {
              /*
               * S Auto H:MM:SS Begin/Calculate/Current/Toggle RTC/Adjust Drift
               * 0 1    2 34 56 7     8         9       10            12
               */
              /* DO SETUP FOR Edit RTC UP */
              if (Menu.SubItem == 1)
                {
                  Options.NTPAuto = !Options.NTPAuto;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem < 7)
                {
                  NTPData.TravelTemp += ((Menu.SubItem == 2 ? 3600 : 0)
                                         + (Menu.SubItem == 3 ? 600 : 0)
                                         + (Menu.SubItem == 4 ? 60 : 0)
                                         + (Menu.SubItem == 5 ? 10 : 0)
                                         + (Menu.SubItem == 6 ? 1 : 0));
                  DoHaptic = true;
                  /* TEST  UpdateDisp = true;  // Quick Update.*/
                  SetTurbo ();
                }
              else if (Menu.SubItem == 12)
                { // Drift Value edit..
                  Diff = SRTC.getDrift (WatchTime.ESPRTC);
                  S = String (Diff);
                  Diff += pow (10, (S.length () - (1 + Menu.SubSubItem)));
                  if (Diff < 0xFFFFFF)
                    {
                      SRTC.setDrift (Diff, SRTC.isFastDrift (WatchTime.ESPRTC),
                                     WatchTime.ESPRTC);
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                    }
                }
              else
                {
                  Menu.SubItem = roller (Menu.SubItem + 1, 7, 10);
                  if (Menu.SubItem == 8
                      && !SRTC.checkingDrift (WatchTime.ESPRTC))
                    Menu.SubItem++;
                  if (Menu.SubItem == 9
                      && SRTC.getDrift (WatchTime.ESPRTC) == 0)
                    Menu.SubItem++;
                  // if (WatchTime.ESPRTC) Menu.SubItem == 9;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_BERR && Menu.SubItem == 1)
            { // Battery Error
              if (Battery.LowError < 0.1)
                {
                  Battery.LowError += 0.01;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_UNVS && Menu.SubItem == 1)
            { // USE NVS
              if (Menu.SubSubItem == 1)
                {
                  Menu.SubSubItem = 0;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              return;
            }
          else
            {
              if (Menu.Style == GSR_MENU_INOPTIONS)
                {
                  Menu.Item
                      = roller (Menu.Item - 1, GSR_MENU_STYL,
                                (Watchy_Chip_Info.HasWiFi
                                 & (NTPData.State > 0 || WatchyAPOn || OTAUpdate
                                    || Battery.Read < getLowBatteryRadio ()))
                                    ? GSR_MENU_TRBL
                                    : GSR_MENU_OTAM);
                  if (Menu.Item == GSR_MENU_WEAT && !GetWantWeather ())
                    Menu.Item = roller (
                        Menu.Item - 1, GSR_MENU_STYL,
                        (Watchy_Chip_Info.HasWiFi
                         & (NTPData.State > 0 || WatchyAPOn || OTAUpdate
                            || Battery.Read < getLowBatteryRadio ()))
                            ? GSR_MENU_TRBL
                            : GSR_MENU_OTAM);
                }
              else if (Menu.Style == GSR_MENU_INALARMS)
                {
                  Menu.Item
                      = roller (Menu.Item - 1, GSR_MENU_ALARM1, GSR_MENU_TONES);
                }
              else if (Menu.Style == GSR_MENU_INTIMERS)
                {
                  Menu.Item = roller (Menu.Item - 1, GSR_MENU_TIMEDN,
                                      GSR_MENU_TIMEUP);
                }
              else if (Menu.Style == GSR_MENU_INTROUBLE)
                {
                  Menu.Item
                      = roller (Menu.Item - 1, GSR_MENU_SCRN, GSR_MENU_UNVS);
                }
              else
                {
#ifndef STABLEBMA_H_INCLUDED
                  Menu.Item = roller (Menu.Item - 1, GSR_MENU_ALARMS,
                                      GSR_MENU_OPTIONS);
                  if (Menu.Item == GSR_MENU_GAME && Options.Game == 255)
                    Menu.Item = roller (Menu.Item - 1, GSR_MENU_ALARMS,
                                        GSR_MENU_OPTIONS);
#else
                  Menu.Item = roller (Menu.Item - 1, GSR_MENU_STEPS,
                                      GSR_MENU_OPTIONS);
                  if (Menu.Item == GSR_MENU_GAME && Options.Game == 255)
                    Menu.Item = roller (Menu.Item - 1, GSR_MENU_STEPS,
                                        GSR_MENU_OPTIONS);
#endif
                }
              Menu.SubItem = 0;
              Menu.SubSubItem = 0;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
        }
      else
        Missed = 3; // Missed a SW3.
      break;
    case 4:
      if (GuiMode == GSR_MENUON)
        { // Down Button [SW4]
          // Handle the sideways choices here.
          if (Menu.Item == GSR_MENU_STEPS && Menu.SubItem > 0)
            {
              switch (Menu.SubItem)
                {
                case 1: // Hour
                  Steps.Hour = roller (Steps.Hour - 1, 0, 23);
                  Options.NeedsSaving = true;
                  Steps.Reset = false;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                  break;
                case 2: //  x0 Minutes
                  mh = (Steps.Minutes / 10);
                  ml = Steps.Minutes - (mh * 10);
                  mh = roller (mh - 1, 0, 5);
                  Steps.Minutes = (mh * 10) + ml;
                  Steps.Reset = false;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                  break;
                case 3: //  x0 Minutes
                  mh = (Steps.Minutes / 10);
                  ml = Steps.Minutes - (mh * 10);
                  ml = roller (ml - 1, 0, 9);
                  Steps.Minutes = (mh * 10) + ml;
                  Steps.Reset = false;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item >= GSR_MENU_ALARM1 && Menu.Item <= GSR_MENU_ALARM8
                   && Menu.SubItem > 0)
            {
              if (Menu.SubItem == 1)
                { // Hour
                  Alarms.setHour (
                      Menu.Item - GSR_MENU_ALARM1,
                      roller (Alarms.getHour (Menu.Item - GSR_MENU_ALARM1) - 1,
                              0, 23));
                  d = Alarms.getActive (Menu.Item - GSR_MENU_ALARM1);
                  d &= GSRALARM_NOTRIGGER;
                  Alarms.setActive (Menu.Item - GSR_MENU_ALARM1, d);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 2)
                { //  x0 Minutes
                  b = Alarms.getMinutes (Menu.Item - GSR_MENU_ALARM1);
                  mh = (b / 10);
                  ml = b - (mh * 10);
                  mh = roller (mh - 1, 0, 5);
                  Alarms.setMinutes (Menu.Item - GSR_MENU_ALARM1,
                                     (mh * 10) + ml);
                  d = Alarms.getActive (Menu.Item - GSR_MENU_ALARM1);
                  d &= GSRALARM_NOTRIGGER;
                  Alarms.setActive (Menu.Item - GSR_MENU_ALARM1, d);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 3)
                { //  x0 Minutes
                  b = Alarms.getMinutes (Menu.Item - GSR_MENU_ALARM1);
                  mh = (b / 10);
                  ml = b - (mh * 10);
                  ml = roller (ml - 1, 0, 9);
                  Alarms.setMinutes (Menu.Item - GSR_MENU_ALARM1,
                                     (mh * 10) + ml);
                  d = Alarms.getActive (Menu.Item - GSR_MENU_ALARM1);
                  d &= GSRALARM_NOTRIGGER;
                  Alarms.setActive (Menu.Item - GSR_MENU_ALARM1, d);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem == 4)
                { //  Repeats.
                  Alarms.setRepeats (
                      Menu.Item - GSR_MENU_ALARM1,
                      roller (Alarms.getRepeats (Menu.Item - GSR_MENU_ALARM1)
                                  + 1,
                              0, 4));
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem > 4)
                {
                  Menu.SubItem = roller (Menu.SubItem - 1, 5, 13);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_TIMEDN && Menu.SubItem > 0)
            {
              switch (Menu.SubItem)
                {
                case 1: // Hour
                  Alarms.setTimerDownMaxHours (
                      roller (Alarms.getTimerDownMaxHours () - 1, 0, 23));
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                  break;
                case 2: //  x0 Minutes
                  b = Alarms.getTimerDownMaxMinutes ();
                  mh = (b / 10);
                  ml = b - (mh * 10);
                  mh = roller (mh - 1, 0, 5);
                  Alarms.setTimerDownMaxMinutes ((mh * 10) + ml);
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !CountdownTimerActive (); // Quick Update.
                  SetTurbo ();
                  break;
                case 3: //  x0 Minutes
                  b = Alarms.getTimerDownMaxMinutes ();
                  mh = (b / 10);
                  ml = b - (mh * 10);
                  ml = roller (ml - 1, 0, 9);
                  Alarms.setTimerDownMaxMinutes ((mh * 10) + ml);
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !CountdownTimerActive (); // Quick Update.
                  SetTurbo ();
                  break;
                case 4: //  x0 Seconds
                  b = Alarms.getTimerDownMaxSeconds ();
                  mh = (b / 10);
                  ml = b - (mh * 10);
                  mh = roller (mh - 1, 0, 5);
                  Alarms.setTimerDownMaxSeconds ((mh * 10) + ml);
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !CountdownTimerActive (); // Quick Update.
                  SetTurbo ();
                  break;
                case 5: //  x0 Secondss
                  b = Alarms.getTimerDownMaxSeconds ();
                  mh = (b / 10);
                  ml = b - (mh * 10);
                  ml = roller (ml - 1, 0, 9);
                  Alarms.setTimerDownMaxSeconds ((mh * 10) + ml);
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !CountdownTimerActive (); // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_TMEDCF && Menu.SubItem > 0)
            {
              switch (Menu.SubItem)
                {
                case 1: // Repeatable
                  Alarms.setTimerDownRepeats (!Alarms.getTimerDownRepeats ());
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                  break;
                case 2: // Tone Repeats
                  Alarms.setTimerDownMaxTones (
                      roller (Alarms.getTimerDownMaxTones () + 1, 0, 4));
                  Alarms.StopCD ();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_GAME && Menu.SubItem > 0)
            {
              if (ChangeGame (false))
                {
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_DARK && Menu.SubItem > 0)
            { // Sleep Mode.
              switch (Menu.SubItem)
                {
                case 1: // Style.
#ifdef STABLEBMA_H_INCLUDED
                  Options.SleepStyle = roller (Options.SleepStyle - 1, 0, 4);
#else
                  if (Options.SleepStyle != 2)
                    Options.SleepStyle = 2;
                  else
                    Options.SleepStyle = 0;
#endif
                  Updates.Tapped = true;
                  Options.NeedsSaving = true;
                  break;
                case 2: // SleepMode (0=off, 10 seconds)
                  Options.SleepMode = roller (Options.SleepMode - 1, 1, 10);
                  Options.NeedsSaving = true;
                  break;
                case 3: // SleepStart (hour)
                  if (Options.SleepStart - 1 != Options.SleepEnd)
                    {
                      Options.SleepStart
                          = roller (Options.SleepStart - 1, 0, 23);
                      Options.NeedsSaving = true;
                    }
                  else
                    return;
                  break;
                case 4: // SleepStart (hour)
                  if (Options.SleepEnd - 1 != Options.SleepStart)
                    {
                      Options.SleepEnd = roller (Options.SleepEnd - 1, 0, 23);
                      Options.NeedsSaving = true;
                    }
                  else
                    return;
                  break;
                case 5: // Orientation.
                  Options.BedTimeOrientation = !Options.BedTimeOrientation;
                  Options.NeedsSaving = true;
                }
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
          else if (Menu.Item == GSR_MENU_SYNC && Menu.SubItem > 0)
            {
              if (Menu.SubItem > 5)
                {
                  /* On HH:MM
                     6  78 9A */
                  switch (Menu.SubItem)
                    {
                    case 6: // On/Off
                      NTPData.AutoSync = !NTPData.AutoSync;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                      break;
                    case 7: // Hour
                      NTPData.SyncHour = roller (NTPData.SyncHour - 1, 0, 23);
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                      break;
                    case 8: //  x0 Minutes
                      mh = (NTPData.SyncMins / 10);
                      ml = NTPData.SyncMins - (mh * 10);
                      mh = roller (mh - 1, 0, 5);
                      NTPData.SyncMins = (mh * 10) + ml;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                      break;
                    case 9: //  x0 Minutes
                      mh = (NTPData.SyncMins / 10);
                      ml = NTPData.SyncMins - (mh * 10);
                      ml = roller (ml - 1, 0, 9);
                      NTPData.SyncMins = (mh * 10) + ml;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                      break;
                    case 10: // Sunday.
                    case 11: // Monday.
                    case 12: // Tuesday.
                    case 13: // Wednesday.
                    case 14: // Thursday.
                    case 15: // Friday.
                    case 16: // Saturday.
                      I = NTPData.SyncDays ^ Bits[Menu.SubItem - 10];
                      if (I)
                        {
                          NTPData.SyncDays = I;
                          Options.NeedsSaving = true;
                          DoHaptic = true;
                          UpdateDisp = true; // Quick Update.
                          SetTurbo ();
                        }
                    }
                }
              else
                {
                  Menu.SubItem = roller (Menu.SubItem + 1, 1, 5);
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_WEAT && Menu.SubItem > 0)
            { // Weather
              if (Menu.SubItem == 3)
                SetWeatherScale (!IsMetric ());
              else if (Menu.SubItem < 3)
                {
                  I = (Menu.SubItem == 1 ? 12 : 1);
                  WeatherData.Interval
                      = roller (WeatherData.Interval - I, 0, 144);
                  WeatherData.LastCall
                      = WatchTime.UTC_RAW - (WatchTime.UTC_RAW % 60)
                        + (WeatherData.Interval * 300); // 15 minutes.
                }
              if (Menu.SubItem < 4)
                {
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_WIFI && Menu.SubItem > 0)
            {
              // Do nothing!
            }
          else if (Menu.Item == GSR_MENU_TOFF && Menu.SubItem > 0)
            {
              /*
               * S Auto H:MM:SS Begin/Calculate/Current/Toggle RTC/Adjust Drift
               * 0 1    2 34 56 7     8         9       10            12
               */
              /* DO SETUP FOR Edit RTC DOWN */
              if (Menu.SubItem == 1)
                {
                  Options.NTPAuto = !Options.NTPAuto;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              else if (Menu.SubItem < 7)
                {
                  NTPData.TravelTemp -= ((Menu.SubItem == 2 ? 3600 : 0)
                                         + (Menu.SubItem == 3 ? 600 : 0)
                                         + (Menu.SubItem == 4 ? 60 : 0)
                                         + (Menu.SubItem == 5 ? 10 : 0)
                                         + (Menu.SubItem == 6 ? 1 : 0));
                  DoHaptic = true;
                  /* TEST  UpdateDisp = true;  // Quick Update.*/
                  SetTurbo ();
                }
              else if (Menu.SubItem == 12)
                { // Drift Value edit..
                  Diff = SRTC.getDrift (WatchTime.ESPRTC);
                  S = String (Diff);
                  Diff -= pow (10, (S.length () - (1 + Menu.SubSubItem)));
                  if (Diff > 0)
                    {
                      SRTC.setDrift (Diff, SRTC.isFastDrift (WatchTime.ESPRTC),
                                     WatchTime.ESPRTC);
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true; // Quick Update.
                      SetTurbo ();
                    }
                }
              else
                {
                  Menu.SubItem = roller (Menu.SubItem - 1, 7, 10);
                  if (Menu.SubItem == 9
                      && SRTC.getDrift (WatchTime.ESPRTC) == 0)
                    Menu.SubItem--;
                  if (Menu.SubItem == 8
                      && !SRTC.checkingDrift (WatchTime.ESPRTC))
                    Menu.SubItem--;
                  if (WatchTime.ESPRTC)
                    Menu.SubItem == 10;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_BERR && Menu.SubItem == 1)
            { // Battery Error
              if (Battery.LowError > 0.0)
                {
                  Battery.LowError -= 0.01;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
            }
          else if (Menu.Item == GSR_MENU_UNVS && Menu.SubItem == 1)
            { // USE NVS
              if (Menu.SubSubItem == 0)
                {
                  Menu.SubSubItem = 1;
                  DoHaptic = true;
                  UpdateDisp = true; // Quick Update.
                  SetTurbo ();
                }
              return;
            }
          else
            {
              if (Menu.Style == GSR_MENU_INOPTIONS)
                {
                  Menu.Item
                      = roller (Menu.Item + 1, GSR_MENU_STYL,
                                (Watchy_Chip_Info.HasWiFi
                                 & (NTPData.State > 0 || WatchyAPOn || OTAUpdate
                                    || Battery.Read < getLowBatteryRadio ()))
                                    ? GSR_MENU_TRBL
                                    : GSR_MENU_OTAM);
                  if (Menu.Item == GSR_MENU_WEAT && !GetWantWeather ())
                    Menu.Item = roller (
                        Menu.Item + 1, GSR_MENU_STYL,
                        (Watchy_Chip_Info.HasWiFi
                         & (NTPData.State > 0 || WatchyAPOn || OTAUpdate
                            || Battery.Read < getLowBatteryRadio ()))
                            ? GSR_MENU_TRBL
                            : GSR_MENU_OTAM);
                }
              else if (Menu.Style == GSR_MENU_INALARMS)
                {
                  Menu.Item
                      = roller (Menu.Item + 1, GSR_MENU_ALARM1, GSR_MENU_TONES);
                }
              else if (Menu.Style == GSR_MENU_INTIMERS)
                {
                  Menu.Item = roller (Menu.Item + 1, GSR_MENU_TIMEDN,
                                      GSR_MENU_TIMEUP);
                }
              else if (Menu.Style == GSR_MENU_INTROUBLE)
                {
                  Menu.Item
                      = roller (Menu.Item + 1, GSR_MENU_SCRN, GSR_MENU_UNVS);
                }
              else
                {
#ifndef STABLEBMA_H_INCLUDED
                  Menu.Item = roller (Menu.Item + 1, GSR_MENU_ALARMS,
                                      GSR_MENU_OPTIONS);
                  if (Menu.Item == GSR_MENU_GAME && Options.Game == 255)
                    Menu.Item = roller (Menu.Item + 1, GSR_MENU_ALARMS,
                                        GSR_MENU_OPTIONS);
#else
                  Menu.Item = roller (Menu.Item + 1, GSR_MENU_STEPS,
                                      GSR_MENU_OPTIONS);
                  if (Menu.Item == GSR_MENU_GAME && Options.Game == 255)
                    Menu.Item = roller (Menu.Item + 1, GSR_MENU_STEPS,
                                        GSR_MENU_OPTIONS);
#endif
                }
              Menu.SubItem = 0;
              Menu.SubSubItem = 0;
              DoHaptic = true;
              UpdateDisp = true; // Quick Update.
              SetTurbo ();
            }
        }
      else
        Missed = 4; // Missed a SW4.
      break;
    default:
      if (Pressed < 9)
        Missed = Pressed; // Pass off other buttons not handled here.
    }
}
void
WatchyGSR::deepSleep ()
{
  uint8_t N, D, H; // 3.0
  uint64_t cI;
  bool BatOk, BT, B, DM, CD, M;
  struct tm *CT;
  time_t t;
  DM = false;
  CD = CountdownTimerActive ();
  UpdateUTC ();
  UpdateClock ();
  M = (GuiMode != GSR_MENUON);
  if (CD && (Alarms.getTimerDownStopAt () - WatchTime.UTC_RAW) > 60)
    CD = false;

  B = false;
  Alarms.endSounds ();
  UpdateBMA ();
  DM = (Darkness.Went && !CD && M);

  if (DM)
    {
      H = WatchTime.Local.Hour;
      BatOk = (Battery.Read == 0 || Battery.Read > getLowBattery ());
      BT = (Options.SleepStyle == 2 && WatchTime.BedTime);
      B = ((Options.SleepStyle == 1
            || (Options.SleepStyle > 2 && Options.SleepStyle != 4))
           || BT);
      if (Battery.Direction == 1 || Battery.State == 3)
        N = (WatchTime.UTC.Minute - (WatchTime.UTC.Minute % 5) + 5) % 60;
      else
        N = (WatchTime.UTC.Minute < 30 ? 30 : 60);
      if (WatchTime.NextAlarm != 99)
        {
          if (Alarms.getMinutes (WatchTime.NextAlarm) > WatchTime.Local.Minute
              && Alarms.getMinutes (WatchTime.NextAlarm) < N)
            N = Alarms.getMinutes (WatchTime.NextAlarm);
        }
      if (CountdownTimerActive ())
        {
          t = Alarms.getTimerDownStopAt ();
          CT = localtime (&t);
          if (CT->tm_hour == H && CT->tm_min < N
              && CT->tm_min > WatchTime.Local.Minute)
            N = CT->tm_min;
        }
      if (WeatherData.LastCall != 0 && GetWantWeather () && !WatchTime.BedTime)
        {
          CT = localtime (&WeatherData.LastCall);
          if (CT->tm_hour == H && CT->tm_min < N
              && CT->tm_min > WatchTime.Local.Minute)
            N = CT->tm_min;
        }
      if (Steps.Hour == H && Steps.Minutes < N
          && Steps.Minutes > WatchTime.Local.Minute)
        N = Steps.Minutes;
      if (syncToday () && NTPData.SyncHour == H && NTPData.SyncMins < N
          && NTPData.SyncMins > WatchTime.Local.Minute)
        N = NTPData.SyncMins;
    }
  if (Options.NeedsSaving)
    RecordSettings ();
  GoDark (M);
  DisplaySleep ();
  RefreshCPU (GSR_CPULOW);
  detectBattery ();
  if (GSR_PIN_STAT != 255)
    {
      B = Battery.Charge;
      Battery.Charge = (analogRead (GSR_PIN_STAT) * 0.80586 > 3298)
                       && digitalRead (GSR_PIN_USB_DET);
      Battery.Read = rawBatteryVoltage () / 100;
      Battery.Level = (Battery.Charge ? 2 : -2);
      if (B != Battery.Charge)
        {
          Updates.Drawn = true;
          UpdateDisp |= Showing ();
        }
      B = false;
    }

  // log_i("WatchyGSR:DS:N = %u, nextAlarm[%u] = %u",N,WatchTime.NextAlarm,
  // Alarms.getMinutes(WatchTime.NextAlarm));

  if (DM)
    SRTC.atMinuteWake (N);
  else
    SRTC.nextMinuteWake ();
  ForceInputs ();
#ifdef STABLEBMA_H_INCLUDED
  esp_sleep_enable_ext1_wakeup (
      (B ? SBMA.WakeMask () : 0) | GSR_BTN_MASK,
      (esp_sleep_ext1_wakeup_mode_t)
          GSR_ESP_WAKEUP); // enable deep sleep wake on button press  ...
                           // |ACC_INT_MASK
#else
  esp_sleep_enable_ext1_wakeup (
      GSR_BTN_MASK, (esp_sleep_ext1_wakeup_mode_t)
                        GSR_ESP_WAKEUP); // enable deep sleep wake on button
                                         // press  ... |ACC_INT_MASK
#endif
  esp_deep_sleep_start ();
}
String
WatchyGSR::GetLangWebID ()
{
  return LGSR.GetWebLang (Options.LanguageID);
}
uint8_t
WatchyGSR::getLangID ()
{
  return Options.LanguageID;
}
void
WatchyGSR::CheckButtons ()
{
  uint8_t B = WatchyGSR::getButtonPins ();
  if (Options.SleepStyle == 4 && !Updates.Tapped)
    return; // Screen didn't permit buttons to work.
  if (!UpdateDisp && B
      && (LastButton == 0 || (millis () - LastButton) > Debounce ())
      && Missed == 0 && Button == 0)
    Button = B;
}
// Alarms Additions for loop communications.
void
WatchyGSR::alarmResponse ()
{
  switch (Alarms.getCallbackOpcode ())
    {
    case ALARMSGSR_REQ_UTCUPDATE:
      WatchyGSR::requestUTCUpdate ();
      break;
    case ALARMSGSR_REQ_SECONDS:
      Alarms.setCallbackByteData (WatchTime.UTC.Second);
      break;
    case ALARMSGSR_REQ_SHOWING:
      Alarms.setCallbackBoolData (WatchyGSR::requestShowing ());
      break;
    case ALARMSGSR_REQ_DISPUPDATE:
      WatchyGSR::requestRefreshScreen ();
      break;
    case ALARMSGSR_REQ_OTA:
      Alarms.setCallbackBoolData (WatchyGSR::requestOTAStatus ());
      break;
    case ALARMSGSR_SET_DARKNESS:
      WatchyGSR::setDarknessLast (Alarms.getCallbackLongData ());
      break;
    case ALARMSGSR_REQ_DISPWAKE:
      WatchyGSR::requestDisplayWake ();
      break;
    case ALARMSGSR_REQ_MENUITEM:
      Alarms.setCallbackByteData (WatchyGSR::requestMenuItem ());
    }
}
void
WatchyGSR::setDarknessLast (uint32_t newDark)
{
  Darkness.Last = newDark;
}
uint8_t
WatchyGSR::requestMenuItem ()
{
  return Menu.Item;
}
void
WatchyGSR::showWatchFace ()
{
  int I;
  bool B = (Battery.Read > getLowBattery (true));
  bool p = (Options.Feedback && DoHaptic && B && AllowHaptic);
  if (Options.Performance && B)
    if (Options.Performance == 1)
      RefreshCPU (GSR_CPUMID);
    else if (Options.Performance == 2)
      RefreshCPU (GSR_CPULOW);
  DisplayInit ();
  display.setFullWindow ();
  if (GuiMode == GSR_GAMEON)
    drawGame ();
  else
    drawWatchFace ();
  // if (WatchStyles.AddOn[Options.WatchFaceStyle] != nullptr){
  // if (WatchStyles.Options[Options.WatchFaceStyle] & GSR_APP){
  drawOverlay (Options.WatchFaceStyle);
  //}
  //}
  WatchyGSR::drawLogOutput ();
  if (p)
    Alarms.doHaptic ();
  Alarms.updateSound ();
  display.display (!Updates.Full); // partial refresh
  if (!(InTurbo () || Alarms.soundsFiring () || !DarkWait ()))
    DisplaySleep ();
  DoHaptic = false;
  Updates.Full = false;
  Updates.Drawn = true;
  UpdateDisp = false;
  Darkness.Went = false;
  Darkness.Last = millis ();
  setDebounce (p);
}
void
WatchyGSR::drawWatchFace ()
{
  display.fillScreen (BackColor ());
  display.setTextWrap (false);
  if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr)
    {
      if (!OverrideBitmap ())
        {
          if (Design.Face.Bitmap)
            display.drawBitmap (0, 0, Design.Face.Bitmap, 200, 200,
                                ForeColor (), BackColor ());
        }
    }
  else
    {
      if (!WatchStyles.AddOn[Options.WatchFaceStyle]->OverrideBitmap ())
        {
          if (Design.Face.Bitmap)
            display.drawBitmap (0, 0, Design.Face.Bitmap, 200, 200,
                                ForeColor (), BackColor ());
        }
    }
  display.setTextColor (ForeColor ());

  drawWatchFaceStyle ();

  if (!GetNoStatus ())
    {
      drawChargeMe ();
      // Show WiFi/AP/TZ/NTP if in progress.
      drawStatus ();
    }
  if (GuiMode == GSR_MENUON)
    drawMenu ();
}
void
WatchyGSR::drawGame ()
{
  if (Options.Game != 255)
    {
      display.fillScreen (BackColor ());
      display.setTextWrap (false);
      display.setTextColor (ForeColor ());
      if (WatchStyles.AddOn[Options.Game] != nullptr)
        WatchStyles.AddOn[Options.Game]->InsertDrawWatchStyle (Options.Game);
    }
}
void
WatchyGSR::drawOverlay (uint8_t styleID)
{
  if (WatchStyles.AddOn[styleID] != nullptr)
    WatchStyles.AddOn[styleID]->InsertDrawOverlay (styleID);
  else
    InsertDrawOverlay (styleID);
}
uint16_t
WatchyGSR::Debounce ()
{
  return lastDispDraw;
}
void
WatchyGSR::setDebounce (bool pulse)
{
  lastDispDraw = millis () - LastButton + (pulse ? 125 : 165);
}
void
WatchyGSR::SaveProgress ()
{
}
void
WatchyGSR::RegisterWatchFaces ()
{
}
uint8_t
WatchyGSR::AddWatchStyle (String StyleName)
{
  return WatchyGSR::AddWatchStyle (StyleName, nullptr, false);
}
wl_status_t
WatchyGSR::currentWiFi ()
{
  if (!GetAskWiFi ())
    return WL_CONNECT_FAILED;
  if (HasIPAddress ())
    return WL_CONNECTED;
  if (GSRWiFi.Working)
    return WL_IDLE_STATUS; // Make like it is relaxing doing nothing.
  return WL_CONNECT_FAILED;
}
wl_status_t
WatchyGSR::WiFiStatus ()
{
  if (GetAskWiFi ())
    return WiFi.status ();
  else
    return WL_DISCONNECTED;
}
void
WatchyGSR::StartNTP (bool TimeSync, bool TimeZone)
{
  bool b = true;
  if (SRTC.checkingDrift (WatchTime.ESPRTC))
    {
      b = (Options.NTPAuto && NTPData.driftSync
           && WatchTime.UTC_RAW > NTPData.driftSyncNOW);
    }
  if (!WatchyAPOn && NTPData.State == 0 && b && WeatherData.State < 2)
    {
      NTPData.TimeZone = (OP.getCurrentPOSIX () == OP.TZMISSING)
                         || TimeZone; // No Timezone, go get one!
      NTPData.UpdateUTC = TimeSync;
      if (NTPData.TimeZone || NTPData.UpdateUTC)
        {
          NTPData.State = 1;
          NTPData.Pause = 2;
        }
    }
}
void
WatchyGSR::StartWeather ()
{
  if (!WatchyAPOn && WeatherData.State == 0 && NTPData.State < 2
      && GSRHandle == NULL && GetWantWeather ())
    {
      WeatherData.State = 1;
      WeatherData.Pause = 2;
    }
}
// Watch Face designs are here.
void
WatchyGSR::initWatchFaceStyle ()
{
  Design.Menu.Top = 72;
  Design.Menu.Header = 25;
  Design.Menu.Data = 66;
  Design.Menu.Gutter = 3;
  Design.Menu.Font = &aAntiCorona12pt7b;
  Design.Menu.FontSmall = &aAntiCorona11pt7b;
  Design.Menu.FontSmaller = &aAntiCorona10pt7b;
  Design.Face.Bitmap = nullptr;
  Design.Face.SleepBitmap = nullptr;
  Design.Face.Gutter = 4;
  Design.Face.Time = 56;
  Design.Face.TimeHeight = 45;
  Design.Face.TimeColor = GSR_AutoFore;
  Design.Face.TimeFont = &aAntiCorona36pt7b;
  Design.Face.TimeLeft = 0;
  Design.Face.TimeStyle = WatchyGSR::dCENTER;
  Design.Face.Day = 101;
  Design.Face.DayGutter = 4;
  Design.Face.DayColor = GSR_AutoFore;
  Design.Face.DayFont = &aAntiCorona16pt7b;
  Design.Face.DayFontSmall = &aAntiCorona15pt7b;
  Design.Face.DayFontSmaller = &aAntiCorona14pt7b;
  Design.Face.DayLeft = 0;
  Design.Face.DayStyle = WatchyGSR::dCENTER;
  Design.Face.Date = 143;
  Design.Face.DateGutter = 4;
  Design.Face.DateColor = GSR_AutoFore;
  Design.Face.DateFont = &aAntiCorona15pt7b;
  Design.Face.DateFontSmall = &aAntiCorona14pt7b;
  Design.Face.DateFontSmaller = &aAntiCorona13pt7b;
  Design.Face.DateLeft = 0;
  Design.Face.DateStyle = WatchyGSR::dCENTER;
  Design.Face.Year = 186;
  Design.Face.YearLeft = 99;
  Design.Face.YearColor = GSR_AutoFore;
  Design.Face.YearFont = &aAntiCorona16pt7b;
  Design.Face.YearLeft = 0;
  Design.Face.YearStyle = WatchyGSR::dCENTER;
  Design.Status.WIFIx = 5;
  Design.Status.WIFIy = 193;
  Design.Status.BATTx = 155;
  Design.Status.BATTy = 178;
  Design.Status.Inverted = false;
  Design.Status.BatteryInverted = false;
  if (DefaultWatchStyles)
    {
      if (Options.WatchFaceStyle > BasicWatchStyles
          && Options.WatchFaceStyle != 255)
        {
          if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr)
            InsertInitWatchStyle (Options.WatchFaceStyle);
          else
            WatchStyles.AddOn[Options.WatchFaceStyle]->InsertInitWatchStyle (
                Options.WatchFaceStyle);
          return;
        }
    }
  else if (WatchStyles.Count > 0 && BasicWatchStyles == -1)
    {
      if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr)
        InsertInitWatchStyle (Options.WatchFaceStyle);
      else
        WatchStyles.AddOn[Options.WatchFaceStyle]->InsertInitWatchStyle (
            Options.WatchFaceStyle);
      return;
    }
  InsertInitWatchStyle (
      Options.WatchFaceStyle); // Allow for Overrides to do this.
}
void
WatchyGSR::drawWatchFaceStyle ()
{
  uint8_t Style = Options.WatchFaceStyle;
  if (DefaultWatchStyles)
    {
      if (Style > BasicWatchStyles && Style != 255)
        {
          if (WatchStyles.AddOn[Style] == nullptr)
            InsertDrawWatchStyle (Style);
          else
            WatchStyles.AddOn[Style]->InsertDrawWatchStyle (Style);
          return;
        }
    }
  else if (WatchStyles.Count > 0 && BasicWatchStyles == -1)
    {
      if (WatchStyles.AddOn[Style] == nullptr)
        InsertDrawWatchStyle (Style);
      else
        WatchStyles.AddOn[Style]->InsertDrawWatchStyle (Style);
      return;
    }
  else
    Style = 0;
  switch (Style)
    {
    default:
      if (SafeToDraw ())
        {
          drawTime ();
          drawDay ();
          drawYear ();
          drawWeather ();
        }
      if (NoMenu ())
        drawDate ();
      break;
    }
}
void
WatchyGSR::initAddOn (WatchyGSR *NewAddOn)
{
  uint8_t I = 0;
  for (I = 0; I < BootAddOns.Count; I++)
    {
      if (NewAddOn == BootAddOns.Inits[I])
        return; // Found it, exit.
    }
  if (BootAddOns.Count < GSR_MaxStyles)
    {
      BootAddOns.Inits[BootAddOns.Count] = NewAddOn;
      BootAddOns.Count++;
    }
}

// Private
void
WatchyGSR::setStatus (String Status)
{
  if (Status == "" && !SRTC.onESP32 () && !SRTC.isOperating ())
    Status = "NC";
  if (GSRWiFi.Requests > 0)
    {
      if (WatchyStatus.startsWith (WiFiTXT) && Status != "")
        WatchyOStatus = WatchyStatus;
      else if (WatchyOStatus.startsWith (WiFiTXT) && Status == "")
        Status = WatchyOStatus;
    }
  else if (WatchyOStatus != "")
    WatchyOStatus = "";
  if (WatchyStatus != Status)
    {
      WatchyStatus = Status;
      UpdateDisp |= Showing ();
    }
}
void
WatchyGSR::drawMenu ()
{
  int16_t x1, y1, z1;
  uint16_t D, L, B, C, P;
  uint16_t w, h;
  String O, S, T, V;
  unsigned long U;
  tmElements_t TM;

  display.drawBitmap (0, Design.Menu.Top,
                      (Menu.Style == GSR_MENU_INOPTIONS) ? OptionsMenuBackground
                                                         : MenuBackground,
                      GSR_MenuWidth, GSR_MenuHeight, ForeColor (),
                      BackColor ());
  display.setTextColor (Options.LightMode && Menu.Style != GSR_MENU_INNORMAL
                            ? GxEPD_WHITE
                            : GxEPD_BLACK);
  switch (Menu.Item)
    {
    case GSR_MENU_STEPS:
      if (Menu.SubItem > 0 && Menu.SubItem < 4)
        O = LGSR.GetID (Options.LanguageID, 2);
      else if (Menu.SubItem == 4)
        O = LGSR.GetID (Options.LanguageID, 3);
      else
        O = LGSR.GetID (Options.LanguageID, 4);
      break;
    case GSR_MENU_ALARMS:
      O = LGSR.GetID (Options.LanguageID, 5);
      break;
    case GSR_MENU_TIMERS:
      O = LGSR.GetID (Options.LanguageID, 6);
      break;
    case GSR_MENU_GAME:
      O = LGSR.GetID (Options.LanguageID, 127);
      break;
    case GSR_MENU_OPTIONS:
      O = LGSR.GetID (Options.LanguageID, 7);
      break;
    case GSR_MENU_ALARM1:
    case GSR_MENU_ALARM2:
    case GSR_MENU_ALARM3:
    case GSR_MENU_ALARM4:
    case GSR_MENU_ALARM5:
    case GSR_MENU_ALARM6:
    case GSR_MENU_ALARM7:
    case GSR_MENU_ALARM8:
      if (Menu.SubItem > 0 && Menu.SubItem < 4)
        O = LGSR.GetID (Options.LanguageID, 8);
      else if (Menu.SubItem == 4)
        O = LGSR.GetID (Options.LanguageID, 9);
      else if (Menu.SubItem > 4)
        O = LGSR.GetID (Options.LanguageID, 10);
      else
        O = LGSR.GetID (Options.LanguageID, 11);
      O.replace ("#", String (char (49 + Menu.Item - GSR_MENU_ALARM1)));
      break;
    case GSR_MENU_TONES:
      O = LGSR.GetID (Options.LanguageID, 24);
      break;
    case GSR_MENU_TIMEDN:
      O = LGSR.GetID (Options.LanguageID, 25);
      break;
    case GSR_MENU_TMEDCF:
      O = LGSR.GetID (Options.LanguageID, 119);
      break;
    case GSR_MENU_TIMEUP:
      O = LGSR.GetID (Options.LanguageID, 26);
      break;
    case GSR_MENU_STYL:
      O = LGSR.GetID (Options.LanguageID, 27);
      break;
    case GSR_MENU_DISP:
      O = LGSR.GetID (Options.LanguageID, 28);
      break;
    case GSR_MENU_LANG:
      O = LGSR.GetID (Options.LanguageID, 29);
      break;
    case GSR_MENU_BRDR:
      O = LGSR.GetID (Options.LanguageID, 30);
      break;
    case GSR_MENU_SIDE:
      O = LGSR.GetID (Options.LanguageID, 31);
      break;
    case GSR_MENU_SWAP:
      O = LGSR.GetID (Options.LanguageID, 32);
      break;
    case GSR_MENU_ORNT:
      O = LGSR.GetID (Options.LanguageID, 33);
      break;
    case GSR_MENU_MODE:
      O = LGSR.GetID (Options.LanguageID, 34);
      break;
    case GSR_MENU_FEED:
      O = LGSR.GetID (Options.LanguageID, 35);
      break;
    case GSR_MENU_TRBO:
      O = LGSR.GetID (Options.LanguageID, 36);
      break;
    case GSR_MENU_DARK:
      switch (Menu.SubItem)
        {
        case 0:
          O = LGSR.GetID (Options.LanguageID, 37);
          break;
        case 1:
          O = LGSR.GetID (Options.LanguageID, 38);
          break;
        case 2:
          O = LGSR.GetID (Options.LanguageID, 39);
          break;
        case 3:
          O = LGSR.GetID (Options.LanguageID, 40);
          break;
        case 4:
          O = LGSR.GetID (Options.LanguageID, 41);
          break;
        case 5:
          O = LGSR.GetID (Options.LanguageID, 42);
        }
      break;
    case GSR_MENU_SAVE:
      O = LGSR.GetID (Options.LanguageID, 43);
      break;
    case GSR_MENU_TPWR:
      O = LGSR.GetID (Options.LanguageID, 44);
      break;
    case GSR_MENU_INFO:
      O = LGSR.GetID (Options.LanguageID, 45);
      break;
    case GSR_MENU_TRBL:
      O = LGSR.GetID (Options.LanguageID, 46);
      break;
    case GSR_MENU_SYNC:
      O = LGSR.GetID (Options.LanguageID, (Menu.SubItem != 4 ? 47 : 107));
      break;
    case GSR_MENU_WEAT: // Weather Interval
      if (Menu.SubItem == 0)
        O = LGSR.GetID (Options.LanguageID, 122);
      else if (Menu.SubItem < 3)
        O = LGSR.GetID (Options.LanguageID, 123);
      else if (Menu.SubItem == 3)
        O = LGSR.GetID (Options.LanguageID, 124);
      else
        O = LGSR.GetID (Options.LanguageID, 128);
      break;
    case GSR_MENU_WIFI:
      O = LGSR.GetID (Options.LanguageID, 48);
      break;
    case GSR_MENU_OTAU:
      if (Menu.SubItem == 2 || Menu.SubItem == 3)
        O = LGSR.GetID (Options.LanguageID, 49);
      else
        O = LGSR.GetID (Options.LanguageID, 50);
      break;
    case GSR_MENU_OTAM:
      if (Menu.SubItem == 2 || Menu.SubItem == 3)
        O = LGSR.GetID (Options.LanguageID, 51);
      else
        O = LGSR.GetID (Options.LanguageID, 52);
      break;
    case GSR_MENU_SCRN:
      O = LGSR.GetID (Options.LanguageID, 53);
      break;
    case GSR_MENU_RSET:
      O = LGSR.GetID (Options.LanguageID, 54);
      break;
    case GSR_MENU_TOFF:
      /*
       * S Auto H:MM:SS Begin/Calculate/Current/Toggle RTC/Adjust Drift
       * 0 1    2 34 56 7     8         9       10            12
       */
      if (Menu.SubItem == 0 || (Menu.SubItem < 7 && Menu.SubItem > 1))
        O = LGSR.GetID (Options.LanguageID, 56);
      else if (Menu.SubItem == 1)
        O = LGSR.GetID (Options.LanguageID, 12);
      else if (Menu.SubItem > 6 && Menu.SubItem < 10)
        O = LGSR.GetID (Options.LanguageID, 121);
      else if (Menu.SubItem == 10)
        O = LGSR.GetID (Options.LanguageID, 106);
      else if (Menu.SubItem == 12)
        O = LGSR.GetID (Options.LanguageID, 129);
      break;
    case GSR_MENU_BERR:
      O = LGSR.GetID (Options.LanguageID, 131);
      break;
    case GSR_MENU_UNVS:
      switch (Menu.SubItem)
        {
        case 0:
          O = LGSR.GetID (Options.LanguageID, 57);
          break;
        case 1:
          O = LGSR.GetID (Options.LanguageID, 58);
          break;
        case 2:
          O = LGSR.GetID (Options.LanguageID, 59);
          break;
        }
    }
  setFontFor (O, Design.Menu.Font, Design.Menu.FontSmall,
              Design.Menu.FontSmaller, Design.Menu.Gutter);
  display.getTextBounds (O, 0, Design.Menu.Header + Design.Menu.Top, &x1, &y1,
                         &w, &h);
  w = (196 - w) / 2;
  display.setCursor (w + 2, Design.Menu.Header + Design.Menu.Top);
  display.print (O);
  display.setTextColor (GxEPD_BLACK); // Only show menu in Light mode
  Updates.Highlighting.Active = false;
  Updates.Indexing.Active = false;
  if (Menu.Item == GSR_MENU_STEPS)
    { // Steps
      if (Menu.SubItem == 0)
        O = CurrentSteps (true);
      else if (Menu.SubItem < 4)
        {
          T = MakeHour (Steps.Hour);
          S = MakeMinutes (Steps.Minutes);
          O = T + ":" + S + MakeTOD (Steps.Hour, true);
          Updates.Indexing.Active = true;
          Updates.Indexing.Offset
              = Menu.SubItem - 1 + (Menu.SubItem > 1 ? T.length () : 0);
          Updates.Indexing.Width = (Menu.SubItem == 1 ? T.length () : 1);
        }
      else
        O = LGSR.GetID (Options.LanguageID, 60);
    }
  else if (Menu.Item == GSR_MENU_ALARMS)
    {
      O = LGSR.GetID (Options.LanguageID, 61);
    }
  else if (Menu.Item >= GSR_MENU_ALARM1 && Menu.Item <= GSR_MENU_ALARM8)
    { // Alarms
      O = "";
      if (Menu.SubItem == 0)
        O = LGSR.GetID (Options.LanguageID, 62);
      else if (Menu.SubItem < 5)
        {
          S = MakeMinutes (Alarms.getMinutes (Menu.Item - GSR_MENU_ALARM1));
          T = getReduce (Alarms.getRepeats (Menu.Item - GSR_MENU_ALARM1));
          O = MakeHour (Alarms.getHour (Menu.Item - GSR_MENU_ALARM1));
          V = MakeTOD (Alarms.getHour (Menu.Item - GSR_MENU_ALARM1), false)
              + " ";
          Updates.Indexing.Active = true;
          Updates.Indexing.Offset
              = Menu.SubItem - 1 + (Menu.SubItem > 1 ? O.length () : 0)
                + (Menu.SubItem > 3 ? S.length () + V.length () - 2 : 0);
          Updates.Indexing.Width
              = (Menu.SubItem == 4 ? T.length ()
                                   : (Menu.SubItem == 1 ? O.length () : 1));
          O += ":" + S + V + T;
        }
      else if (Menu.SubItem > 4 && Menu.SubItem < 14)
        {
          O = "";
          Updates.Highlighting.Active = true;
          Updates.Highlighting.Bits = 0;
          for (L = 0; L < 7; L++)
            {
              T = LGSR.GetWeekday (Options.LanguageID, L);
              O = O + T.charAt (0);
              Updates.Highlighting.Bits
                  |= ((Alarms.getActive (Menu.Item - GSR_MENU_ALARM1) & Bits[L])
                              == Bits[L]
                          ? Bits[L]
                          : 0);
            }
          T = LGSR.GetID (Options.LanguageID, 63);
          O = O + " " + T.charAt (0);
          Updates.Highlighting.Bits
              |= ((Alarms.getActive (Menu.Item - GSR_MENU_ALARM1)
                   & GSRALARM_REPEAT)
                  == GSRALARM_REPEAT)
                     ? BitValue64 (8)
                     : 0;
          T = LGSR.GetID (Options.LanguageID, 64);
          O = O + " " + T.charAt (0);
          Updates.Highlighting.Bits
              |= ((Alarms.getActive (Menu.Item - GSR_MENU_ALARM1)
                   & GSRALARM_ACTIVE)
                  == GSRALARM_ACTIVE)
                     ? BitValue64 (10)
                     : 0;
          Updates.Indexing.Active = true;
          Updates.Indexing.Offset = Menu.SubItem - 5
                                    + (Menu.SubItem > 11 ? 1 : 0)
                                    + (Menu.SubItem == 13 ? 1 : 0);
          Updates.Indexing.Width = 1;
        }
    }
  else if (Menu.Item == GSR_MENU_TONES)
    { // Repeats on Alarms.
      O = getReduce (Options.MasterRepeats) + " "
          + LGSR.GetID (Options.LanguageID, 65);
    }
  else if (Menu.Item == GSR_MENU_TIMERS)
    { // Timers
      O = LGSR.GetID (Options.LanguageID, 66);
    }
  else if (Menu.Item == GSR_MENU_TIMEDN)
    { // Countdown
      if (CountdownTimerActive ())
        UpdateUTC ();
      S = MakeMinutes (CountdownTimerActive ()
                           ? Alarms.getTimerDownMinutes ()
                           : Alarms.getTimerDownMaxMinutes ());
      T = MakeMinutes (CountdownTimerActive ()
                           ? Alarms.getTimerDownSeconds ()
                           : Alarms.getTimerDownMaxSeconds ());
      if (Menu.SubItem == 0)
        O = LGSR.GetID (Options.LanguageID, 62);
      else
        {
          V = String (CountdownTimerActive () ? Alarms.getTimerDownHours ()
                                              : Alarms.getTimerDownMaxHours ());
          O = V + ":" + S + ":" + T + " " + CountdownTimerState ();
          Updates.Indexing.Active = true;
          Updates.Indexing.Offset
              = Menu.SubItem - 1 + (Menu.SubItem > 1 ? V.length () : 0)
                + (Menu.SubItem > 3 ? 1 : 0) + (Menu.SubItem > 5 ? 1 : 0);
          Updates.Indexing.Width
              = (Menu.SubItem == 1
                     ? V.length ()
                     : (Menu.SubItem > 5 ? CountdownTimerState ().length ()
                                         : 1));
        }
    }
  else if (Menu.Item == GSR_MENU_TMEDCF)
    { // Countdown Settings
      if (Menu.SubItem == 0)
        O = LGSR.GetID (Options.LanguageID, 62);
      else
        {
          V = LGSR.GetID (Options.LanguageID,
                          (Alarms.getTimerDownRepeats () ? 118 : 117));
          T = getReduce (Alarms.getTimerDownMaxTones ());
          O = V + " " + T;
          Updates.Indexing.Offset
              = (Menu.SubItem == 1 ? 0
                                   : (Menu.SubItem == 2 ? V.length () + 1 : 0));
          Updates.Indexing.Width
              = (Menu.SubItem == 1 ? V.length ()
                                   : (Menu.SubItem == 2 ? T.length () : 0));
          Updates.Indexing.Active = true;
        }
    }
  else if (Menu.Item == GSR_MENU_TIMEUP)
    { // Elapsed
      switch (Menu.SubItem)
        {
        case 0:
          O = LGSR.GetID (Options.LanguageID, 62);
          break;
        case 1:
          if (TimerUp.Active)
            {
              UpdateUTC ();
              U = (WatchTime.UTC_RAW - TimerUp.SetAt);
            }
          else
            U = (TimerUp.StopAt - TimerUp.SetAt);
          y1 = U / 3600;
          x1 = (U - (y1 * 3600)) / 60;
          z1 = U % 60;
          V = String (y1) + ":" + MakeMinutes (x1) + ":" + MakeMinutes (z1)
              + " ";
          S = ElapsedTimerState ();
          O = V + S;
          Updates.Indexing.Offset = V.length () + 1;
          Updates.Indexing.Width = S.length ();
          Updates.Indexing.Active = true;
        }
    }
  else if (Menu.Item == GSR_MENU_GAME)
    { // Game Menu
      switch (Menu.SubItem)
        {
        case 0:
          O = LGSR.GetID (Options.LanguageID, 67);
          break;
        case 1:
          O = LGSR.LangString (CurrentGameStyle (), false, Options.LanguageID,
                               0, 1);
        }
    }
  else if (Menu.Item == GSR_MENU_OPTIONS)
    { // Options Menu
      O = LGSR.GetID (Options.LanguageID, 70);
    }
  else if (Menu.Item == GSR_MENU_STYL)
    { // Switch Watch Style
      O = LGSR.LangString (CurrentWatchStyle (), false, Options.LanguageID, 0,
                           1); // Only translate the onboard ones.
    }
  else if (Menu.Item == GSR_MENU_LANG)
    { // Show current Language
      O = LGSR.GetLangName (Options.LanguageID);
    }
  else if (Menu.Item == GSR_MENU_DISP)
    { // Switch Mode
      O = LGSR.GetID (Options.LanguageID, (Options.LightMode ? 71 : 72));
    }
  else if (Menu.Item == GSR_MENU_SIDE)
    { // Dexterity Mode
      O = LGSR.GetID (Options.LanguageID, (Options.Lefty ? 73 : 74));
    }
  else if (Menu.Item == GSR_MENU_SWAP)
    { // Swap Menu/Back Buttons
      O = LGSR.GetID (Options.LanguageID, (Options.Swapped ? 75 : 76));
    }
  else if (Menu.Item == GSR_MENU_BRDR)
    { // Border Mode
      O = LGSR.GetID (Options.LanguageID, (Options.Border) ? 72 : 71);
    }
  else if (Menu.Item == GSR_MENU_ORNT)
    { // Watchy Orientation.
      O = LGSR.GetID (Options.LanguageID, (Options.Orientated ? 77 : 78));
    }
  else if (Menu.Item == GSR_MENU_MODE)
    { // 24hr Format Swap.
      O = LGSR.GetID (Options.LanguageID, (Options.TwentyFour ? 79 : 80));
    }
  else if (Menu.Item == GSR_MENU_FEED)
    { // Feedback
      O = LGSR.GetID (Options.LanguageID, (Options.Feedback ? 81 : 83));
    }
  else if (Menu.Item == GSR_MENU_TRBO)
    { // Turbo!
      if (Options.Turbo)
        O = String (Options.Turbo) + " " + MakeSeconds (Options.Turbo);
      else
        O = LGSR.GetID (Options.LanguageID, 68);
    }
  else if (Menu.Item == GSR_MENU_DARK)
    { // Dark Running.
      if (Menu.SubItem == 0)
        O = LGSR.GetID (Options.LanguageID, 67);
      else if (Menu.SubItem == 2)
        O = String (Options.SleepMode) + " " + MakeSeconds (Options.SleepMode);
      else if (Menu.SubItem == 5)
        O = LGSR.GetID (Options.LanguageID,
                        (Options.BedTimeOrientation ? 77 : 78));
      else if (Menu.SubItem == 1)
        {
          switch (Options.SleepStyle)
            {
            case 0:
              O = LGSR.GetID (Options.LanguageID, 83);
              break;
            case 1:
              O = LGSR.GetID (Options.LanguageID, 84);
              break;
            case 2:
              O = LGSR.GetID (Options.LanguageID, 85);
              break;
            case 3:
              O = LGSR.GetID (Options.LanguageID, 86);
              break;
            case 4:
              O = LGSR.GetID (Options.LanguageID, 87);
            }
        }
      else
        {
          S = MakeHour (Options.SleepStart)
              + MakeTOD (Options.SleepStart, true);
          T = " " + LGSR.GetID (Options.LanguageID, 88) + " ";
          V = MakeHour (Options.SleepEnd) + MakeTOD (Options.SleepEnd, true);
          O = S + T + V;
          Updates.Indexing.Active = true;
          Updates.Indexing.Offset
              = (Menu.SubItem == 4 ? S.length () + T.length () : 0);
          Updates.Indexing.Width
              = (Menu.SubItem == 3 ? S.length () : T.length ());
        }
    }
  else if (Menu.Item == GSR_MENU_TPWR)
    { // WiFi Tx Power
      O = IDWiFiTX[getTXOffset (GSRWiFi.TransmitPower)];
    }
  else if (Menu.Item == GSR_MENU_INFO)
    { // Information
      switch (Menu.SubItem)
        {
        case 0:
          O = LGSR.GetID (Options.LanguageID, 89) + ": " + String (Build);
          break;
        case 1:
          Battery.Read = (rawBatteryVoltage ()) / 100;
          O = LGSR.GetID (Options.LanguageID, 90) + ": "
              + String (Battery.Read, 2) + "V";
          break;
        case 2:
          if (HWVer > 1.0f)
            {
              if (HWVer == 3.0f)
                O = "V3 ESP32S3";
              else
                O = (HWVer == 2.0f) ? "V2.0 PCF8563" : "V1.5 PCF8563";
            }
          else
            O = "V1 DS3231M";
          if (HWVer != 3.0f && SRTC.onESP32 ())
            O += " ESP";
          break;
        }
    }
  else if (Menu.Item == GSR_MENU_SAVE)
    { // Performance
      if (Options.Performance == 2)
        O = LGSR.GetID (Options.LanguageID, 91);
      else
        O = LGSR.GetID (Options.LanguageID,
                        (Options.Performance == 1 ? 76 : 92));
    }
  else if (Menu.Item == GSR_MENU_TRBL)
    { // Troubleshooting.
      O = LGSR.GetID (Options.LanguageID, 70);
    }
  else if (Menu.Item == GSR_MENU_SYNC)
    { // NTP
      if (Menu.SubItem > 5 && Menu.SubItem < 10)
        {
          V = MakeHour (NTPData.SyncHour);
          S = MakeMinutes (NTPData.SyncMins);
          T = LGSR.GetID (Options.LanguageID, (NTPData.AutoSync ? 69 : 68));
          O = T + " " + V + ":" + S + MakeTOD (NTPData.SyncHour, true);
          Updates.Indexing.Offset = Menu.SubItem - 6
                                    + (Menu.SubItem > 6 ? T.length () : 0)
                                    + (Menu.SubItem > 7 ? V.length () : 0);
          Updates.Indexing.Width
              = (Menu.SubItem == 6 ? T.length ()
                                   : (Menu.SubItem == 7 ? V.length () : 1));
          Updates.Indexing.Active = true;
        }
      else if (Menu.SubItem > 9)
        {
          O = "";
          Updates.Highlighting.Active = true;
          Updates.Highlighting.Bits = 0;
          for (L = 0; L < 7; L++)
            {
              T = LGSR.GetWeekday (Options.LanguageID, L);
              O += T.charAt (0);
              Updates.Highlighting.Bits
                  |= ((NTPData.SyncDays & Bits[L]) == Bits[L] ? Bits[L] : 0);
            }
          Updates.Indexing.Active = true;
          Updates.Indexing.Offset = Menu.SubItem - 10;
          Updates.Indexing.Width = 1;
        }
      else
        {
          switch (Menu.SubItem)
            {
            case 0:
              O = LGSR.GetID (Options.LanguageID, 93);
              break;
            case 1:
              O = LGSR.GetID (Options.LanguageID, 94);
              break;
            case 2:
              O = LGSR.GetID (Options.LanguageID, 95);
              break;
            case 3:
              O = LGSR.GetID (Options.LanguageID, 96);
              break;
            case 4: // Sync at Charge.
              O = LGSR.GetID (Options.LanguageID, 133) + " ";
              L = O.length ();
              S = (Options.ntpAtCharge ? LGSR.GetID (Options.LanguageID, 69)
                                       : LGSR.GetID (Options.LanguageID, 68));
              O += S;
              Updates.Indexing.Offset = L;
              Updates.Indexing.Width = S.length ();
              Updates.Indexing.Active = true;
              break;
            case 5:
              O = LGSR.GetID (Options.LanguageID, 105);
            }
        }
    }
  else if (Menu.Item == GSR_MENU_WEAT)
    { // Weather Interval
      /* DO WEATHER TIME HERE */
      if (Menu.SubItem == 0)
        O = LGSR.GetID (Options.LanguageID, 67);
      else if (Menu.SubItem > 0 && Menu.SubItem < 3)
        {
          if (WeatherData.Interval == 0)
            O = LGSR.GetID (Options.LanguageID, 83);
          else
            {
              D = WeatherData.Interval * 300;
              L = D / 3600;
              O = "";
              V = String (L);
              B = V.length ();
              Updates.Indexing.Offset
                  = ((Menu.SubItem == 1 || L == 0) ? 0 : 2 + B);
              Updates.Indexing.Width = ((Menu.SubItem == 1 && L) ? B : 2);
              Updates.Indexing.Active = true;
              if (L)
                O = MakeHour (L) + "H:";
              D -= (L * 3600);
              O += MakeMinutes (D / 60) + "M";
            }
        }
      else if (Menu.SubItem == 3)
        O = LGSR.GetID (Options.LanguageID, (IsMetric () ? 125 : 126));
      else
        O = LGSR.GetID (Options.LanguageID, 97);
    }
  else if (Menu.Item == GSR_MENU_WIFI)
    { // WiFi Connect.
      if (Menu.SubItem == 0)
        {
          O = LGSR.GetID (Options.LanguageID, 97);
        }
      else if (Menu.SubItem == 1)
        {
          O = LGSR.GetID (Options.LanguageID, 98);
        }
      else if (Menu.SubItem == 2)
        {
          O = WiFi_AP_SSID;
        }
      else if (Menu.SubItem == 3)
        {
          O = WiFi.softAPIP ().toString ();
        }
      else if (Menu.SubItem == 4)
        {
          O = LGSR.GetID (Options.LanguageID, 99);
        }
      else if (Menu.SubItem == 5)
        {
          O = LGSR.GetID (Options.LanguageID, 130);
        }
    }
  else if (Menu.Item == GSR_MENU_OTAU || Menu.Item == GSR_MENU_OTAM)
    { // OTA Update.
      if (Menu.SubItem == 0)
        {
          O = LGSR.GetID (Options.LanguageID, 100);
        }
      else if (Menu.SubItem == 1)
        {
          O = LGSR.GetID (Options.LanguageID, 101);
        }
      else if (Menu.SubItem == 2 || Menu.SubItem == 3)
        {
          O = GSRWiFi.LocalIP.toString ();
        }
    }
  else if (Menu.Item == GSR_MENU_SCRN)
    { // Reset Screen.
      O = LGSR.GetID (Options.LanguageID, 60);
    }
  else if (Menu.Item == GSR_MENU_RSET)
    { // Reboot Watchy
      switch (Menu.SubItem)
        {
        case 1:
          O = LGSR.GetID (Options.LanguageID, 102);
          break;
        default:
          O = LGSR.GetID (Options.LanguageID, 103);
        }
    }
  else if (Menu.Item == GSR_MENU_TOFF)
    { // Time Travel detect.
      /*
       * S Auto H:MM:SS Begin/Calculate/Current/Toggle RTC/Adjust Drift
       * 0 1    2 34 56 7     8         9       10            12
       */
      if (Menu.SubItem == 0)
        {
          O = LGSR.GetID (Options.LanguageID, 62);
          NTPData.TravelTemp = 0;
        }
      else if (Menu.SubItem == 1)
        {
          O = LGSR.GetID (Options.LanguageID, (Options.NTPAuto ? 13 : 14));
        }
      else if (Menu.SubItem > 1 && Menu.SubItem < 7)
        {
          UpdateUTC ();
          TM = UTCtoLocal (WatchTime.UTC_RAW + NTPData.TravelTemp);
          S = MakeHour (TM.Hour);
          V = MakeMinutes (TM.Minute);
          T = MakeMinutes (TM.Second);
          O = S + ":" + V + ":" + T + MakeTOD (TM.Hour, true);
          Updates.Indexing.Active = true;
          Updates.Indexing.Offset = Menu.SubItem - 2
                                    + (Menu.SubItem > 2 ? S.length () : 0)
                                    + (Menu.SubItem > 4 ? 1 : 0);
          Updates.Indexing.Width = (Menu.SubItem == 2 ? S.length () : 1);
        }
      else if (Menu.SubItem == 7)
        { // Drift Begin
          O = LGSR.GetID (Options.LanguageID, 55);
        }
      else if (Menu.SubItem == 8)
        { // Drift Calculate
          O = LGSR.GetID (Options.LanguageID, (NTPData.driftSync ? 15 : 120));
        }
      else if (Menu.SubItem == 9)
        { // Drift Current
          O = String (SRTC.getDrift (WatchTime.ESPRTC)) + "/"
              + (SRTC.isFastDrift (WatchTime.ESPRTC) ? "-1" : "1");
        }
      else if (Menu.SubItem == 10)
        { // Toggle ESP32 RTC
          O = LGSR.GetID (
              Options.LanguageID,
              (HWVer == 3.0
                   ? 82
                   : (WatchTime.ESPRTC ? 81 : 83))); /* WatchTime.ESPRTC, really
                                                        says use the ESP32 */
        }
      else if (Menu.SubItem == 12)
        { // Drift Current
          S = String (SRTC.getDrift (WatchTime.ESPRTC));
          Menu.SubSubItem = (Menu.SubSubItem < S.length () ? Menu.SubSubItem
                                                           : S.length () - 1);
          Updates.Indexing.Active = true;
          Updates.Indexing.Offset = Menu.SubSubItem;
          Updates.Indexing.Width = 1;
          O = S + "/" + (SRTC.isFastDrift (WatchTime.ESPRTC) ? "-1" : "1");
        }
    }
  else if (Menu.Item == GSR_MENU_BERR)
    { // Battery Error
      O = (Menu.SubItem == 0
               ? LGSR.GetID (Options.LanguageID, 67)
               : (Battery.LowError > 0.1
                      ? LGSR.GetID (Options.LanguageID, 19)
                      : (Battery.LowError > 0
                             ? "+" + String (Battery.LowError)
                             : LGSR.GetID (Options.LanguageID, 132))));
    }
  else if (Menu.Item == GSR_MENU_UNVS)
    { // USE NVS
      switch (Menu.SubItem)
        {
        case 0:
          O = LGSR.GetID (Options.LanguageID, (OkNVS (GSR_GName) ? 108 : 109));
          break;
        case 1:
          O = LGSR.GetID (Options.LanguageID,
                          (Menu.SubSubItem == 0 ? 110 : 67));
          break;
        case 2:
          O = LGSR.GetID (Options.LanguageID, 111);
          break;
        }
    }

  if (O)
    {
      setFontFor (O, Design.Menu.Font, Design.Menu.FontSmall,
                  Design.Menu.FontSmaller, Design.Menu.Gutter);
      D = Design.Menu.Data + Design.Menu.Top;
      display.getTextBounds (O, 0, D, &x1, &y1, &w, &h);
      /*
       * D = Down (vertical)
       * L = Left (horizontal)
       * B = Begin (horizontal)
       * C = Count (characters horizontal)
       * P = Position (characters)
       */
      L = ((196 - w) / 2) + 2;
      B = L;
      C = 0;
      display.setCursor (L, Design.Menu.Data + Design.Menu.Top);
      if (Updates.Indexing.Active || Updates.Highlighting.Active)
        {
          for (P = 0; P < O.length (); P++)
            {
              T = O.charAt (P);
              if (Updates.Indexing.Active && P == Updates.Indexing.Offset)
                {
                  C = Updates.Indexing.Width;
                  B = L;
                }
              if (Updates.Highlighting.Active)
                display.setTextColor (isBitOn64 (Updates.Highlighting.Bits, P)
                                          ? GxEPD_WHITE
                                          : GxEPD_BLACK);
              display.print (T);
              L = display.getCursorX (); /* Read the Cursor AFTER printing */
              if (Updates.Indexing.Active)
                {
                  if (C > 0)
                    {
                      C--;
                      if (C == 0)
                        {
                          display.fillRect (B, D + 5, L - B, 3, GxEPD_BLACK);
                        }
                    }
                }
            }
        }
      else
        display.print (O);
    }
}
void
WatchyGSR::setFontFor (String O, const GFXfont *Normal, const GFXfont *Small,
                       const GFXfont *Smaller, byte Gutter)
{
  int16_t x1, y1;
  uint16_t w, h;
  display.setTextWrap (false);
  byte wi = (200 - (2 * Gutter));
  display.setFont (Normal);
  display.getTextBounds (O, 0, 0, &x1, &y1, &w, &h);
  if (w > wi)
    {
      display.setFont (Small);
      display.getTextBounds (O, 0, 0, &x1, &y1, &w, &h);
    }
  if (w > wi)
    {
      display.setFont (Smaller);
      display.getTextBounds (O, 0, 0, &x1, &y1, &w, &h);
    }
}
void
WatchyGSR::drawData (String dData, byte Left, byte Bottom,
                     WatchyGSR::DesOps Style, byte Gutter, bool isTime, bool PM)
{
  uint16_t w, Width, Height, Ind;
  int16_t X, Y;

  display.getTextBounds (dData, Left, Bottom, &X, &Y, &Width, &Height);

  Bottom = constrain (Bottom, Gutter, 200 - Gutter);
  switch (Style)
    {
    case WatchyGSR::dLEFT:
      Left = Gutter;
      break;
    case WatchyGSR::dRIGHT:
      Left = constrain (200 - (Gutter + Width), Gutter, 200 - Gutter);
      break;
    case WatchyGSR::dSTATIC:
      Left = constrain (Left, Gutter, 200 - Gutter);
      break;
    case WatchyGSR::dCENTER:
      Left = constrain (4 + ((196 - (Gutter + Width)) / 2), Gutter,
                        200 - Gutter);
      break;
    };
  display.setCursor (Left, Bottom);
  display.print (dData);

  if (isTime && PM)
    {
      if (Style == WatchyGSR::dRIGHT)
        Left = constrain (Left - 12, Gutter, 200 - Gutter);
      else
        Left = constrain (Left + Width + 6, Gutter, 190);
      display.drawBitmap (Left, Bottom - Design.Face.TimeHeight, PMIndicator, 6,
                          6, ForeColor ());
    }
}
void
WatchyGSR::GoDark (bool DeepSleeping)
{
  bool B;
  if (Options.SleepStyle == 0 || (Options.SleepStyle == 2 && !WatchTime.BedTime)
      || GuiMode == GSR_MENUON)
    return;
  if ((Updates.Drawn || !Darkness.Went) && (DeepSleeping || !Showing ()))
    {
      Darkness.Went = true;
      Darkness.Woke = false;
      Darkness.Tilt = 0;
      Updates.Init = Updates.Drawn;
      display.setFullWindow ();
      DisplayInit (true); // Force it here so it fixes the border.
      display.fillScreen (GxEPD_BLACK);
      if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr)
        {
          if (!OverrideSleepBitmap ())
            {
              if (Design.Face.SleepBitmap)
                display.drawBitmap (0, 0, Design.Face.SleepBitmap, 200, 200,
                                    GxEPD_WHITE, GxEPD_BLACK);
            }
        }
      else
        {
          if (!WatchStyles.AddOn[Options.WatchFaceStyle]
                   ->OverrideSleepBitmap ())
            {
              if (Design.Face.SleepBitmap)
                display.drawBitmap (0, 0, Design.Face.SleepBitmap, 200, 200,
                                    GxEPD_WHITE, GxEPD_BLACK);
              else
                B = OverrideSleepBitmap ();
            }
        }
      drawChargeMe (true);
      drawStatus (true);
      drawOverlay (Options.WatchFaceStyle); // Temporary.
      Battery.DarkDirection = Battery.Direction;
      Battery.DarkState = Battery.State;
      display.display (true);
      setDebounce ();
      Updates.Drawn = false;
      display.hibernate ();
      LastHelp = 0;
    }
}
void
WatchyGSR::espPinSetup (gpio_num_t pin, bool pullUp, bool bOutput)
{
  if (GSR_PIN_RTC == 255 && pin < 22)
    {
      rtc_gpio_set_direction (pin, (bOutput ? RTC_GPIO_MODE_OUTPUT_ONLY
                                            : RTC_GPIO_MODE_INPUT_ONLY));
      if (pullUp)
        {
          rtc_gpio_pulldown_dis (pin);
          rtc_gpio_pullup_en (pin);
        }
      else
        {
          rtc_gpio_pullup_dis (pin);
          rtc_gpio_pulldown_en (pin);
        }
    }
}
void
WatchyGSR::adcPins ()
{
  if (GSR_PIN_USB_DET != 255)
    {
      pinMode (GSR_PIN_USB_DET, INPUT);
      pinMode (GSR_PIN_ADC, ANALOG);
      pinMode (GSR_PIN_STAT, ANALOG);
    }
}
void
WatchyGSR::ForceInputs ()
{
  if (GSR_PIN_RTC == 255)
    {
      espPinSetup ((gpio_num_t)GSR_UP_PIN, true, false);
      espPinSetup ((gpio_num_t)GSR_PIN_USB_DET, false, false);
      espPinSetup ((gpio_num_t)GSR_PIN_ACC_INT1, true, false);
      //    esp_sleep_enable_ext0_wakeup((gpio_num_t)GSR_PIN_USB_DET,
      //    digitalRead(GSR_PIN_USB_DET) ? HIGH : LOW);
      return;
    }
  uint8_t P = SRTC.getADCPin ();
  /* Unused GPIO PINS */
  pinMode (0, INPUT); /* BTN3 v3 */
  pinMode (2, INPUT); /*  ??  */
  if (GSR_PIN_ACC_INT2 != 13)
    pinMode (13, INPUT); /*  ??  */
  pinMode (15, INPUT);   /*  ??  */
  pinMode (20, INPUT);   /*  ??  */
  pinMode (36, INPUT);   /*  ??  */
  pinMode (37, INPUT);   /*  ??  */
  pinMode (38, INPUT);   /*  ??  */
  pinMode (39, INPUT);   /*  ??  */
                         /* RS232 */
  pinMode (3, INPUT);    /*  RX  */
                         /* BUTTONS */
  pinMode (26, INPUT);   /* BTN1 */
  pinMode (25, INPUT);   /* BTN2 */
  if (GSR_UP_PIN != 32)
    pinMode (32, INPUT); /* BTN3     */
  if (!(GSR_UP_PIN == 35 || P == 35))
    pinMode (35, INPUT); /* ADC/BTN3 */
  pinMode (4, INPUT);    /* BTN4 */
                         /* RTC */
  pinMode (21, INPUT);   /* SDA  */
  pinMode (22, INPUT);   /* SCL  */
                         /* DISPLAY */
  pinMode (5, INPUT);    /*  CS  */
  if (P != 9)
    pinMode (9, INPUT); /* RES/ADC v3  */
  pinMode (10, INPUT);  /*  DC  */
  pinMode (18, INPUT);  /* SLCK */
  pinMode (19, INPUT);  /* BUSY */
  pinMode (23, INPUT);  /* MOSI */
                        /* BMA 423 */
  pinMode (12, INPUT);  /* INT2/SDA v3 */
  pinMode (14, INPUT);  /* INT1 */
}
void
WatchyGSR::detectBattery (bool booting)
{
  float BATOff, Diff, Uping;
  uint8_t Mins;
  bool B = false;
  time_t tmp;
  if (!((Battery.ReadFloat[Battery.FloatTop].Stamp + 60 > WatchTime.UTC_RAW
         && !booting)
        || GSRWiFi.Requests || WiFiInProgress ()))
    { // || (HWVer = 3.0f && millis() < 475)
      /* <Unusable, V3 has a ~500ms spike on most GPIO, rendering them unusable
       * for that duration after boot> if
       * ((Battery.ReadFloat[Battery.FloatTop].Stamp + 60 > WatchTime.UTC_RAW)
       * && GSR_PIN_STAT == 255) return; if (GSR_PIN_STAT == 255){
       */
      BATOff = rawBatteryVoltage ();
      tmp = WatchTime.UTC_RAW - WatchTime.UTC.Second;
      if (BATOff < 330.00f)
        return; // Bugged.
      if (!Battery.floatInit)
        {
          Battery.floatInit = true;
          for (Mins = 0; Mins < 5; Mins++)
            {
              Battery.ReadFloat[Mins].Voltage = BATOff;
              Battery.ReadFloat[Mins].Stamp = tmp;
            }
        }
      Battery.FloatTop = roller (Battery.FloatTop + 1, 0, 4);
      Battery.FloatBottom = roller (Battery.FloatBottom + 1, 0, 4);
      Battery.ReadFloat[Battery.FloatTop].Stamp = tmp;
      Battery.ReadFloat[Battery.FloatTop].Voltage = BATOff;
      Battery.Read = Battery.ReadFloat[Battery.FloatTop].Voltage / 100;
      Diff = (Battery.ReadFloat[Battery.FloatTop].Stamp
              - Battery.ReadFloat[Battery.FloatBottom].Stamp)
             / 60;
      Mins = floor (Diff);
      Diff = Battery.ReadFloat[Battery.FloatTop].Voltage
             - Battery.ReadFloat[Battery.FloatBottom].Voltage;
      BATOff = round (Diff * 100) / 100;
      if (Mins < 1)
        return; // No minute.
      Uping = (Darkness.Went && Mins > 3
               && Battery.ReadFloat[Battery.FloatTop].Voltage < 425.00f)
                  ? (Mins * GSR_ChargePerMin)
                  : GSR_ChargePerMin;
      if (GSR_PIN_STAT != 255)
        {
          if (analogRead (GSR_PIN_STAT) * 0.80586 < 3299)
            BATOff = -11.00f;
        }
      if (BATOff < -10.00f)
        Battery.Level
            = -2; /* Battery sees a massive jump due to it being unplugged */
      else if (BATOff < 0.00f)
        {
          Battery.Level = gobig (Battery.Level - 1, -2);
        } /* Drop down to -2 for discharging */
      else if (BATOff > 10.00f)
        Battery.Level
            = 2; /* Battery sees a massive jump due to it being plugged in */
      else if (BATOff > Uping)
        Battery.Level
            = golow (Battery.Level + 1, 2); /* Jump up to 2 for charging */
      /* <Unusable, V3 has a ~500ms spike on most GPIO, rendering them unusable
       * for that duration after boot> }else { B = Battery.Charge;
       *        Battery.Charge = (analogRead(GSR_PIN_STAT) * 0.80586 > 3200); //
       * && digitalRead(GSR_PIN_USB_DET) Battery.Read = rawBatteryVoltage() /
       * 100; Battery.Level = (Battery.Charge ? 2 : -2); if (B !=
       * Battery.Charge) { Updates.Drawn = true; UpdateDisp |= Showing(); } B =
       * false;
       *    }
       */
    }
  if (Battery.Level > 1)
    {
      Battery.Direction = 1;
    }
  else if (Battery.Level < 0)
    {
      Battery.Direction = -1;
    }
  if (syncToday ())
    B |= (WatchTime.Local.Hour == NTPData.SyncHour
          && WatchTime.Local.Minute == NTPData.SyncMins);
  B |= (SRTC.checkingDrift (WatchTime.ESPRTC) && NTPData.driftSync
        && WatchTime.UTC_RAW > NTPData.driftSyncNOW);

  Mins
      = Battery.State; /* Set update for display if the battery state changes */

  // Do battery state here.
  if (WatchTime.UTC_RAW > Battery.LastState)
    { // || GSR_PIN_STAT != 255){
      Battery.LastState = WatchTime.UTC_RAW + 10;
      if (Battery.Direction > 0)
        Battery.State = 3;
      else
        Battery.State = (Battery.Read > getLowBattery (true)
                             ? (Battery.Read > getLowBattery () ? 0 : 1)
                             : 2);
      B |= (Mins != Battery.State && Options.ntpAtCharge && Battery.State == 3
            && WatchTime.UTC_RAW - NTPData.Last > 14399); // Charging.
      if (Mins != Battery.State
          || (Darkness.Went
              && (Battery.DarkState != Battery.State
                  || Battery.DarkDirection != Battery.Direction)))
        {
          Updates.Drawn = true;
          UpdateDisp |= Showing ();
        } // Fix the battery indicator.
    }
  if (Battery.Read > getLowBatteryRadio () && B
      && WatchTime.UTC_RAW - NTPData.Last > 59)
    StartNTP (true);
}
bool
WatchyGSR::inBrownOut ()
{
  float B = golow (getBatteryVoltage (), 4.22);
  return (B && B < 3.36 && Battery.Start >= 3.4);
}

/*  RTC_CNTL_BROWN_OUT_REG:
 *  CHIP_ESP32   = 1, //!< ESP32     3FF480D4
 *  CHIP_ESP32S2 = 2, //!< ESP32-S2  3F4080E4
 *  CHIP_ESP32S3 = 9, //!< ESP32-S3  600080E8
 *  CHIP_ESP32C3 = 5, //!< ESP32-C3  600080D8

#define CHIP_FEATURE_EMB_FLASH      BIT(0)      //!< Chip has embedded flash
memory #define CHIP_FEATURE_WIFI_BGN       BIT(1)      //!< Chip has 2.4GHz WiFi
#define CHIP_FEATURE_BLE            BIT(4)      //!< Chip has Bluetooth LE
#define CHIP_FEATURE_BT             BIT(5)      //!< Chip has Bluetooth Classic
#define CHIP_FEATURE_IEEE802154     BIT(6)      //!< Chip has IEEE 802.15.4

 */

void
WatchyGSR::BrownOutDetect (bool On)
{
  if (Watchy_Chip_Info.BrownOutDetection && GSR_PIN_RTC != 255)
    WRITE_PERI_REG (Watchy_Chip_Info.BrownOutDetection, (On ? 1 : 0));
}
void
WatchyGSR::SetupESPValues ()
{
  uint32_t F; // Feature.
  esp_chip_info_t Chip_Info[sizeof (esp_chip_info_t)];
  esp_chip_info (Chip_Info);
  if (Chip_Info->model == CHIP_ESP32)
    {
      F = Chip_Info->features;
      Watchy_Chip_Info.Base = 0x3FF48000;
      Watchy_Chip_Info.BrownOutDetection = Watchy_Chip_Info.Base + 0xD4;
    }
  else if (Chip_Info->model == CHIP_ESP32S2)
    {
      F = Chip_Info->features;
      Watchy_Chip_Info.Base = 0x3F408000;
      Watchy_Chip_Info.BrownOutDetection = Watchy_Chip_Info.Base + 0xD8;
    }
  else if (Chip_Info->model == CHIP_ESP32S3)
    {
      F = Chip_Info->features;
      Watchy_Chip_Info.Base = 0x60008000;
      Watchy_Chip_Info.BrownOutDetection = Watchy_Chip_Info.Base + 0xE8;
    }
  else if (Chip_Info->model == CHIP_ESP32C3)
    {
      F = Chip_Info->features;
      Watchy_Chip_Info.Base = 0x60008000;
      Watchy_Chip_Info.BrownOutDetection = Watchy_Chip_Info.Base + 0xD8;
    }
  if (F)
    {
      Watchy_Chip_Info.HasWiFi
          = (F & (CHIP_FEATURE_WIFI_BGN | CHIP_FEATURE_IEEE802154));
      Watchy_Chip_Info.HasBLE = (F & CHIP_FEATURE_BLE);
      Watchy_Chip_Info.HasBT
          = ((F & CHIP_FEATURE_BT) | Watchy_Chip_Info.HasBLE);
    }
}
void
WatchyGSR::espPinModes ()
{
  uint8_t i;
  float f;
  pinMode (GSR_MENU_PIN, (HWVer == 3.0f ? INPUT_PULLUP : INPUT));
  pinMode (GSR_BACK_PIN, (HWVer == 3.0f ? INPUT_PULLUP : INPUT));
  pinMode (GSR_UP_PIN, (HWVer == 3.0f ? INPUT_PULLUP : INPUT));
  pinMode (GSR_DOWN_PIN, (HWVer == 3.0f ? INPUT_PULLUP : INPUT));
  if (HWVer == 3.0f)
    {
      for (i = 0; i < 36; i++)
        f = getBatteryVoltage ();
    }
}
// Obtains the SCL and SDA values before use (if not already present).
void
WatchyGSR::startSetup ()
{
  if (GSR_PIN_SDA == 255)
    {
      if (WatchyGSR::isESP32S3 ())
        {
          GSR_PIN_SCL = 11;
          GSR_PIN_SDA = 12;
          GSR_ESP_WAKEUP = 0;
          GSR_PIN_MISO = 46;
          GSR_PIN_SCK = 47;
          GSR_PIN_MOSI = 48;
          GSR_PIN_SS = 33;
        }
      else
        {
          GSR_PIN_SCL = 22;
          GSR_PIN_SDA = 21;
          GSR_PIN_RTC = 27;
          GSR_ESP_WAKEUP = 1;
          GSR_PIN_SCK = 18;
          GSR_PIN_MISO = 19;
          GSR_PIN_MOSI = 23;
          GSR_PIN_SS = 5;
        }
    }
  if (i2cIsInit (0))
    Wire.end ();
  Wire.begin (GSR_PIN_SDA, GSR_PIN_SCL);
}
bool
WatchyGSR::isESP32S3 ()
{
  esp_chip_info_t chip_info[sizeof (esp_chip_info_t)];
  esp_chip_info (chip_info);
  return (chip_info->model == CHIP_ESP32S3);
}
// Sets up the pin definitions for the current Watchy hardware.
void
WatchyGSR::getPins (float Version)
{
  uint8_t V = (Version * 10);
  GSR_MENU_PIN = 26;
  GSR_BACK_PIN = 25;
  GSR_UP_PIN = 32;
  GSR_DOWN_PIN = 4;
  GSR_PIN_ADC = 33;
  GSR_PIN_SCL = 22;
  GSR_PIN_SDA = 21;
  GSR_PIN_ACC_INT1 = 14;
  GSR_MASK_ACC_INT = ((uint64_t)(((uint64_t)1) << 14)); // GPIO_SEL_14;
  GSR_PIN_ACC_INT2 = 12;
  GSR_PIN_VIB_PWM = 13;
  switch (V)
    {
    case 15:
      GSR_PIN_ADC = 35;
      break;
    case 20:
      GSR_UP_PIN = 35;
      GSR_PIN_ADC = 34;
      break;
    case 30:
      GSR_MENU_PIN = 7;
      GSR_BACK_PIN = 6;
      GSR_UP_PIN = 0;
      GSR_DOWN_PIN = 8;
      GSR_PIN_ADC = 9;
      GSR_PIN_STAT = 10;
      GSR_PIN_SCL = 11;
      GSR_PIN_SDA = 12;
      GSR_PIN_ACC_INT2 = 13;
      GSR_PIN_VIB_PWM = 17;
      GSR_PIN_USB_DET = 21;
      break;
    }
  GSR_MENU_MASK = ((uint64_t)(((uint64_t)1) << GSR_MENU_PIN));
  GSR_BACK_MASK = ((uint64_t)(((uint64_t)1) << GSR_BACK_PIN));
  GSR_UP_MASK = ((uint64_t)(((uint64_t)1) << GSR_UP_PIN));
  GSR_DOWN_MASK = ((uint64_t)(((uint64_t)1) << GSR_DOWN_PIN));
  GSR_BTN_MASK = GSR_MENU_MASK | GSR_BACK_MASK | GSR_UP_MASK | GSR_DOWN_MASK;
}
uint16_t
WatchyGSR::getDispCS ()
{
  return (WatchyGSR::isESP32S3 () ? 33 : 5);
}
uint16_t
WatchyGSR::getDispDC ()
{
  return (WatchyGSR::isESP32S3 () ? 34 : 10);
}
uint16_t
WatchyGSR::getDispRES ()
{
  return (WatchyGSR::isESP32S3 () ? 35 : 9);
}
uint16_t
WatchyGSR::getDispBSY ()
{
  return (WatchyGSR::isESP32S3 () ? 36 : 19);
}
void
WatchyGSR::ProcessNTP ()
{
  bool B, y;
  time_t tmp;
  uint8_t I;
  if (UpdateDisp || GSRWiFi.Slow > 0)
    return;
  // Do ProgressNTP here.
  switch (NTPData.State)
    {
    // Start WiFi and Connect.
    case 1:
      {
        if (!GetAskWiFi ())
          {
            if (rawBatteryVoltage (true) > getLowBatteryRadio ())
              {
                AskForWiFi ();
              }
            else if (NTPData.Wait > 2)
              {
                NTPData.Pause = 0;
                NTPData.State = 99;
              }
            break;
          }
        if (WiFiInProgress () && !GSRWiFi.Results)
          {
            if (NTPData.Wait > 2)
              {
                NTPData.Pause = 0;
                NTPData.State = 99;
              }
            break;
          }
        NTPData.Wait = 0;
        NTPData.Pause = 10;
        NTPData.State++;
        break;
      }

    // Am I Connected?  If so, ask for NTP.
    case 2:
      {
        if (WiFiStatus () != WL_CONNECTED)
          {
            if (currentWiFi () == WL_CONNECT_FAILED || NTPData.Wait > 2)
              {
                NTPData.Pause = 0;
                NTPData.State = 99;
                break;
              }
          }
        if (NTPData.TimeZone == false)
          {
            NTPData.State = 5;
            NTPData.Pause = 0;
            break;
          }

        HTTP.setUserAgent (UserAgent);
        NTPData.State++;
        setStatus ("TZ");
        // Do the next part.
        if (!OP.beginOlsonFromWeb (WiFiC))
          {
            NTPData.Pause = 0;
            NTPData.State = 99;
            break;
          }
        NTPData.Wait = 0;
        NTPData.tWait = millis () + 4000;
        NTPData.Pause = 1;
        break;
      }

    case 3:
      {
        // Get GeoLocation
        if (WiFiStatus () == WL_DISCONNECTED)
          {
            NTPData.Pause = 0;
            NTPData.State = 99;
            break;
          }
        if (OP.gotOlsonFromWeb ())
          {
            WatchTime.TimeZone = OP.getCurrentOlson ();
            OP.endOlsonFromWeb ();
            NTPData.Wait = 0;
            NTPData.Pause = 0;
            NTPData.State++; // Test
          }
        else if (NTPData.tWait < millis ())
          {
            NTPData.Pause = 0;
            NTPData.Wait = 0;
            NTPData.State = 5;
            OP.endOlsonFromWeb ();
          }
        if (OP.getOlsonWebError ())
          LastWebError = OP.getOlsonWebError ();
        break;
      }

    case 4:
      {
        // Process Timezone POSIX.
        String NewTZ = OP.getPOSIX (WatchTime.TimeZone);
        if (NewTZ == OP.TZMISSING)
          {
            NTPData.Pause = 0;
            NTPData.State = 99;
            break;
          }
        if (OkNVS (GSR_GName))
          {
            B = NVS.setString (GSR_GTZ, NewTZ);
            NVS.commit ();
          }
        OP.setCurrentPOSIX (NewTZ);
        NTPData.Wait = 0;
        NTPData.Pause = 0;
        NTPData.State++;
        if (!NTPData.UpdateUTC)
          UpdateDisp |= Showing ();
        break;
      }

    case 5:
      {
        if (NTPData.UpdateUTC == false || WiFiStatus () == WL_DISCONNECTED
            || NTPData.Wait > 0)
          {
            NTPData.State = 99;
            NTPData.Pause = 0;
            break;
          }
        setStatus ("NTP");
        SNTP.Begin (overrideNTPfromWeb (getCurrentNTPServer ()));
        NTPData.Wait = 0;
        NTPData.tWait = millis () + 4000;
        NTPData.Pause = 01;
        NTPData.State++;
        break;
      }

    case 6:
      {
        if (!SNTP.Query ())
          {
            if (NTPData.tWait < millis ())
              {
                NTPData.Pause = 0;
                NTPData.State = 99;
              }
            break;
          }
        tmp = WatchTime.UTC_RAW;
        WatchTime.UTC_RAW = SNTP.Results;
        tmp = WatchTime.UTC_RAW - tmp;
        tmp -= (tmp % 60); // Remove remainder.
        for (I = 0; I < 5; I++)
          {
            Battery.ReadFloat[I].Stamp += tmp;
          }
        B = (Options.NTPAuto && NTPData.driftSync);
        y = SRTC.checkingDrift (WatchTime.ESPRTC);
        if (B && y && WatchTime.UTC_RAW >= NTPData.driftSyncNOW)
          {
            SRTC.endDrift (SNTP.tmResults, WatchTime.ESPRTC);
            NTPData.driftSync = false;
          }
        else if (B && !y)
          {
            WatchTime.UTC = SNTP.tmResults;
            SRTC.beginDrift (WatchTime.UTC, WatchTime.ESPRTC);
            UpdateClock ();
            NTPData.driftSyncNOW = WatchTime.UTC_RAW + 90000UL;
          }
        else
          {
            WatchTime.UTC = SNTP.tmResults;
            SRTC.set (WatchTime.UTC);
            SRTC.pauseDrift (false);
          }
        NTPData.Pause = 0;
        NTPData.State = 99;
        UpdateClock ();
        break;
      }

    case 99:
      {
        SNTP.End ();
        if (NTPData.driftSync)
          NTPData.driftSyncNOW += 3600UL;
        if (WeatherData.State == 0)
          endWiFi (); // Leave it open for weather.
        NTPData.Wait = 0;
        NTPData.Pause = 0;
        NTPData.State = 0;
        NTPData.Last = WatchTime.UTC_RAW; // Moved from section 6 to here, to
                                          // limit the atttempts.
        NTPData.UpdateUTC = false;
        NTPData.TimeZone = false;
        setStatus ("");
      }
    }
}
void
WatchyGSR::UpdateUTC ()
{
  SRTC.read (WatchTime.UTC);
  WatchTime.UTC_RAW = SRTC.doMakeTime (WatchTime.UTC);
  SRTC.doBreakTime (WatchTime.UTC_RAW, WatchTime.UTC);
}
void
WatchyGSR::UpdateClock ()
{
  bool B = WatchTime.BedTime;
  WatchTime.Local = UTCtoLocal (WatchTime.UTC_RAW);
  WatchTime.Local_RAW = SRTC.doMakeTime (WatchTime.Local);
  if (Options.SleepEnd > Options.SleepStart)
    WatchTime.BedTime = (WatchTime.Local.Hour >= Options.SleepStart
                         && WatchTime.Local.Hour < Options.SleepEnd);
  else
    WatchTime.BedTime = (WatchTime.Local.Hour >= Options.SleepStart
                         || WatchTime.Local.Hour < Options.SleepEnd);
  /* SORT OUT THE NIGHTTIME Wallpaper Change */
  if (WatchTime.BedTime != B)
    {
      Updates.Drawn = true;
    }              // Fake we messed up the screen.
  monitorSteps (); // Moved here for accuracy during Deep Sleep.
}
bool
WatchyGSR::TimerAbuse ()
{
  bool B = (Menu.SubItem > 1 && Menu.SubItem < 7 && Menu.Item == GSR_MENU_TOFF);
  bool A = ((TimerUp.Active && Menu.Item == GSR_MENU_TIMEUP) || B);
  if (A)
    UpdateDisp |= Showing ();
  return (A || (CountdownTimerActive () && Menu.Item == GSR_MENU_TIMEDN)
          || (CountdownTimerActive ()
              && ((Alarms.getTimerDownHours () * 3600)
                  + (Alarms.getTimerDownMinutes () * 60)
                  + Alarms.getTimerDownSeconds ())
                     < 60)
          || B);
}
void
WatchyGSR::KeysStart ()
{
  if (KeysHandle == NULL)
    {
      KeysRet = xTaskCreate (WatchyGSR::KeysCheck, "WatchyGSR_KeysCheck", 1024,
                             NULL, (tskIDLE_PRIORITY + 1), &KeysHandle);
    }
}
void
WatchyGSR::KeysStop ()
{
  while (KeysHandle != NULL)
    {
      KeysCheckOn = false;
      vTaskDelay (20 / portTICK_PERIOD_MS);
    }
}
void
WatchyGSR::drawLogOutput ()
{
  int16_t lleft, lbottom, lx, ly;
  uint16_t lwidth, lheight;
  if (GSRLogOutput > "")
    {
      setFontFor (GSRLogOutput, Design.Menu.Font, Design.Menu.FontSmall,
                  Design.Menu.FontSmaller, 0);
      display.getTextBounds (GSRLogOutput, lleft, lbottom, &lx, &ly, &lwidth,
                             &lheight);
      display.fillRect (lleft, 0, lwidth, lheight, BackColor ());
      display.setCursor (lleft, lbottom + 16);
      display.print (GSRLogOutput);
      GSRLogOutput = "";
    }
}
void
WatchyGSR::KeysCheck (void *parameter)
{
  bool Ok;
  uint8_t B;
  KeysCheckOn = true;
  while (KeysCheckOn)
    {
      Ok = !(Options.SleepStyle == 4 && !Updates.Tapped);
      if (Ok && (LastButton == 0 || (millis () - LastButton) > Debounce ())
          && Missed == 0 && Button == 0)
        {
          B = WatchyGSR::getButtonPins ();
          if (B)
            Button = B;
        }
      vTaskDelay (50 / portTICK_PERIOD_MS); // 100ms pauses.
    }
  KeysHandle = NULL;
  vTaskDelete (NULL);
}
time_t
WatchyGSR::tmTOtime_t (tm intm, bool flatten)
{
  tmElements_t tmp;
  tmp.Second = 0;
  tmp.Minute = intm.tm_min;
  tmp.Hour = intm.tm_hour;
  tmp.Wday = 0;
  tmp.Day = intm.tm_mday;
  tmp.Month = intm.tm_mon;
  tmp.Year = intm.tm_year
             - (flatten ? 70 : 0); // Take 70 years off to bring it to 1900.
  return SRTC.doMakeTime (tmp);
}
// Manage time will determine if the RTC is in use, will also set a flag to "New
// Minute" for the loop functions to see the minute change.
void
WatchyGSR::ManageTime ()
{
  tmElements_t TM; //    struct tm * tm;
  uint32_t Test = millis ();
  int I;
  bool b = true;

  if (SRTC.isNewMinute () || WatchTime.NewMinute)
    {
      log_w ("New Minute!");
      UpdateUTC ();
      WatchTime.NewMinute = true;
      b = false;
      WfNM = false;
    }

  if (WatchTime.NewMinute)
    {
      UpdateDisp |= Showing ();
      if (b)
        UpdateUTC ();
      UpdateClock ();
      if (Options.Game != 255)
        if (WatchStyles.AddOn[Options.Game] != nullptr)
          WatchStyles.AddOn[Options.Game]->InsertOnMinute ();
      if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr)
        InsertOnMinute ();
      else
        WatchStyles.AddOn[Options.WatchFaceStyle]->InsertOnMinute ();
    }

  /* ALARMS */
  Alarms.CalculateTones ();
}
void
WatchyGSR::_bmaConfig ()
{
  uint8_t Type = SRTC.getType ();

#ifdef STABLEBMA_H_INCLUDED
  if (SBMA.begin (_readRegister, _writeRegister, vTaskDelay, Type,
                  BMA4_I2C_ADDR_PRIMARY, (HWVer < 3.0f), GSR_PIN_ACC_INT1)
      == false)
    {
      // fail to init BMA
      return;
    }

  if (!SBMA.defaultConfig ())
    return; // Failed.  // HWVer != 3.0
  // Enable BMA423 isStepCounter feature
  SBMA.enableFeature (BMA423_STEP_CNTR, true);
#endif
}
void
WatchyGSR::UpdateBMA ()
{
  bool BT = (Options.SleepStyle == 2 && WatchTime.BedTime);
  bool B = (Options.SleepStyle > 2 && Options.SleepStyle != 4);
  bool A = (Options.SleepStyle == 1);

#ifdef STABLEBMA_H_INCLUDED
  SBMA.enableDoubleClickWake (B | BT);
  SBMA.enableTiltWake ((A | B) & !WatchTime.BedTime);
#endif
}
uint16_t
WatchyGSR::_readRegister (uint8_t address, uint8_t reg, uint8_t *data,
                          uint16_t len)
{
  Wire.beginTransmission (address);
  Wire.write (reg);
  Wire.endTransmission ();
  Wire.requestFrom ((uint8_t)address, (uint8_t)len);
  uint8_t i = 0;
  while (Wire.available ())
    {
      data[i++] = Wire.read ();
    }
  return 0;
}
uint16_t
WatchyGSR::_writeRegister (uint8_t address, uint8_t reg, uint8_t *data,
                           uint16_t len)
{
  Wire.beginTransmission (address);
  Wire.write (reg);
  Wire.write (data, len);
  return (0 != Wire.endTransmission ());
}
bool
WatchyGSR::syncToday ()
{
  return !SRTC.checkingDrift (WatchTime.ESPRTC) && NTPData.AutoSync
         && (NTPData.SyncDays & Bits[WatchTime.Local.Wday]);
}
String
WatchyGSR::MakeTOD (uint8_t Hour, bool AddZeros)
{
  if (Options.TwentyFour)
    {
      if (AddZeros)
        return ":00";
      return "";
    }
  return " " + LGSR.GetID (Options.LanguageID, (Hour > 11 ? 114 : 115));
}
String
WatchyGSR::MakeSeconds (uint8_t Seconds)
{
  return LGSR.GetID (Options.LanguageID, (Seconds > 1 ? 112 : 113)) + ".";
}
String
WatchyGSR::MakeSteps (uint32_t uSteps)
{
  String S, T, X;
  uint8_t I, C;

  S = String (uSteps);
  I = S.length ();
  C = 0;
  T = "";

  while (I > 0)
    {
      X = (I > 1 && C == 2) ? "," : "";
      T = X + S.charAt (I - 1) + T;
      C = roller (C + 1, 0, 2);
      I--;
    }
  return T;
}
String
WatchyGSR::getReduce (uint8_t Amount)
{
  switch (Amount)
    {
    case 0:
      return LGSR.GetID (Options.LanguageID, 116);
    case 1:
      return "80%";
    case 2:
      return "60%";
    case 3:
      return "40%";
    }
  return "20%";
}
// Catches Steps and moves "Yesterday" into the other setting.
void
WatchyGSR::monitorSteps ()
{
  bool B;
  if (Steps.Hour == WatchTime.Local.Hour
      && Steps.Minutes == WatchTime.Local.Minute)
    {
      if (!Steps.Reset)
        {
          Steps.Yesterday = CurrentStepCount ();
#ifdef STABLEBMA_H_INCLUDED
          SBMA.resetStepCounter ();
#endif
          Steps.Cached = 0;
          Steps.Reset = true;
        }
    }
  else if (Steps.Reset)
    Steps.Reset = false;
  /* Store steps in NVS if it has been a minute+ since last store */
  if (WatchTime.UTC_RAW - Steps.Stored > 59)
    {
      Steps.Stored = WatchTime.UTC_RAW;
      if (OkNVS (GSR_GName))
        {
          B = NVS.setString (GSR_STP, String (CurrentStepCount ()));
          B = NVS.setString (GSR_YSTP, String (Steps.Yesterday));
          NVS.commit ();
        }
    }
}
uint8_t
WatchyGSR::getButtonMaskToID (uint64_t HW)
{
  uint8_t HB = getSWValue ((HW & GSR_MENU_MASK), (HW & GSR_BACK_MASK),
                           (HW & GSR_UP_MASK), (HW & GSR_DOWN_MASK));
  uint8_t LB = WatchyGSR::getButtonPins ();
#ifdef STABLEBMA_H_INCLUDED
  if (SBMA.didBMAWakeUp (HW))
    { // Acccelerometer.
      if (SBMA.isDoubleClick ())
        return 9; // Double Tap.
      else if (SBMA.isTilt ())
        return 10; // Wrist Tilt.
    }
#endif
  if (LB > 0 && LB != HB && GSR_PIN_RTC != 255)
    HB = LB; // V3 doesn't allow custom buttons at boot time.
  return HB;
}
uint8_t
WatchyGSR::getSWValue (bool SW1, bool SW2, bool SW3, bool SW4)
{
  bool T;
  if (Options.Swapped)
    {
      T = SW1;
      SW2 = SW1;
      SW1 = T;
    }
  if (Options.Lefty)
    {
      T = SW1;
      SW1 = SW4;
      SW4 = T;
      T = SW3;
      SW3 = SW2;
      SW2 = T;
    }
  if ((SW1 | SW2 | SW3 | SW4))
    {
      if (SW3 && SW1)
        return 5;
      if (SW3 && SW2 && HWVer != 3.0f)
        return 6;
      if (SW4 && SW1)
        return 7;
      if (SW4 && SW2)
        return 8;
      if (SW1)
        return 1;
      if (SW2)
        return 2;
      if (SW3)
        return 3;
      if (SW4)
        return 4;
    }
  return 0;
}
void
WatchyGSR::SetAskWiFi (bool SetWiFi)
{
  if (GuiMode != GSR_GAMEON)
    WatchStyles.Options[Options.WatchFaceStyle]
        = (WatchStyles.Options[Options.WatchFaceStyle] & ~GSR_AFW)
          | (SetWiFi ? GSR_AFW : 0);
}
bool
WatchyGSR::HasIPAddress ()
{
  if (WiFi.status () == WL_CONNECTED && GSRWiFi.Powered)
    {
      GSRWiFi.LocalIP = WiFi.localIP ();
      return !(GSRWiFi.LocalIP[0] == 0 || GSRWiFi.LocalIP[3] == 0
               || (GSRWiFi.LocalIP[0] == 169 && GSRWiFi.LocalIP[1] == 254));
    }
  return false;
}
bool
WatchyGSR::WiFiInProgress ()
{
  return (GSRWiFi.Requests > 0
          && (GSRWiFi.Requested || GSRWiFi.Working || GSRWiFi.Results));
}
void
WatchyGSR::processWiFiRequest ()
{
  wl_status_t WiFiE = WL_CONNECT_FAILED;
  wifi_config_t conf;
  String AP, PA, O;
  uint8_t I;

  if (Alarms.soundsFiring ())
    return; // Don't do this while the buzzer is on (trying to avoid brownouts).
  if (GSRWiFi.Requests > 0)
    {
      if (!GSRWiFi.Requested)
        {
          RefreshCPU (GSR_CPUMAX);
          ResetOTA ();
          GSRWiFi.Powered = false;
          if (GSRWiFi.WiFiEventID == 0)
            GSRWiFi.WiFiEventID = WiFi.onEvent (
                WatchyGSR::WiFiStationDisconnected,
                ARDUINO_EVENT_WIFI_STA_DISCONNECTED); // SYSTEM_EVENT_STA_DISCONNECTED
          GSRWiFi.Index = 0;
          GSRWiFi.Tried = false;
          GSRWiFi.Last = 0;
          GSRWiFi.Slow = 3;
          GSRWiFi.Requested = true;
          GSRWiFi.Working = true;
          return;
        }
    }

  if (WiFiInProgress ())
    {
      if (WatchyGSR::getButtonPins () != 2)
        OTATimer = millis (); // Not pressing "BACK".
      if (millis () - OTATimer > 10000 || millis () > OTAFail || IsEndOTA ())
        OTAEnd = true; // Fail if holding back for 10 seconds OR 600 seconds has
                       // passed.
    }

  if (GSRWiFi.Working)
    {
      if (HasIPAddress ())
        {
          GSRWiFi.Working = false;
          setStatus (WiFiIndicator (GSRWiFi.Index ? GSRWiFi.Index : 24));
          GSRWiFi.Results = true;
          return;
        } // We got connected.
      if (millis () > GSRWiFi.Last)
        {
          if (!GSRWiFi.Prepped)
            {
              RefreshCPU (GSR_CPUMAX);
              WiFi.disconnect ();
              WiFi.mode (WIFI_STA);
              GSRWiFi.Powered = true;
              if (GSRWiFi.Tried)
                {
                  GSRWiFi.Index++;
                }
              GSRWiFi.Tried = false;
              GSRWiFi.Prepped = true;
              GSRWiFi.Slow = 1;
              UpdateScreen ();
              WiFi.setHostname (WiFi_AP_SSID);
              return;
            }
          if (GSRWiFi.Index < 11 && GSRWiFi.Prepped && GSRWiFi.Powered
              && !inBrownOut ()
              && rawBatteryVoltage (true) > getLowBatteryRadio ())
            {
              GSRWiFi.Prepped = false;
              WatchyOStatus = "";
              if (GSRWiFi.Index == 0)
                {
                  GSRWiFi.Tried = true;
                  GSRWiFi.Slow = 3;
                  if (WiFi_DEF_SSID > "")
                    {
                      WiFiE = WiFi.begin (WiFi_DEF_SSID, WiFi_DEF_PASS);
                      UpdateWiFiPower (WiFi_DEF_SSID, WiFi_DEF_PASS);
                      esp_wifi_set_ps (WIFI_PS_MAX_MODEM);
                      setStatus (
                          WiFiIndicator (GSRWiFi.Index ? GSRWiFi.Index : 24)
                          + "!");
                    }
                  else
                    {
                      WiFiE = WiFi.begin ();
                      esp_wifi_get_config ((wifi_interface_t)WIFI_IF_STA,
                                           &conf);
                      UpdateWiFiPower (
                          reinterpret_cast<const char *> (conf.sta.ssid),
                          reinterpret_cast<const char *> (conf.sta.password));
                      esp_wifi_set_ps (WIFI_PS_MAX_MODEM);
                      setStatus (
                          WiFiIndicator (GSRWiFi.Index ? GSRWiFi.Index : 24)
                          + "!");
                    }
                  GSRWiFi.Last = millis () + 9000;
                  return;
                }
              if (GSRWiFi.Index > 0)
                {
                  AP = APIDtoString (GSRWiFi.Index - 1);
                  PA = PASStoString (GSRWiFi.Index - 1);
                  if (AP.length () > 0)
                    {
                      GSRWiFi.Tried = true;
                      WiFiE = WiFi.begin (AP.c_str (), PA.c_str ());
                      GSRWiFi.Slow = 3;
                      setStatus (
                          WiFiIndicator (GSRWiFi.Index ? GSRWiFi.Index : 24)
                          + "!");
                      UpdateWiFiPower (GSRWiFi.AP[GSRWiFi.Index].TPWRIndex);
                      esp_wifi_set_ps (WIFI_PS_MAX_MODEM);
                      GSRWiFi.Last = millis () + 9000;
                    }
                  else
                    GSRWiFi.Index++;
                }
            }
          else
            endWiFi ();
        }
    }
}
String
WatchyGSR::WiFiIndicator (uint8_t Index)
{
  return String (WiFiTXT) + String ((char)('@' + Index));
}
void
WatchyGSR::UpdateWiFiPower (String SSID, String PSK)
{
  uint8_t I;
  for (I = 0; I < 11; I++)
    {
      if (APIDtoString (I) == SSID && PASStoString (I) == PSK)
        {
          UpdateWiFiPower (GSRWiFi.AP[I].TPWRIndex);
          return;
        }
    }
  UpdateWiFiPower ();
}
void
WatchyGSR::UpdateWiFiPower (String SSID)
{
  uint8_t I;
  for (I = 0; I < 11; I++)
    {
      if (APIDtoString (I) == SSID)
        {
          UpdateWiFiPower (GSRWiFi.AP[I].TPWRIndex);
          return;
        }
    }
  UpdateWiFiPower ();
}
void
WatchyGSR::UpdateWiFiPower (uint8_t PWRIndex)
{
  wifi_power_t CW = WiFi.getTxPower ();
  bool B;

  if (PWRIndex > 0)
    B = WiFi.setTxPower (RawWiFiTX[PWRIndex - 1]);
  else if (CW != GSRWiFi.TransmitPower && GSRWiFi.TransmitPower != 0)
    B = WiFi.setTxPower (GSRWiFi.TransmitPower);
}
void
WatchyGSR::WiFiStationDisconnected (WiFiEvent_t event, WiFiEventInfo_t info)
{
  GSRWiFi.Last = millis () + 1000;
}
String
WatchyGSR::buildWiFiAPPage ()
{
  String S = LGSR.LangString (wifiIndexA, true, Options.LanguageID, 14, 14);
  String T;
  uint8_t I, J;

  for (I = 0; I < 10; I++)
    {
      T = wifiIndexB;
      T.replace ("$", String (char (65 + I)));
      T.replace ("#", String (char (48 + I)));
      T.replace ("?", APIDtoString (I));
      S += T;

      T = LGSR.LangString (wifiIndexC, true, Options.LanguageID, 15, 16);
      T.replace ("#", String (char (48 + I)));
      T.replace ("$", PASStoString (I));
      S += T;

      for (J = 0; J < 12; J++)
        {
          T = wifiIndexC1;
          T.replace ("#", String (J));
          T.replace ("%", (J == GSRWiFi.AP[I].TPWRIndex ? " selected" : ""));
          T.replace ("$", (J > 0 ? IDWiFiTX[J - 1]
                                 : "* " + LGSR.GetWebID (Options.LanguageID, 0)
                                       + " *"));
          S += T;
        }
      T = wifiIndexC2;
      S += (T + (I < 9 ? "</tr>" : ""));
    }
  return S + LGSR.LangString (wifiIndexD, true, Options.LanguageID, 6, 6);
}
void
WatchyGSR::parseWiFiPageArg (String ARG, String DATA)
{
  uint8_t I = String (ARG.charAt (2)).toInt ();
  String S = ARG.substring (0, 2);

  if (S == "AP")
    strncpy (&GSRWiFi.AP[I].APID[0], DATA.c_str (), 33);
  else if (S == "PA")
    strncpy (&GSRWiFi.AP[I].PASS[0], DATA.c_str (), 65);
  else if (S == "TX")
    GSRWiFi.AP[I].TPWRIndex = DATA.toInt ();
}
// Init Defaults after a reboot, setup all the variables here for defaults to
// avoid randomness.
void
WatchyGSR::setupDefaults ()
{
  Options.TwentyFour = false;
  Options.LightMode = true;
  Options.Feedback = true;
  Options.Border = false;
  Options.Lefty = false;
  Options.Swapped = false;
  Options.Turbo = 3;
  Options.SleepStyle = 0;
  Options.SleepMode = 3;
  Options.SleepStart = 23;
  Options.SleepEnd = 7;
  Options.MasterRepeats = 0; // 100%.
  Options.BedTimeOrientation = false;
  Options.WatchFaceStyle = 0;
  Options.LanguageID = 0;
  Options.NTPAuto = true;
  Options.ntpAtCharge = true;
  Options.Performance = 2;
  GSRWiFi.TransmitPower = RawWiFiTX[0];
  Steps.Hour = 6;
  Steps.Minutes = 0;
  NTPData.AutoSync = false;
  NTPData.SyncHour = 9;
  NTPData.SyncMins = 0;
  NTPData.SyncDays = 1;
  InsertDefaults ();
}
String
WatchyGSR::APIDtoString (uint8_t Index)
{
  String S = "";
  uint8_t I = 0;
  while (GSRWiFi.AP[Index].APID[I] != 0 && I < 32)
    {
      S += char (GSRWiFi.AP[Index].APID[I]);
      I++;
    }
  return S;
}
String
WatchyGSR::PASStoString (uint8_t Index)
{
  String S = "";
  uint8_t I = 0;
  while (GSRWiFi.AP[Index].PASS[I] != 0 && I < 63)
    {
      S += char (GSRWiFi.AP[Index].PASS[I]);
      I++;
    }
  return S;
}
String
WatchyGSR::getNTPfromWeb ()
{
  String S = "";
  uint8_t I = 0;
  while (NTPData.fromWeb[I] != 0 && I < 64)
    {
      S += char (NTPData.fromWeb[I]);
      I++;
    }
  return S;
}
void
WatchyGSR::setNTPfromWeb (String newNTP)
{
  memset (&NTPData.fromWeb[0], '\0', 64);
  strncpy (&NTPData.fromWeb[0], newNTP.c_str (), newNTP.length ());
}
String
WatchyGSR::overrideNTPfromWeb (String currentNTP)
{
  String _o = getNTPfromWeb ();
  return (_o > "" ? _o : currentNTP);
}
void
WatchyGSR::initZeros ()
{
  String S = "";
  uint8_t I, J;
  GuiMode = GSR_WATCHON;
  WatchyStatus = "";
  WatchTime.TimeZone = "";
  OP.init ();
  Menu.Style = GSR_MENU_INNORMAL;
  Menu.Item = 0;
  Menu.SubItem = 0;
  Menu.SubSubItem = 0;
  Battery.Level = -2;
  ActiveMode = false;
  OTATry = 0;
  OTAEnd = false;
  OTAUpdate = false;
  OTATimer = millis ();
  WatchyAPOn = false;
  DoHaptic = false;
  Steps.Reset = false;
  Steps.Yesterday = 0;
  Steps.Cached = 0;
  GSRWiFi.Requested = false;
  GSRWiFi.Working = false;
  GSRWiFi.Results = false;
  GSRWiFi.Index = 0;
  Updates.Full = true;
  Updates.Drawn = true;
  for (I = 0; I < 10; I++)
    {
      GSRWiFi.AP[I].TPWRIndex = 0;
      memset (&GSRWiFi.AP[I].APID[0], '\0', 33);
      memset (&GSRWiFi.AP[I].PASS[0], '\0', 65);
    }
  NTPData.Pause = 0;
  NTPData.Wait = 0;
  NTPData.TimeZone = false;
  NTPData.UpdateUTC = true;
  NTPData.State = 0;
  NTPData.TravelTemp = 0;
  memset (&NTPData.fromWeb[0], '\0', 64);
  TimerUp.SetAt = WatchTime.UTC_RAW;
  TimerUp.StopAt = TimerUp.SetAt;
  DefaultWatchStyles = true;
  WatchStyles.Count = 0;
  BasicWatchStyles = -1;
  for (I = 0; I < GSR_MaxStyles; I++)
    {
      WatchStyles.Options[I] = 0;
      WatchStyles.AddOn[I] = nullptr;
      BootAddOns.Inits[I] = nullptr;
    }
  BootAddOns.Count = 0;
  WeatherData.Metric = false;
  WeatherData.State = 0;
  WeatherData.Weather.ID = 0;
  WeatherData.Weather.Temperature.FeelsLike = 0;
  WeatherData.Weather.Temperature.Current = 0;
  WeatherData.Weather.Humidity = 0;
  WeatherData.Weather.WindSpeed = 0;
  WeatherData.Weather.WindDirection = 0;
  WeatherData.Weather.WindGust = 0;
  WeatherData.LastCall = 0;
  WeatherData.LastLon = NOLOC;
  WeatherData.LastLat = NOLOC;
  WeatherData.StaticLon = NOLOC;
  WeatherData.StaticLat = NOLOC;
  WeatherData.UseStaticPOS = false;
  WeatherData.Interval = 6; // 30 mins.
  GSRWebData.Ready = false;
  GSRWebData.Response = 0;
  GSRWebData.webURL = "";
  GSRWebData.secTimeout = 0;
  GSRWebData.Data = "";
  Options.Game = 255;
  Options.GameCount = 0;
}
// Settings Storage & Retrieval here.
String
WatchyGSR::GetSettings ()
{
  unsigned char I[2048];
  unsigned char O[2048];
  int K = 0;
  int J = 0;
  uint8_t X, Y, W;
  uint64_t D;
  uint16_t V;
  String S;
  size_t L;

  // Retrieve the settings from the current state into a base64 string.

  I[J] = 141;
  J++; // New Version.
  I[J] = (Steps.Hour & 31);
  J++;
  I[J] = (Steps.Minutes & 63) | (Options.NTPAuto ? 64 : 0);
  J++;
  K = Options.TwentyFour ? 1 : 0;
  K |= Options.LightMode ? 2 : 0;
  K |= Options.Feedback ? 4 : 0;
  K |= Options.Border ? 8 : 0;
  K |= Options.Lefty ? 16 : 0;
  K |= Options.Swapped ? 32 : 0;
  K |= Options.Orientated ? 64 : 0;
  K |= Options.GDEY0154D67 ? 128 : 0;
  I[J] = (K);
  J++;
  // Version 129, 137
  DWordtoUSC (SRTC.getDrift (false), I[J + 3], I[J + 2], I[J + 1], I[J]);
  J += 4; /* 129 */
  // Version 137
  DWordtoUSC (SRTC.getDrift (true), I[J + 3], I[J + 2], I[J + 1], I[J]);
  J += 4; /* 137 */

  W = ((Options.Performance & 15) << 4);
  I[J] = (Options.SleepStyle | W);
  J++;
  I[J] = (Options.SleepMode);
  J++;
  I[J] = (Options.SleepStart);
  J++;
  I[J] = (Options.SleepEnd);
  J++;
  // End Version 129.
  // Version 130.
  I[J] = (getTXOffset (GSRWiFi.TransmitPower));
  J++;
  K = (Options.BedTimeOrientation ? 1 : 0) | (WeatherData.Metric ? 2 : 0);
  I[J] = (K);
  J++;
  // End Version 130.
  // Version 131.
  I[J] = Options.WatchFaceStyle;
  J++;
  I[J] = Options.Game;
  J++; /* 136 */
  I[J] = Options.LanguageID;
  J++;
  // End Version 131.

  V = (Options.MasterRepeats << 5);
  I[J] = (Options.Turbo | V);
  J++;

  V = (Alarms.getTimerDownMaxTones () << 5);
  I[J] = ((Alarms.getTimerDownMaxHours ()) | V);
  J++;
  I[J] = (Alarms.getTimerDownMaxMinutes ());
  J++;
  I[J] = ((Alarms.getTimerDownMaxSeconds ())
          | (Alarms.getTimerDownRepeats () ? 128 : 0));
  J++;

  // Version 141 (8 alarms not 4)
  for (K = 0; K < 8; K++)
    {
      V = (Alarms.getRepeats (K) << 5);
      I[J] = (Alarms.getHour (K) | V);
      J++;
      I[J] = (Alarms.getMinutes (K));
      J++;
      V = (Alarms.getActive (K) & GSRALARM_NOTRIGGER);
      WordtoUSC (V, I[J + 1], I[J]);
      J += 2; //  I[J] = (V & 255); J++; I[J] = ((V >> 8) & 255); J++;
    }

  I[J] = (SRTC.isFastDrift (false) ? 1 : 0);
  J++;
  I[J] = (NTPData.AutoSync ? 2 : 0) + (WatchTime.ESPRTC ? 4 : 0)
         + (SRTC.isFastDrift (true) ? 8 : 0); /* The New Drift */
  V = ((int)(Battery.LowError * 100)) * 16;
  I[J] = I[J] + V;
  J++; /* New Battery LowError Level */
  I[J] = NTPData.SyncHour + (Options.ntpAtCharge ? 64 : 0);
  J++;
  I[J] = NTPData.SyncMins;
  J++;
  I[J] = NTPData.SyncDays;
  J++;
  // Version 140
  S = getNTPfromWeb ();
  X = S.length ();
  I[J] = X;
  J++; // Store length.
  if (X)
    {
      for (Y = 0; Y < X; Y++)
        {
          I[J] = S.charAt (Y);
          J++;
        }
    } // Store NTP from web.
  // End Version 140
  // Version 139
  S = String (WeatherData.StaticLat, 6);
  X = S.length ();
  for (Y = 0; Y < 13; Y++)
    {
      if (Y < X)
        I[J] = S.charAt (Y);
      else
        I[J] = 0;
      J++;
    }

  S = String (WeatherData.StaticLon, 6);
  X = S.length ();
  for (Y = 0; Y < 13; Y++)
    {
      if (Y < X)
        I[J] = S.charAt (Y);
      else
        I[J] = 0;
      J++;
    }
  // End Version 139
  I[J] = WeatherData.Interval;
  J++;

  for (X = 0; X < 10; X++)
    {
      S = APIDtoString (X);
      if (S > "")
        {
          I[J] = GSRWiFi.AP[X].TPWRIndex;
          J++; // Store the WiFi power per AP.
          W = S.length ();
          I[J] = W;
          J++;
          for (Y = 0; Y < W; Y++)
            {
              I[J] = S.charAt (Y);
              J++;
            }
          S = PASStoString (X);
          W = S.length ();
          I[J] = W;
          J++;
          for (Y = 0; Y < W; Y++)
            {
              I[J] = S.charAt (Y);
              J++;
            }
        }
    }
  I[J] = 0;
  J++;

  mbedtls_base64_encode (&O[0], 2047, &L, &I[0], J);

  O[L] = 0;
  S = reinterpret_cast<const char *> (O);
  return S;
}
void
WatchyGSR::StoreSettings (String FromUser)
{
  unsigned char O[2048], E[2048];
  int K = 0;
  int J = 0;
  uint16_t V;
  size_t L;
  bool Ok;
  int64_t DI, DX;
  bool FI, FX;
  uint8_t I, A, W, NewV; // For WiFi
  String S, T;

  DI = 0;
  DX = 0;
  J = FromUser.length ();
  if (J < 5)
    return;
  for (K = 0; K < J; K++)
    E[K] = FromUser.charAt (K);
  NewV = 0;

  mbedtls_base64_decode (&O[0], 2047, &L, &E[0], J);
  L--; // Take dead zero off end.

  J = 0;
  if (L > J && O[J] > 128)
    {
      NewV = O[J];
      J++;
    } // Detect NewVersion and go past.
  if (L > J)
    Steps.Hour = constrain (O[J] & 31, 0, 23);
  J++;
  if (L > J)
    Steps.Minutes = constrain (O[J] & 63, 0, 59);
  if (NewV > 140)
    Options.NTPAuto = (O[J] & 64) ? true : false;
  J++;
  if (L > J)
    {
      V = O[J];
      Options.TwentyFour = (V & 1) ? true : false;
      Options.LightMode = (V & 2) ? true : false;
      Options.Feedback = (V & 4) ? true : false;
#ifdef GxEPD2DarkBorder
      Options.Border = (V & 8) ? true : false;
#else
      Options.Border = false;
#endif
      Options.Lefty = (V & 16) ? true : false;
      Options.Swapped = (V & 32) ? true : false;
      Options.Orientated = (V & 64) ? true : false;
      if (NewV > 140)
        Options.GDEY0154D67 = (V & 128) ? true : false;
    }
  if (NewV > 128)
    {
      J++;
      if (L > J + 1)
        {
          if (NewV > 136)
            { // 137 introduces 32bit drifts for both external and internal.
              DX = USCtoDWord (O[J + 3], O[J + 2], O[J + 1], O[J]);
              J += 4;
              DI = USCtoDWord (O[J + 3], O[J + 2], O[J + 1], O[J]);
              J += 3; /* 137 */
            }
          else
            {
              if (NewV < 133)
                J++; // 129 had 16bit drift.
              if (NewV > 133 && NewV < 137)
                J++; // 133 introduced 24bit drift.
            }
        }
      J++;
      if (L > J)
        {
          V = ((O[J] & 240) >> 4);
          Options.Performance = constrain (V, 0, 2);
          Options.SleepStyle = constrain ((O[J] & 7), 0, 4);
#ifndef STABLEBMA_H_INCLUDED
          if (Options.SleepStyle)
            Options.SleepStyle = 2;
#endif
        }
      J++;
      if (L > J)
        Options.SleepMode = constrain (O[J], 1, 10);
      J++;
      if (L > J)
        Options.SleepStart = constrain (O[J], 0, 23);
      J++;
      if (L > J)
        Options.SleepEnd = constrain (O[J], 0, 23);
      if (NewV > 129)
        {
          J++;
          if (L > J)
            {
              V = constrain (O[J], 0, 10);
              GSRWiFi.TransmitPower = RawWiFiTX[V];
            }
          J++;
          if (L > J)
            {
              V = O[J];
              Options.BedTimeOrientation = (V & 1) ? true : false;
              WeatherData.Metric = (V & 2) ? true : false;
            }
        }
      if (NewV > 130)
        {
          J++;
          if (L > J)
            {
              V = constrain (O[J], 0, WatchStyles.Count - 1);
              Options.WatchFaceStyle = V;
            }
          if (NewV > 135)
            {
              J++;
              if (L > J)
                {
                  V = constrain (O[J], 0, WatchStyles.Count - 1);
                  Options.Game = V;
                  if (!(WatchStyles.Options[V] & GSR_GAM))
                    Options.Game = 255;
                }
            }
        }
      if (NewV > 131)
        {
          J++;
          if (L > J)
            {
              V = constrain (O[J], 0, LGSR.MaxLangID ());
              Options.LanguageID = V;
            }
        }
    }
  J++;
  if (L > J)
    {
      V = ((O[J] & 224) >> 5);
      Options.MasterRepeats = constrain (V, 0, 4);
      Options.Turbo = constrain ((O[J] & 31), 0, 10);
    }
  J++;
  if (L > J)
    {
      V = ((O[J] & 224) >> 5);
      Alarms.setTimerDownMaxTones (constrain (V, 0, 4));
      Alarms.setTimerDownMaxHours (constrain ((O[J] & 31), 0, 23));
    }
  J++;
  if (L > J)
    Alarms.setTimerDownMaxMinutes (constrain (O[J], 0, 59));
  if (NewV > 132)
    {
      J++;
      if (L > J)
        {
          Alarms.setTimerDownMaxSeconds (constrain ((O[J] & 127), 0, 59));
          Alarms.setTimerDownRepeats ((O[J] & 128) == 128);
        }
    }

  W = (NewV > 140 ? 8 : 4);

  for (K = 0; K < W; K++)
    {
      J++;
      if (L > J)
        {
          V = ((O[J] & 224) >> 5);
          Alarms.setEditMode (K, true);
          Alarms.setRepeats (K, constrain (V, 0, 4));
          Alarms.setHour (K, constrain ((O[J] & 31), 0, 23));
        }
      J++;
      if (L > J)
        Alarms.setMinutes (K, constrain (O[J], 0, 59));
      J++;
      if (L > J + 1)
        {
          Alarms.setActive (K,
                            (USCtoWord (O[J + 1], O[J]) & GSRALARM_NOTRIGGER));
          J++;
        }
      Alarms.setEditMode (K);
    }

  if (NewV > 133)
    {
      J++;
      if (L > J)
        {
          FX = ((O[J] & 1) == 1);
          J++;
          if (NewV > 136)
            FI = ((O[J] & 8) == 8);
          NTPData.AutoSync = ((O[J] & 2) == 2);
          WatchTime.ESPRTC = ((O[J] & 4) == 4);
          SRTC.useESP32 (WatchTime.ESPRTC);
          Battery.LowError = (O[J] & 240);
          Battery.LowError /= 1600;
        }
      J++;
      if (L > J)
        {
          NTPData.SyncHour = constrain ((O[J] & 31), 0, 23);
          if (NewV > 140)
            {
              Options.ntpAtCharge = (O[J] & 64);
            }
          J++;
        } // Test
      if (L > J)
        {
          NTPData.SyncMins = constrain (O[J], 0, 59);
          J++;
        } // Test
      if (NewV > 137 && L > J)
        {
          NTPData.SyncDays = O[J];
          J++;
        }
      if (!NTPData.SyncDays)
        NTPData.SyncDays = 1;
      if (NewV > 139)
        {
          S = "";
          W = O[J];
          J++; // Get Length.
          if (W)
            {
              for (A = 0; A < W; A++)
                {
                  S += String (char (O[J]));
                  J++;
                }
            } // Get NTP from web.
          setNTPfromWeb (S);
        }
      if (NewV > 134)
        {
          if (NewV > 138)
            {
              S = "";
              for (I = 0; I < 13; I++)
                {
                  if (O[J])
                    S += String (char (O[J]));
                  J++;
                }
              T = "";
              for (I = 0; I < 13; I++)
                {
                  if (O[J])
                    T += String (char (O[J]));
                  J++;
                }
              WeatherData.UseStaticPOS = (S.length () && T.length ());
              WeatherData.StaticLat = S.toDouble ();
              WeatherData.StaticLon = T.toDouble ();
            }
          else
            J += 32;
          WeatherData.Interval = constrain (O[J], 0, 144);
        }
    }

  S = "";
  for (I = 0; I < 10; I++)
    {
      memset (&GSRWiFi.AP[I].APID[0], '\0', 33);
      memset (&GSRWiFi.AP[I].PASS[0], '\0', 65);
    }

  J++;
  A = 0;
  while (L > J)
    {
      Ok = false;
      if (L > J)
        { // get APx
          if (NewV > 129)
            {
              GSRWiFi.AP[A].TPWRIndex = constrain (O[J], 0, 10);
              J++;
            }
          W = O[J];
          J++;
          S = "";
          if (L > J + (W - 1))
            { // Read APx.
              for (I = 0; L > J && I < W; I++)
                {
                  S += String (char (O[J]));
                  J++;
                }
              memcpy (&GSRWiFi.AP[A].APID, S.c_str (), strlen (S.c_str ()));
              Ok = true;
            }
          if (L > J)
            { // get APx
              W = O[J];
              J++;
              S = "";
              if (L > J + (W - 1))
                { // Read PAx.
                  for (I = 0; L > J && I < W; I++)
                    {
                      S += String (char (O[J]));
                      J++;
                    }
                  memcpy (&GSRWiFi.AP[A].PASS, S.c_str (), strlen (S.c_str ()));
                  Ok = true;
                }
            }
        }
      if (Ok)
        A++;
    }

  if (DX < 0)
    DX = 0 - DX;
  if (DI < 0)
    DI = 0 - DI;
  SRTC.setDrift (DI, FI, true);
  SRTC.setDrift (DX, FX, false);
}
uint16_t
WatchyGSR::USCtoWord (unsigned char High, unsigned char Low)
{
  unsigned char B[2];
  uint16_t V = 0;
  B[1] = High;
  B[0] = Low;
  memcpy (&V, &B[0], 2);
  return V;
}
void
WatchyGSR::WordtoUSC (uint16_t Word, unsigned char &High, unsigned char &Low)
{
  unsigned char B[2];
  memcpy (&B, &Word, 2);
  High = B[1];
  Low = B[0];
}
uint32_t
WatchyGSR::USCtoDWord (unsigned char HHigh, unsigned char HLow,
                       unsigned char LHigh, unsigned char LLow)
{
  unsigned char B[4];
  uint32_t V = 0;
  B[3] = HHigh;
  B[2] = HLow;
  B[1] = LHigh;
  B[0] = LLow;
  memcpy (&V, &B[0], 4);
  return V;
}
void
WatchyGSR::DWordtoUSC (uint32_t DWord, unsigned char &HHigh,
                       unsigned char &HLow, unsigned char &LHigh,
                       unsigned char &LLow)
{
  unsigned char B[4];
  memcpy (&B[0], &DWord, 4);
  HHigh = B[3];
  HLow = B[2];
  LHigh = B[1];
  LLow = B[0];
}
// NVS code.
void
WatchyGSR::RetrieveSettings ()
{
  if (OkNVS (GSR_GName))
    {
      String S = NVS.getString (GSR_GSettings);
      StoreSettings (S);
    }
  Options.NeedsSaving = false;
}
void
WatchyGSR::RecordSettings ()
{
  bool B = true;
  if (OkNVS (GSR_GName))
    {
      if (Options.Game != 255 && WatchStyles.AddOn[Options.Game] != nullptr)
        WatchStyles.AddOn[Options.Game]->SaveProgress ();
      B = NVS.setString (GSR_GSettings, GetSettings ());
      NVS.commit ();
    }
  Options.NeedsSaving = !B;
}
String
WatchyGSR::getBasicIndex ()
{
  String S = basicIndex;
  String T = "Unavailable";
  String c = OP.getCurrentPOSIX ();
  if (OkNVS (GSR_GName))
    {
      if (NVS.getString (GSR_GTZ, T))
        {
          if (T && c != OP.TZMISSING)
            {
              T += " [" + c + "]";
            }
          else
            {
              T += " [No POSIX]";
            }
        }
    }
  S.replace ("{%#%}", T);
  S.replace ("{%^%}", (OTA () ? basicOTA : ""));
  return LGSR.LangString (S, true, Options.LanguageID, 1, 27);
}
void
WatchyGSR::ResetOTA ()
{
  OTATimer = millis ();
  OTAFail = OTATimer + 600000;
}
void
WatchyGSR::EndOTA (uint8_t Seconds)
{
  OTAOff = millis () + Seconds * 1000;
}
bool
WatchyGSR::IsEndOTA ()
{
  return (OTAOff > 0 && millis () > OTAOff);
}
void
WatchyGSR::ResetEndOTA ()
{
  OTAOff = 0;
}
bool
WatchyGSR::OkNVS (String FaceName)
{
  String S = NVS.getString ("NoNVS");
  String R = ".#.";
  R.replace ("#", FaceName);
  return (S.indexOf (R) < 0);
}
void
WatchyGSR::SetNVS (String FaceName, bool Enabled)
{
  String S = NVS.getString ("NoNVS");
  String R = ".#.";
  bool B;
  R.replace ("#", FaceName);
  int I = S.indexOf (R);
  if (!Enabled)
    {
      if (I == 0)
        {
          S += R;
          B = NVS.setString ("NoNVS", S);
        }
      else if (I > -1)
        {
          S.replace (R, "");
          B = NVS.setString ("NoNVS", S);
        }
      if (B)
        NVS.commit ();
    }
}
void
WatchyGSR::NVSEmpty ()
{
  NVS.erase (GSR_GSettings);
  NVS.erase (GSR_GTZ);
  NVS.commit ();
}
// Turbo Mode!
void
WatchyGSR::SetTurbo ()
{
  LastButton = millis ();
  if (Battery.Read > getLowBattery (true))
    TurboTime = LastButton;
}
bool
WatchyGSR::InTurbo ()
{
  return (Options.Turbo > 0 && TurboTime != 0
          && millis () - TurboTime < (Options.Turbo * 1000));
}
bool
WatchyGSR::UpRight ()
{
#ifdef STABLEBMA_H_INCLUDED
  if (Options.Orientated || (WatchTime.BedTime && Options.BedTimeOrientation))
    return SBMA.IsUp ();
#endif
  return true; // Fake it til you make it.
}
bool
WatchyGSR::DarkWait ()
{
  bool B = AlarmsOn
           || (Options.SleepStyle > 0 && Darkness.Last != 0
               && (millis () - Darkness.Last) < (Options.SleepMode * 1000));
  if (Options.SleepStyle == 2)
    {
      if (!WatchTime.BedTime)
        return AlarmsOn;
      return B;
    }
  else if (GuiMode != GSR_MENUON && Options.SleepStyle > 0
           && Options.SleepStyle != 4)
    return B;
  return AlarmsOn;
}
bool
WatchyGSR::Showing ()
{ // Moved (GuiMode == GSR_MENUON) to Bool B = from 4701.
  bool B = (Updates.Full || Alarms.firing () || Alarms.soundsFiring ()
            || (GuiMode == GSR_MENUON));
  if (Options.SleepStyle > 0)
    {
      B |= (Darkness.Last > 0
            && (millis () - Darkness.Last) < (Options.SleepMode * 1000));
      if (Options.SleepStyle == 1)
        {
          return B; // Hide because it isn't checking the rest.
        }
      if (Options.SleepStyle == 2)
        {
          if (B)
            return B;
          if (WatchTime.BedTime)
            return (GuiMode == GSR_MENUON);
        }
      else if (Options.SleepStyle > 2 && Options.SleepStyle != 4)
        return B;
    }
  return true;
}
void
WatchyGSR::RefreshCPU ()
{
  RefreshCPU (0);
}
void
WatchyGSR::RefreshCPU (int Value)
{
  uint32_t C = 80;
  if (Battery.Read > getLowBattery (true))
    {
      if (Value == GSR_CPUMAX)
        CPUSet.Locked = true;
      if (Value == GSR_CPUDEF)
        CPUSet.Locked = false;
      if (!CPUSet.Locked && Options.Performance != 2 && Value != GSR_CPULOW)
        C = (InTurbo () || Value == GSR_CPUMID) ? 160 : 80;
      if (WatchyAPOn || OTAUpdate || GSRWiFi.Requests > 0 || CPUSet.Locked
          || (Options.Performance == 0 && Value != GSR_CPULOW))
        C = 240;
    }
  if (C != CPUSet.Freq)
    if (setCpuFrequencyMhz (C))
      ;
  CPUSet.Freq = C;
}
/* This is here so that all ESP.restarts store the step counter */
void
WatchyGSR::Reboot ()
{
  bool B;
  if (OkNVS (GSR_GName))
    {
      if (Options.NeedsSaving)
        RecordSettings ();
      B = NVS.setString (GSR_STP, String (CurrentStepCount ()));
      B = NVS.setString (GSR_YSTP, String (Steps.Yesterday));
      NVS.commit ();
      NVS.close ();
    }
  KeysStop ();
  if (GSRHandle != NULL)
    vTaskDelete (GSRHandle);
  if (Alarms.soundsFiring ())
    Alarms.endSounds ();
  vTaskDelay (100 / portTICK_PERIOD_MS);
  ESP.restart ();
}
bool
WatchyGSR::OTA ()
{
  esp_partition_iterator_t IT = esp_partition_find (
      ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
  if (IT != NULL)
    {
      const esp_partition_t *Part = esp_partition_get (IT);
      uint64_t Size = Part->size;
      esp_partition_iterator_release (IT);
      return (Size >= 0x1E0000);
    }
  return false;
}
float
WatchyGSR::rawBatteryVoltage (bool useSecond)
{
  float fout = 0.0f, ftmp = 0.0f, fcnt = 0.0f;
  uint8_t fl = 0;
  for (fl = 0; fl < 4; fl++)
    {
      if (WatchyGSR::readRawBatteryVoltage (ftmp, useSecond))
        {
          if ((useSecond && ftmp > 2.0 && ftmp < 4.5)
              || (!useSecond && ftmp > 200.0 && ftmp < 450.0))
            {
              fout += ftmp;
              fcnt += 1.0f;
            }
        }
    }
  if (fcnt)
    {
      return fout / fcnt;
    }
  return 0;
}
bool
WatchyGSR::readRawBatteryVoltage (float &rawBattery, bool useSecond)
{
  rawBattery = (analogReadMilliVolts (Battery.ADCPin)
                / (useSecond ? Battery.Divider2 : Battery.Divider));
  return (rawBattery > (useSecond ? 2.30f : 230.00f));
}
uint8_t
WatchyGSR::getTXOffset (wifi_power_t Current)
{
  uint8_t I;
  for (I = 0; I < 11; I++)
    {
      if (RawWiFiTX[I] == Current)
        return I;
    }
  return 0;
}
void
WatchyGSR::DisplayInit (bool ForceDark)
{
#ifdef GxEPD2DarkBorder
  display.epd2.setDarkBorder (Options.Border | ForceDark);
#endif
  if (Updates.Init)
    {
      WatchyGSR::hspi.begin (GSR_PIN_SCK, GSR_PIN_MISO, GSR_PIN_MOSI,
                             GSR_PIN_SS);
      display.epd2.selectSPI (hspi,
                              SPISettings (20000000, MSBFIRST, SPI_MODE0));
      display.init (0, Rebooted, 10,
                    true); // Force it here so it fixes the border.
      Updates.Init = false;
      Rebooted = false;
    }
  if (Updates.Full)
    {
      display.setFullWindow ();
      display.fillScreen (GxEPD_WHITE);
      display.display (false);
      display.fillScreen (GxEPD_BLACK);
      display.display (false);
      setDebounce ();
    }
}
void
WatchyGSR::DisplayWake (bool Tapped)
{
  Darkness.Woke = true;
  Darkness.Last = millis ();
  Darkness.Tilt = Darkness.Last;
  UpdateDisp = true;
  if (Tapped)
    Updates.Tapped = true;
}
void
WatchyGSR::DisplaySleep ()
{
  if (!Updates.Init)
    {
      Updates.Init = true;
      display.hibernate ();
    }
}
/* Code to correct for 3.8v dying batteries. */
float
WatchyGSR::getRealLowBattery (bool useCritical, bool radioLevel)
{
  if (useCritical)
    {
      return (Battery.LowError > 0.1 ? 3.8
                                     : Battery.MinLevel + Battery.LowError);
    }
  if (radioLevel)
    {
      return (Battery.LowError > 0.1 ? 3.9
                                     : Battery.LowLevel + Battery.LowError);
    }
  return (Battery.LowError > 0.1 ? 3.87 : Battery.LowLevel + Battery.LowError);
}
void
WatchyGSR::ProcessWeather ()
{
  String S, T, payload;
  struct tm sunTime;
  uint16_t wID;
  uint16_t *wSearch;
  JsonDocument root;
  if (UpdateDisp || GSRWiFi.Slow > 0)
    return;

  switch (WeatherData.State)
    {

    case 1:
      {
        if (!GetAskWiFi ())
          {
            if (rawBatteryVoltage (true) > getLowBatteryRadio ())
              {
                AskForWiFi ();
              }
            else if (WeatherData.Wait > 2)
              {
                WeatherData.Pause = 0;
                WeatherData.State = 99;
              }
            break;
          }
        if (WiFiInProgress () && !GSRWiFi.Results)
          {
            if (WeatherData.Wait > 2)
              {
                WeatherData.Pause = 0;
                WeatherData.State = 99;
              }
            break;
          }
        WeatherData.Wait = 0;
        WeatherData.Pause = 10;
        WeatherData.State++;
        break;
      }

    // Am I Connected?  If so, ask for GeoLocate to start..
    case 2:
      {
        if (WeatherData.UseStaticPOS)
          {
            WeatherData.LastLon = WeatherData.StaticLon;
            WeatherData.LastLat = WeatherData.StaticLat;
            WeatherData.State = 4;
            break;
          }
        if (WiFiStatus () != WL_CONNECTED)
          {
            if (currentWiFi () == WL_CONNECT_FAILED || WeatherData.Wait > 2)
              {
                WeatherData.Pause = 0;
                WeatherData.State = 99;
                break;
              }
            WeatherData.Pause = 5;
            if (WeatherData.Wait > 2)
              {
                WeatherData.Pause = 0;
                WeatherData.State = 99;
              }
            break;
          }
        if (!GetWebAvailable ())
          break; // Break out incase a URL request is going.

        setStatus ("GL");
        HTTP.setUserAgent (UserAgent);
        if (!AskForWeb ("http://ip-api.com/json/?fields=lat,lon"))
          {
            WeatherData.Pause = 0;
            WeatherData.State = 99;
          }
        else
          {
            WeatherData.State++;
            // Do the next part.
            WeatherData.Wait = 0;
            WeatherData.Pause = 1;
          }
        break;
      }

    case 3:
      {
        if (WiFiStatus () == WL_DISCONNECTED)
          {
            WeatherData.Pause = 0;
            WeatherData.State = 99;
            break;
          }
        if (GetWebReady ())
          {
            WeatherData.Wait = 0;
            WeatherData.Pause = 0;
            payload = GetWebData ();
            T = "";
            S = "";
            if (!deserializeJson (root, payload))
              {
                T = CleanString (root["lat"]);
                S = CleanString (root["lon"]);
              }
            if (S && T)
              {
                WeatherData.State++;
                WeatherData.LastLon = S.toDouble ();
                WeatherData.LastLat = T.toDouble ();
              }
            else
              WeatherData.State = 99;
          }
        else if (WeatherData.Wait > 0 || GetWebAvailable ()
                 || (GetWebResponse () > 0 && GetWebResponse () != 200))
          {
            WeatherData.Pause = 0;
            WeatherData.State = 99;
          }
        break;
      }

    // Am I Connected?  If so, ask for Weather now, since have GeoLocate.
    case 4:
      {
        if (WiFiStatus () != WL_CONNECTED)
          {
            if (currentWiFi () == WL_CONNECT_FAILED || WeatherData.Wait > 1)
              WeatherData.State = 99;
            break;
          }

        if (!GetWebAvailable ())
          break; // Break out incase a URL request is going.
        WeatherData.Pause = 1;
        setStatus ("WE");
        if (!AskForWeb (
                "http://api.open-meteo.com/v1/forecast?latitude="
                + String (WeatherData.LastLat, 6)
                + "&longitude=" + String (WeatherData.LastLon, 6)
                + "&current=temperature_2m,relative_humidity_2m,apparent_"
                  "temperature,weather_code,cloud_cover,surface_pressure,wind_"
                  "speed_10m,wind_direction_10m,wind_gusts_10m&hourly="
                  "visibility&daily=sunrise,sunset&timezone=UTC&forecast_days="
                  "1&forecast_hours=1"))
          {
            WeatherData.State = 99;
          }
        else
          {
            WeatherData.State++;
            // Do the next part.
            WeatherData.Wait = 0;
          }
        break;
      }

    case 5:
      {
        if (WiFiStatus () == WL_DISCONNECTED)
          {
            WeatherData.Pause = 0;
            WeatherData.State = 99;
            break;
          }
        if (GetWebReady ())
          {
            WeatherData.Wait = 0;
            WeatherData.Pause = 0;
            payload = GetWebData ();
            if (!deserializeJson (root, payload))
              {
                S = CleanString (root["error"]);
                if (S == "null")
                  {
                    S = CleanString (root["current"]["weather_code"]);
                    wID = S.toInt ();
                    wSearch
                        = std::find (std::begin (wIDs), std::end (wIDs), wID);
                    if (wSearch != std::end (wIDs))
                      {
                        WeatherData.Weather.ID = wID;
                        S = CleanString (root["current"]["temperature_2m"]);
                        WeatherData.Weather.Temperature.Current
                            = (S.toFloat ());
                        S = CleanString (
                            root["current"]["apparent_temperature"]);
                        WeatherData.Weather.Temperature.FeelsLike
                            = (S.toFloat ());
                        S = CleanString (
                            root["current"]["relative_humidity_2m"]);
                        WeatherData.Weather.Humidity = uint8_t (S.toInt ());
                        S = CleanString (root["current"]["cloud_cover"]);
                        WeatherData.Weather.Clouds = uint8_t (S.toInt ());
                        S = CleanString (root["current"]["surface_pressure"]);
                        WeatherData.Weather.Pressure = uint16_t (S.toInt ());
                        S = CleanString (root["current"]["wind_speed_10m"]);
                        WeatherData.Weather.WindSpeed = S.toFloat ();
                        S = CleanString (root["current"]["wind_gusts_10m"]);
                        WeatherData.Weather.WindGust = S.toFloat ();
                        S = CleanString (root["current"]["wind_direction_10m"]);
                        WeatherData.Weather.WindDirection = S.toInt ();
                        S = CleanString (root["daily"]["sunrise"]);
                        WeatherData.Weather.SunRise = getISO8601 (S);
                        S = CleanString (root["daily"]["sunset"]);
                        WeatherData.Weather.SunSet = getISO8601 (S);
                        S = CleanString (root["hourly"]["visibility"]);
                        WeatherData.Weather.Visibility = uint32_t (S.toInt ());
                        WeatherData.goneStale
                            = WatchTime.UTC_RAW; // Add the current value time
                                                 // onto this.
                        WeatherData.Ready = true;
                      }
                  }
              }
            WeatherData.Pause = 0;
            WeatherData.State = 99;
          }
        else if (WeatherData.Wait > 0 || GetWebAvailable ()
                 || (GetWebResponse () > 0 && GetWebResponse () != 200))
          {
            WeatherData.Pause = 0;
            WeatherData.State = 99;
          }
        break;
      }

    case 99:
      {
        WeatherData.Wait = 0;
        WeatherData.Pause = 0;
        WeatherData.State = 0;
        endWiFi ();
        WeatherData.LastCall = WatchTime.UTC_RAW - (WatchTime.UTC_RAW % 60)
                               + (WeatherData.Interval * 300); // 15 minutes.
        setStatus ("");
      }
    }
}
String
WatchyGSR::makeGeo (String inGeo, bool isLat)
{
  double _g = inGeo.toDouble ();
  String _s = String (_g, 6);
  if (inGeo.length () == 0)
    return inGeo;
  return (WatchyGSR::filterGeo (_g, isLat) ? _s : String (NOLOC));
}
void
WatchyGSR::GSRWebGet (void *parameter)
{
  vTaskDelay (10 / portTICK_PERIOD_MS);
  int tmp = 0;
  int len = 0;
  int cnt = 0;
  size_t size = 0;
  uint8_t buff[512] = { 0 };
  WiFiClient *netstream;
  GSRWebData.Response = 0;
  GSRWebData.Ready = false;
  GSRWebData.Data = "";
  bool Good = ((WatchStyles.Options[Options.WatchFaceStyle] & GSR_AFW)
               && WiFi.status () == WL_CONNECTED);
  bool Sent = false;
  uint16_t webTimeout = (golow (GSRWebData.secTimeout, 29) + 1) * 1000;
  unsigned long Stay = millis () + webTimeout;
  while (Good && millis () < Stay)
    {
      Good = ((WatchStyles.Options[Options.WatchFaceStyle] & GSR_AFW)
              && WiFi.status () == WL_CONNECTED);
      if (!Sent && Good)
        {
          vTaskDelay (10 / portTICK_PERIOD_MS); // 10ms pauses.
          Sent = true;
          HTTP.setConnectTimeout (webTimeout);
          Good = HTTP.begin (WiFiC, GSRWebData.webURL);
          vTaskDelay (10 / portTICK_PERIOD_MS); // 10ms pauses.
        }
      if (GSRWebData.Response == HTTP_CODE_OK)
        {
          len = HTTP.getSize ();
          netstream = HTTP.getStreamPtr ();
          while (HTTP.connected () && Good && (len > 0 || len == -1)
                 && millis () < Stay)
            {
              size = netstream->available ();
              if (size)
                {
                  cnt = netstream->readBytes (
                      buff, ((size > sizeof (buff)) ? sizeof (buff) : size));
                  if (cnt)
                    {
                      GSRWebData.Data
                          += String ((const char *)buff).substring (0, cnt);
                      Stay = millis () + webTimeout;
                    }
                  if (len == -1)
                    {
                      Good = !(size > 0 && !(size - cnt));
                    }
                  else if (len > 0)
                    {
                      len -= cnt;
                    }
                }
              vTaskDelay (10 / portTICK_PERIOD_MS); // 10ms pauses.
            }
          if (len == -1)
            {
              cnt = GSRWebData.Data.indexOf ("\x0a");
              if (cnt && cnt + 1 < GSRWebData.Data.length ())
                {
                  GSRWebData.Data = GSRWebData.Data.substring (cnt + 1);
                }
            }
          GSRWebData.Ready = GSRWebData.Data.length ();
          Good = false;
        }
      if (Good)
        {
          vTaskDelay (100 / portTICK_PERIOD_MS); // 100ms pauses.
          tmp = HTTP.GET ();
          if (tmp)
            {
              GSRWebData.Response = tmp;
              if (tmp != HTTP_CODE_OK)
                {
                  LastWebError = tmp;
                  Good = false;
                }
            }
        }
    }
  HTTP.end ();
  GSRHandle = NULL;
  vTaskDelete (NULL);
}
void
WatchyGSR::WatchFaceStart (uint8_t NewFace, bool NoEndWiFi)
{
  if (NoEndWiFi)
    {
      GSRWiFi.Requests = 0;
      GSRWiFi.Requested = false;
      GSRWiFi.Index = 0;
    }
  else
    {
      GSRWiFi.Requests = 0; /* Fix to stop WiFi from running */
      endWiFi ();
    }
  Options.WatchFaceStyle = NewFace;
  if (Options.WatchFaceStyle > WatchStyles.Count - 1)
    Options.WatchFaceStyle = 0;
  WatchStyles.Options[Options.WatchFaceStyle] = 0;
  if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr)
    initWatchFaceStyle ();
  else
    WatchStyles.AddOn[Options.WatchFaceStyle]->initWatchFaceStyle ();
  if (NoEndWiFi && !GetAskWiFi ())
    AskForWiFi ();
}
bool
WatchyGSR::ChangeGame (bool Up)
{
  byte I = (Options.Game == 255 ? 0 : Options.Game);
  byte C = 0;
  // if WiFiInProgress() return false;  // Avoid WiFi connection.
  while (C < WatchStyles.Count)
    {
      I = roller (I + (Up ? 1 : -1), 0, WatchStyles.Count - 1);
      C++;
      if (WatchStyles.Options[I] & GSR_GAM)
        C = WatchStyles.Count;
    }
  if (I != Options.Game && (WatchStyles.Options[I] & GSR_GAM))
    {
      if (Options.NeedsSaving)
        RecordSettings ();
      initGame (I);
      return true;
    }
  return false;
}
void
WatchyGSR::initGame (uint8_t GameID)
{
  if (GameID != 255 && (WatchStyles.Options[GameID] & GSR_GAM))
    {
      Options.GameStatus = false;
      // WatchFaceEnd();
      if (WatchStyles.AddOn[GameID] != nullptr)
        WatchStyles.AddOn[GameID]->InsertInitWatchStyle (GameID);
      Options.Game = GameID;
      Options.NeedsSaving = true;
    }
}
void
WatchyGSR::WatchFaceEnd ()
{
  if (GetAskWiFi ())
    {
      SetAskWiFi (false);
      endWiFi ();
    }
}
