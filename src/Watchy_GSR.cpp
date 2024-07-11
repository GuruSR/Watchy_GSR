#include "Watchy_GSR.h"

static const char UserAgent[] PROGMEM = "Watchy";
//WiFi statics.
static const wifi_power_t RawWiFiTX[11]= {WIFI_POWER_19_5dBm,WIFI_POWER_19dBm,WIFI_POWER_18_5dBm,WIFI_POWER_17dBm,WIFI_POWER_15dBm,WIFI_POWER_13dBm,WIFI_POWER_11dBm,WIFI_POWER_8_5dBm,WIFI_POWER_7dBm,WIFI_POWER_5dBm,WIFI_POWER_2dBm};
static const char *IDWiFiTX[11] = {"19.5dBm [Max]","19dBm","18.5dBm","17dBm","15dBm [Laptop]","13dBm","11dBm","8.5dBm [Medium]","7dBm","5dBm [10 meters]","2dBm [Low]"};

static const char WiFiTXT[] PROGMEM = "WiFi-";
char WiFiIDs[] PROGMEM = "ABCDEFGHIJ";

int AlarmVBs[] = {0x01FE, 0x00CC, 0x01B6, 0x014A};
const uint16_t Bits[10] = {1,2,4,8,16,32,64,128,256,512};
const float Reduce[5] = {1.0,0.8,0.6,0.4,0.2};

// Specific defines to this Watchy face.
#define GName "GSR"
#define GSettings "GSR-Options"
#define GTZ "GSR-TZ"
#define Bugh "GSR-Bugh"
#define STP "Steps"
#define YSTP "YSteps"
// Protected
RTC_DATA_ATTR struct GSRWireless final {
    bool Powered;            // Radio is on.
    bool Requested;          // Request WiFi.
    bool Working;            // Working on getting WiFi.
    bool Results;            // Results of WiFi, found an AP?
    bool Prepped;            // Used to denote the WiFi AP ID is being prepped.
    uint8_t Index;           // 10 = built-in, roll backwards to 0.
    uint8_t Requests;        // WiFi Connect requests.
    uint8_t Slow;            // Slows down initial connection to avoid brownouts.
    IPAddress LocalIP;       // Address gotten when connection happens.
    struct APInfo {
        char APID[33];
        char PASS[65];
        uint8_t TPWRIndex;
    } AP[10];                // Using APID to avoid internal confusion with SSID.
    unsigned long Last;      // Used with millis() to maintain sanity.
    bool Tried;              // Tried to connect at least once.
    wifi_power_t TransmitPower;
    wifi_event_id_t WiFiEventID;
} GSRWiFi;
RTC_DATA_ATTR struct CPUWork final {
    uint32_t Freq;
    bool     Locked;
} CPUSet;
RTC_DATA_ATTR struct Stepping final {
    uint8_t Hour;
    uint8_t Minutes;
    bool Reset;
    uint32_t Yesterday;
    uint32_t Cached;      // When a reboot happens,this is added.
    time_t Stored;
} Steps;
RTC_DATA_ATTR struct Optional final {
    bool TwentyFour;                  // If the face shows 24 hour or Am/Pm.
    bool LightMode;                   // Light/Dark mode.
    bool Feedback;                    // Haptic Feedback on buttons.
    bool Border;                      // True to set the border to black/white.
    bool Lefty;                       // Swaps the buttons to the other side.
    bool Swapped;                     // Menu and Back buttons swap ends (vertically).
    bool Orientated;                  // Set to false to not bother which way the buttons are.
    uint8_t Turbo;                    // 0-10 seconds.
    uint8_t MasterRepeats;            // Done for ease, will be in the Alarms menu.
    uint8_t SleepStyle;               // 0==Disabled, 1==Always, 2==Sleeping
    uint8_t SleepMode;                // Turns screen off (black, won't show any screen unless a button is pressed)
    uint8_t SleepStart;               // Hour when you go to bed.
    uint8_t SleepEnd;                 // Hour when you wake up.
    uint8_t Performance;              // Performance style, "Turbo", "Normal", "Battery Saving" 
    bool NeedsSaving;                 // NVS code to tell it things have been updated, so save to NVS.
    bool BedTimeOrientation;          // Make Buttons only work while Watch is in normal orientation.
    uint8_t WatchFaceStyle;           // Using the Style values from Defines_GSR.
    uint8_t LanguageID;               // The LanguageID.
    uint8_t Game;
    uint8_t GameCount;                // Counts the Games installed.
    bool GameStatus;                  // Game needs attention.
} Options;
RTC_DATA_ATTR struct DesignStyles final {
    uint8_t Count;
    uint8_t Options[GSR_MaxStyles];      // Contains all the Watchface selections.
    WatchyGSR *AddOn[GSR_MaxStyles];     // Holds the class to call when the Functions are present.
    char Style[32 * GSR_MaxStyles];
} WatchStyles;

RTC_DATA_ATTR struct WatchyBootAddOns final {
    uint8_t Count;                       // Used at boot time to record Addons to be called.
    WatchyGSR *Inits[GSR_MaxStyles];     // Holds the class to call for "Watch Faces" addition.
} BootAddOns;

RTC_DATA_ATTR struct MenuUse final {
    int8_t Style;           // GSR_MENU_INNORMAL or GSR_MENU_INOPTIONS
    int8_t Item;            // What Menu Item is being viewed.
    int8_t SubItem;         // Used for menus that have sub items, like alarms and Sync Time.
    int8_t SubSubItem;      // Used mostly in the alarm to offset choice.
} Menu;

RTC_DATA_ATTR int GuiMode;
RTC_DATA_ATTR bool VibeMode;          // Vibe Motor is On=True/Off=False, used for the Haptic and Alarms.
RTC_DATA_ATTR String WatchyStatus;    // Used for the indicator in the bottom left, so when it changes, it asks for a screen refresh, if not, it doesn't.
RTC_DATA_ATTR String WatchyOStatus;   // Original status kept when WiFi or BT until disconnected.
RTC_DATA_ATTR int BasicWatchStyles;
RTC_DATA_ATTR bool DefaultWatchStyles;  // States that the original 2 Watch Styles are to be added.
RTC_DATA_ATTR float HWVer;
RTC_DATA_ATTR volatile bool KeyIRQ; // Used to stop repeats.
RTC_DATA_ATTR uint8_t  GSR_MENU_PIN = 0;
RTC_DATA_ATTR uint8_t  GSR_BACK_PIN = 0;
RTC_DATA_ATTR uint8_t  GSR_UP_PIN = 0;
RTC_DATA_ATTR uint8_t  GSR_DOWN_PIN = 0;
RTC_DATA_ATTR uint8_t  GSR_PIN_ADC = 0;
RTC_DATA_ATTR uint8_t  GSR_PIN_STAT = 255;
RTC_DATA_ATTR uint8_t  GSR_PIN_SCL = 255;
RTC_DATA_ATTR uint8_t  GSR_PIN_SDA = 255;
RTC_DATA_ATTR uint8_t  GSR_PIN_ACC_INT1 = 0;
RTC_DATA_ATTR uint8_t  GSR_PIN_ACC_INT2 = 0;
RTC_DATA_ATTR uint8_t  GSR_PIN_VIB_PWM = 0;
RTC_DATA_ATTR uint8_t  GSR_PIN_USB_DET = 255;
RTC_DATA_ATTR uint8_t  GSR_PIN_SCK = 255;
RTC_DATA_ATTR uint8_t  GSR_PIN_MISO = 255;
RTC_DATA_ATTR uint8_t  GSR_PIN_MOSI = 255;
RTC_DATA_ATTR uint8_t  GSR_PIN_SS = 255;
RTC_DATA_ATTR uint8_t  GSR_PIN_RTC = 255;
RTC_DATA_ATTR uint64_t GSR_BTN_MASK;
RTC_DATA_ATTR uint64_t GSR_MENU_MASK;
RTC_DATA_ATTR uint64_t GSR_BACK_MASK;
RTC_DATA_ATTR uint64_t GSR_UP_MASK;
RTC_DATA_ATTR uint64_t GSR_DOWN_MASK;
RTC_DATA_ATTR uint64_t GSR_MASK_ACC_INT;
RTC_DATA_ATTR uint64_t GSR_ESP_WAKEUP = 0; // 1 for v1-2, 0 for v3.

RTC_DATA_ATTR struct Countdown final {
  bool Active;
  bool Repeats;
  bool Repeating;
  uint8_t Hours;  // Converted to work the same as the CountUp so seconds can be done.
  uint8_t Mins;
  uint8_t Secs;
  uint8_t MaxHours;
  uint8_t MaxMins;
  uint8_t MaxSecs;
  uint32_t MaxTime; // MaxHours*3600+MaxMins*60+MaxSecs.
  uint8_t Tone;       // 10 to start.
  uint8_t ToneLeft;   // 30 to start.
  uint8_t MaxTones;   // 0-4 (20-80%) tone duration reduction.
  time_t LastUTC;
  time_t StopAt;
} TimerDown;

RTC_DATA_ATTR struct CountUp final {
  bool Active;
  time_t SetAt;
  time_t StopAt;
} TimerUp;

RTC_DATA_ATTR struct BatteryUse final {
    float Read;             // div 100 from Last, since Last is much larger than normal.
    int8_t Direction;       // -1 for draining, 1 for charging.
    int8_t DarkDirection;   // Direction copy for Options.SleepMode.
    int8_t Level;           // Can go to +3 or -2 to determine direction of battery.
    int8_t State;           // 0=not visible, 1= showing chargeme, 2= showing reallychargeme, 3=showing charging.
    int8_t DarkState;       // Dark state of above.
    float MinLevel;         // Lowest level before the indicator comes on.
    float LowLevel;         // The battery is about to get too low for the RTC to function.
    float RadioLevel;       // WiFi and BT need this battery level to not brownout.
    time_t LastState;       // Used to marshall the battery indicator for 10 second intervals to avoid Active Mode abuse.
    float Start;            // Used at the start of Init to catch brownouts.
    uint32_t ADCPin;        // Here for static use.
    float Divider;          // Divider for battery.
    float Divider2;         // For formatted battery.
    bool Charge;            // V3 charge indicator.
    bool Ugh;               // 3rd party.
    int8_t CState;
    struct {
        time_t Stamp;       // Timestamp for the current Voltage below.
        float Voltage;      // Voltage of that current Stamp.
    } ReadFloat[5];
    int8_t FloatBottom;     // Used to indicate the oldest entry of the ReadFloat.
    int8_t FloatTop;        // Used to indicate the "last" entry of the ReadFloat (it is the one stored in last)
} Battery;

RTC_DATA_ATTR struct NTPUse final {
    uint8_t State;          // State = 0=Off, 1=Start WiFi, 2=Wait for WiFi, TZ, send NTP request, etc, Finish.  See function ProcessNTP();
    uint8_t Wait;           // Counts up to 3 minutes, then fails.
    uint8_t Pause;          // How many 50ms to pause for.
    unsigned long tWait;    // Used to Only wait X seconds for a response.
    time_t Last;            // Last time it worked.
    bool TimeZone;          // Update Timezone during ProcessNTP.
    bool UpdateUTC;         // Update UTC during ProcessNTP.
    bool AutoSync;
    uint8_t SyncHour;
    uint8_t SyncMins;
    uint8_t SyncDays;
    time_t TravelTemp;      // Value used in offset of time while changing the time to match, later used to determine drift more closely.
} NTPData;

RTC_DATA_ATTR struct GoneDark final {
    bool Went;
    unsigned long Last;
    bool Woke;
    unsigned long Tilt;     // Used to track Tilt, if upright count for 1 second.
} Darkness;                 // Whether or not the screen is darkened.

RTC_DATA_ATTR struct dispUpdate final {
    bool Full;
    bool Drawn;
    bool Init;
    bool Tapped;
    /* Display setup for menu item filtering */
    struct HighInfo {
        uint64_t Bits;  /* Binary values for each character */
        bool Active;
    } Highlighting;
    struct IndexInfo {
        uint8_t Offset;
        uint8_t Width;  /* In characters */
        bool Active;
    } Indexing;
    struct DrawingInfo {
        uint8_t Offset;
        uint8_t Width;
    } MenuData;
} Updates;

RTC_DATA_ATTR struct WeatherGSR final {
   bool Ready;             /* Weather read and ready */
   bool Metric;            /* SetWeatherScale is set here. */
   uint8_t State;          /* Like NTP State 0 = idle, 1 = asking for WiFi, 2 = asking for code, 3 = got code or error. */
   uint8_t Wait;           /* Counts up to 3 minutes, then fails. */
   uint8_t Pause;          /* How many 50ms to pause for. */
   struct Weatherinfo {
     uint16_t ID;          /* ID from open-medeo.com */
     struct TempsGSR {
       float FeelsLike;    /* These are set to the Kelvin, when the function call (SetWeatherScale(Metric is true/Imperial is false)) */
       float Current;      /* is done, the functions for these will return the proper values based on the selected scale. */
     } Temperature;
     uint8_t Humidity;     /* "humidity" from open-medeo.com */
     uint8_t Clouds;       /* "clouds" from open-medeo.com */
     uint16_t Pressure;    /* "pressure" from open-medeo.com */
     time_t SunRise;       /* "sunrise" from open-medeo.com */
     time_t SunSet;        /* "sunset" from open-medeo.com */
     uint32_t Visibility;  /* "visibility" from open-medeo.com */
     float WindSpeed;      /* "wind" "speed" from open-medeo.com */
     int WindDirection;    /* "wind" "deg" from open-medeo.com */
     float WindGust;       /* "wind" "gust" from open-medeo.com */
   } Weather;
   time_t LastCall;        /* Last call to open-medeo.com + Honor ## minute delay */
   double LastLat;         /* Last Latitude  ^^^ */
   double LastLon;         /* Last Longitude ^^^ */
   double StaticLat;       /* Static Latitude  from OTA Website */
   double StaticLon;       /* Static Longitude from OTA Website */
   bool UseStaticPOS;      /* Use the above 2 values */
   uint8_t Interval;       /* Interval between checks * 5 */
} WeatherData; /* COD 'error' means something is amiss. */

RTC_DATA_ATTR struct webGSR final {
  uint8_t Ready;          /* Data is ready for reading */
  int Response;           /* Code response from HTTP */
  String webURL;          /* NOT PERSISTENT! */
  uint8_t secTimeout;     /* URL Timeout IN SECONDS */
  String Data;            /* Data retrieved from HTTP Get NOT PERSISTENT! */
} GSRWebData;

GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> WatchyGSR::display(GxEPD2_154_D67(WatchyGSR::getDispCS(), WatchyGSR::getDispDC(), WatchyGSR::getDispRES(), WatchyGSR::getDispBSY()));
RTC_DATA_ATTR SmallRTC WatchyGSR::SRTC;
RTC_DATA_ATTR SmallNTP WatchyGSR::SNTP;
RTC_DATA_ATTR Designing Design;
RTC_DATA_ATTR TimeData WatchTime;
#ifdef STABLEBMA_H_INCLUDED
RTC_DATA_ATTR StableBMA SBMA;
#endif
RTC_DATA_ATTR LocaleGSR LGSR;
RTC_DATA_ATTR WatchyGSR *MonitorTo;
SPIClass WatchyGSR::hspi(HSPI);

volatile uint8_t Button;
volatile uint8_t Missed;    // Button not in menu, not used, so can be used by override.
volatile bool KeysCheckOn;  // Used to validate the loop during use.

//RTC_DATA_ATTR alignas(8) struct AlarmID {
RTC_DATA_ATTR     uint8_t   Alarms_Hour[4];
RTC_DATA_ATTR     uint8_t   Alarms_Minutes[4];
RTC_DATA_ATTR     uint16_t  Alarms_Active[4];         // Bits 0-6 (days of week), 7 Repeat, 8 Active, 9 Tripped (meaning if it isn't repeating, don't fire).
RTC_DATA_ATTR     uint8_t   Alarms_Times[4];          // Set to 10 to start, ends when zero.
RTC_DATA_ATTR     uint8_t   Alarms_Playing[4];        // Means the alarm tripped and it is to be played (goes to 0 when it finishes).
RTC_DATA_ATTR     uint8_t   Alarms_Repeats[4];        // 0-4 (20-80%) reduction in repetitions.
//} Alarms[4];

WiFiClient WiFiC;   // Tz
HTTPClient HTTP;    // Tz
Olson2POSIX OP;     // Tz code.
WebServer server(80);
unsigned long OTATimer;
bool WatchyAPOn;   // States Watchy's AP is on for connection.  Puts in Active Mode until back happens.
bool ActiveMode;   // Moved so it can be checked.
bool Sensitive;    // Loop code is sensitive, like OTAUpdate, TimeTest
bool OTAUpdate;    // Internet based OTA Update.
bool AlarmReset;   // Moved out here to work with Active Mode.
bool Alarming;     // Means an alarm has triggered.
bool OTAEnd;       // Means somewhere, it wants this to end, so end it.
bool OTAAsked;     // OTA Website/Update asked for WiFi (this gets rid of the loop).
int OTATry;        // Tries to connect to WiFi.
bool DoHaptic;     // Want it to happen after screen update.
bool UpdateDisp;   // Display needs to be updated.
bool AlarmsOn;     // Moved for CPU.
bool Rebooted;     // Used in DisplayInit to force a full initial on power up.
time_t TurboTime;  // Moved here for less work.
bool AllowHaptic;  // For the Task to tell the main thread it can't use the Haptic feedback due to alarm/timer.
uint8_t HapticMS;  // Holds how long it runs for.
bool WfNM;         // Wait for New Minute.
bool setPinModes;  // In the attempt to not break the V3...
unsigned long LastButton, OTAFail, OTAOff, LastHelp;
RTC_DATA_ATTR TaskHandle_t SoundHandle;
RTC_DATA_ATTR BaseType_t SoundRet;
RTC_DATA_ATTR TaskHandle_t GSRHandle;
RTC_DATA_ATTR BaseType_t GSRRet;
RTC_DATA_ATTR TaskHandle_t KeysHandle;
RTC_DATA_ATTR BaseType_t KeysRet;
RTC_DATA_ATTR bool Started = false, SoundStart = false;
RTC_DATA_ATTR GSRCPUInfo Watchy_Chip_Info;
RTC_DATA_ATTR volatile int LastWebError;

WatchyGSR::WatchyGSR(){ if (!Started) { Started = true; initZeros(); } }  //constructor

// Init Defaults after a reboot, setup all the variables here for defaults to avoid randomness.
void WatchyGSR::setupDefaults(){
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
    Options.MasterRepeats = 0;  // 100%.
    Options.BedTimeOrientation = false;
    Options.WatchFaceStyle = 0;
    Options.LanguageID = 0;
    GSRWiFi.TransmitPower = RawWiFiTX[0];
    Steps.Hour = 6;
    Steps.Minutes = 0;
    NTPData.AutoSync = false;
    NTPData.SyncHour = 9;
    NTPData.SyncMins = 0;
    NTPData.SyncDays = 1;
    InsertDefaults();
}

void WatchyGSR::init(String datetime){
    uint64_t wakeupBit;
    bool B, Up, OTAWiFi;
    unsigned long Since, APLoop;
    uint8_t I;
    String S;
    startSetup();
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause(); //get wake up reason
    esp_reset_reason_t reset_reason = esp_reset_reason();
    wakeupBit = esp_sleep_get_ext1_wakeup_status();
    NVS.begin();
    ResetEndOTA();
    WfNM = false;
    Alarming = false;
    OTAUpdate = false;
    OTAEnd = false;
    OTAAsked = false;
    WatchyAPOn = false;
    AlarmReset = false;
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
    TurboTime = 0;
    AllowHaptic = true;
    SoundHandle = NULL;
    GSRHandle = NULL;
    KeysHandle = NULL;
    KeysCheckOn = false;
    setPinModes = false;
    CPUSet.Freq=getCpuFrequencyMhz();

    switch (wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_EXT0: //RTC Alarm & V3 Charge indicator
        case ESP_SLEEP_WAKEUP_TIMER: // Internal RTC Alarm
            RefreshCPU(GSR_CPUDEF);
            BrownOutDetect();
#ifndef SMALL_RTC3_H
            SRTC.clearAlarm();
#endif
            UpdateUTC();
            UpdateClock();
            if (WatchTime.Local.Second == 0) WatchTime.NewMinute = true;
            else WfNM = (WatchTime.Local.Second > 58);
            detectBattery();
            CalculateTones();
            AlarmsOn = Alarming;
            UpdateDisp=Showing();
            break;
        case ESP_SLEEP_WAKEUP_EXT1: //button Press
            RefreshCPU(GSR_CPUDEF);
            BrownOutDetect();
            UpdateUTC();
            espPinModes();
            Button = getButtonMaskToID(wakeupBit);
            B = (Options.SleepStyle != 4);
            if (B) UpdateDisp |= Showing();
            if (Darkness.Went && (UpRight() || HWVer == 3.0f)){
                if (Button == 9 && Options.SleepStyle > 1 && B){  // Accelerometer caused this.
                    if (Options.SleepMode == 0) Options.SleepMode = 2;  // Do this to avoid someone accidentally not setting this before usage.
                    DisplayWake(true); // Update Screen to new state.
                }else if (Button == 10 && !WatchTime.BedTime){  // Wrist.
                    DisplayWake(); // Do this anyways, always.
                }else if (Button) {
                    DisplayWake(); LastButton = millis(); Button = 0; // Update Screen to new state. if (Options.SleepStyle > 1 && B) 
                }
            }
            if (Darkness.Woke || Button) UpdateClock();  // Make sure this is done when buttons are pressed for a wakeup.
            break;
        default: //reset
            MonitorTo = this;
            SetupESPValues();
            BrownOutDetect();
            WatchStyles.Count = 0;
            BasicWatchStyles = -1;
            SRTC.init();
            Battery.MinLevel = SRTC.getRTCBattery();
            Battery.LowLevel = SRTC.getRTCBattery(true);
            Battery.RadioLevel = Battery.LowLevel + 0.1;
            HWVer = SRTC.getWatchyHWVer();
            getPins(HWVer);
            setupDefaults();
            _bmaConfig();
            Rebooted=true;
            UpdateUTC();
            Battery.ADCPin = SRTC.getADCPin();
            Battery.LastState = 0;
            Battery.FloatBottom = 0;
            Battery.FloatTop = 4;
            if (OkNVS(GName)){ B = NVS.getString(Bugh,S); Battery.Ugh = (S > ""); }
            Battery.Divider = (HWVer == 3.0f ? 7.826f : 5.0f); // 7.34694 vs 7.826
            Battery.Divider2 = (HWVer == 3.0f ? 782.608f : 500.0f);  //734.693 vs 782.608
            Battery.State = 0;
            Battery.DarkState = 0;
            Battery.Direction = -1;
            Battery.DarkDirection = 0;
            for (I = 0; I < 5; I++){
              Battery.ReadFloat[I].Voltage = rawBatteryVoltage();
              Battery.ReadFloat[I].Stamp = WatchTime.UTC_RAW - WatchTime.UTC.Second;
            }
            if (DefaultWatchStyles){
                I = AddWatchStyle("{%0%}"); // Classic GSR
                BasicWatchStyles = I;
            }
            InsertAddWatchStyles();
            if (WatchStyles.Count == 0){
                I = AddWatchStyle("{%0%}"); // Classic GSR
                BasicWatchStyles = I;
                DefaultWatchStyles = true;
            }
            for (I = 0; I < BootAddOns.Count; I++) { if (BootAddOns.Inits[I] != nullptr) BootAddOns.Inits[I]->RegisterWatchFaces(); }
            if (OkNVS(GName)) B = NVS.getString(GTZ,S);
            OP.setCurrentPOSIX(S);
            RetrieveSettings();
            if (OkNVS(GName)) {
                S=""; B = NVS.getString(STP,S); if (S.toInt()) Steps.Cached = S.toInt();
                S=""; B = NVS.getString(YSTP,S); if (S.toInt()) Steps.Yesterday = S.toInt();
                NVS.erase(STP);
                NVS.erase(YSTP);
                NVS.commit();
            }
            RefreshCPU(GSR_CPUDEF);
            UpdateClock();
            InsertPost();
            Updates.Full=true;
#ifndef GxEPD2DarkBorder
            Options.Border=false;
#endif
            if (Options.GameCount > 0){
                if (Options.Game == 255) B = ChangeGame();
                if (Options.Game != 255) initGame(Options.Game);
            }
            WatchFaceStart(Options.WatchFaceStyle);
            StartNTP(true);
            DisplayWake(true);
            DoHaptic = true;
#ifndef STABLEBMA_H_INCLUDED
            Menu.Item = GSR_MENU_ALARMS;
#else
            Menu.Item = GSR_MENU_STEPS;
#endif
            break;
    }

    espPinModes();

//HapticMS = 5; SoundBegin();

    B = true; Up = false;
    if (Darkness.Went && GuiMode != GSR_MENUON)
    {
        B = (Battery.DarkState != Battery.State || Battery.DarkDirection != Battery.Direction);
        if (Options.SleepStyle == 4) B |= Updates.Tapped; else B |= (Darkness.Woke || Button);
        //if (!B && Options.Game != 255 && WatchTime.NewMinute) { if (WatchStyles.AddOn[Options.Game] != nullptr) WatchStyles.AddOn[Options.Game]->InsertOnMinute(); }
    }
    B |= Alarming;

    if (B || Updates.Full || WatchTime.NewMinute || WfNM){

        // Sometimes BMA crashes - simply try to reinitialize bma...

#ifdef STABLEBMA_H_INCLUDED
        if (SBMA.getErrorCode() != 0) {
            SBMA.shutDown();
            SBMA.wakeUp();
            SBMA.softReset();
            _bmaConfig();
        }
#endif

        ActiveMode = true;
        RefreshCPU((Darkness.Went && GuiMode != GSR_MENUON && WatchTime.BedTime) ? GSR_CPULOW : 0); // Use low CPU usage at night.
        Battery.Start = golow(getBatteryVoltage(),4.22);
        KeysStart();
        while(ActiveMode || UpdateDisp || WfNM){
              Since=millis();
              ManageTime();   // Handle Time method.
#ifdef STABLEBMA_H_INCLUDED
              Up=SBMA.IsUp(); //SBMA.IsUp();
#endif
              B = DarkWait();
              if (GSRWiFi.Slow > 0 && !inBrownOut()) GSRWiFi.Slow--;
/* Pre-Tilt */

              if (!B && !Up) GoDark();

/* Tilt */
              // Wrist Tilt delay, keep screen on during this until you put your wrist down.
              if ((Options.SleepStyle == 1 || (Options.SleepStyle > 2 && Options.SleepStyle != 4)) && !WatchTime.BedTime){
                if (Darkness.Went && Up && !Darkness.Woke){ // Do this when the wrist is UP.
                  if (Darkness.Tilt == 0) Darkness.Tilt = millis();
                  else if (millis() - Darkness.Tilt > 999) { Darkness.Last = millis(); Darkness.Woke = true; UpdateDisp |= Showing(); }
                }
                if (!Up) Darkness.Tilt = 0;
                else { if (B) Darkness.Last = millis(); if (Darkness.Tilt != 0 && millis() - Darkness.Tilt > 999 && Darkness.Woke) { Darkness.Last = millis(); } }
              }

              // Here, check for button presses and respond, done here to avoid turbo button presses.
              if (Button) { handleButtonPress(Button); Button = 0; }

/* ALARMS */
              CalculateTones();

/* Insert */

              if (currentWiFi() == WL_CONNECTED && NTPData.State == 0 && WeatherData.State == 0 && !OTAUpdate && !WatchyAPOn && !UpdateDisp && GSRWiFi.Slow == 0 && !inBrownOut()) { if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr) InsertWiFi(); else WatchStyles.AddOn[Options.WatchFaceStyle]->InsertWiFi(); }

/* NTP */

              if (Battery.Read > Battery.RadioLevel && WatchTime.UTC_RAW >= WeatherData.LastCall && !WatchTime.BedTime && WeatherData.Interval > 0 ) StartWeather();
              if (NTPData.State && WeatherData.State < 2 && !WatchyAPOn && !OTAUpdate){
                if (GSRWiFi.Slow == 0 && !inBrownOut()) { if (NTPData.Pause == 0) ProcessNTP(); else NTPData.Pause--; }
                if (WatchTime.NewMinute){
                  NTPData.Wait++;
                  UpdateDisp |= Showing();
                }
              }

/* Weather */

              if (WeatherData.State && NTPData.State < 2 && !WatchyAPOn && !OTAUpdate && GSRWiFi.Slow == 0 && !inBrownOut()){

                if (WeatherData.Pause == 0) ProcessWeather(); else WeatherData.Pause--;
                if (WatchTime.NewMinute){
                  WeatherData.Wait++;
                  UpdateDisp |= Showing();
                }
              }

              if (GuiMode == GSR_WATCHON && GetMenuOverride()) {
                if (Missed == 0) Missed = WatchyGSR::getButtonPins();
                if ((Missed != 1 && Button) || (Missed == 1 && LastHelp == 0)) { LastHelp = millis(); SetTurbo(); }
                else if (Missed == 1 && millis() - LastHelp > 9999) { ShowDefaultMenu(); Missed = 0; }
              }
              if (Missed){
                if (GuiMode == GSR_WATCHON) { if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr) { if (InsertHandlePressed(Missed, DoHaptic, UpdateDisp)) SetTurbo(); } else if (WatchStyles.AddOn[Options.WatchFaceStyle]->InsertHandlePressed(Missed, DoHaptic, UpdateDisp)) SetTurbo(); }
                else if (GuiMode == GSR_GAMEON && Options.Game != 255) { if (WatchStyles.AddOn[Options.Game]->InsertHandlePressed(Missed, DoHaptic, UpdateDisp)) SetTurbo(); }
                Missed = 0; Button = 0;
              }

              // Here, make sure the clock updates on-screen.
              if (WatchTime.NewMinute) UpdateDisp |= Showing();

/* OTA */

              // OTAEnd code.
              if (OTAEnd){
                if (Menu.Item == GSR_MENU_OTAU)      { ArduinoOTA.end(); Menu.SubItem = 0; }
                else if (Menu.Item == GSR_MENU_OTAM) { server.stop(); Menu.SubItem = 0; }
                if (WatchyAPOn) server.stop();
                VibeTo(false);
                OTAEnd=false;
                OTAAsked=false;
                endWiFi();
                if (Menu.Item == GSR_MENU_WIFI && Menu.SubItem > 0) Menu.SubItem = 0;
                if (OTAUpdate) Menu.SubItem=0;
                UpdateUTC();
                UpdateClock();
                OTAUpdate=false;
                setStatus("");
                UpdateDisp |= Showing() | WatchyAPOn;
                WatchyAPOn = false;
                if (Options.NeedsSaving) RecordSettings();
                Missed = 0; Button = 0;
              }

/* OTA */

              if (WatchyAPOn && IsEndOTA()) OTAEnd = true;  // Fail if holding back for 10 seconds OR 600 seconds has passed.
              if (OTAUpdate && !UpdateDisp && GSRWiFi.Slow == 0 && !inBrownOut()){
                switch (Menu.SubItem){
                  case 1: // Wait for WiFi to connect or fail.
                    if (!GetAskWiFi() && !OTAAsked) { AskForWiFi(); OTAAsked = true; }
                    else if (WiFiStatus() != WL_CONNECTED && currentWiFi() != WL_CONNECT_FAILED) OTATimer = millis();
                    else if (HasIPAddress()){
                      setStatus(WiFiIndicator(GSRWiFi.Index ? GSRWiFi.Index : 24));
                      Menu.SubItem++;
                      UpdateDisp |= Showing();
                    }else if (!WiFiInProgress()) OTAEnd=true;
                    break;
                  case 2: // Setup Arduino OTA and wait for it to either finish or fail by way of back button held for too long OR 2 minute with no upload.
                    if (Menu.Item == GSR_MENU_OTAU){
                    ArduinoOTA.setHostname(WiFi_AP_SSID);
                    ArduinoOTA
                    .onStart([]() {
                      String Type;
                      if (ArduinoOTA.getCommand() == U_FLASH)
                      Type = "sketch";
                      else // U_SPIFFS
                      Type = "filesystem";
                      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                    })
                    .onEnd([]() {
                      OTAEnd = true;
                    })
                    .onProgress([](unsigned int progress, unsigned int total) {
                      OTATimer=millis();
                    })
                    .onError([](ota_error_t error) {
                      OTAEnd=true;
                    });
                    RefreshCPU(GSR_CPUMAX);
                    ArduinoOTA.begin();
                    }else if (Menu.Item == GSR_MENU_OTAM) WatchyGSR::StartWeb();
                    Menu.SubItem++;
                    //showWatchFace();
                    break;
                  case 3: // Monitor back button and turn WiFi off if it happens, or if duration is longer than 2 minutes.
                    if (WiFiStatus() == WL_DISCONNECTED || OTAEnd) OTAEnd = true;
                    else if (Menu.Item == GSR_MENU_OTAU)      ArduinoOTA.handle();
                    else if (Menu.Item == GSR_MENU_OTAM)      server.handleClient();
                    if (WatchyGSR::getButtonPins() != 2) OTATimer = millis(); // Not pressing "BACK".
                    if (millis() - OTATimer > 10000 || millis() > OTAFail || IsEndOTA()) OTAEnd = true;  // Fail if holding back for 10 seconds OR 600 seconds has passed.
                }
              }
              /* removed: (Alarming || !inBrownOut()) && */
              if (UpdateDisp) showWatchFace(); //partial updates on tick

/* SOUNDS */

              if (SoundStart) SoundBegin(); /* removed: (Alarming || !inBrownOut()) &&  */

/* NON-SENSITIVE */
              // Don't do anything time sensitive while in OTA Update.

              if (!Sensitive){

/* Battery Detection */

                detectBattery();

/* WATCHY AP */

              if (GSRWiFi.Requests == 0 && GSRWiFi.Slow == 0 && !inBrownOut() && WatchyAPOn && !OTAUpdate){
                switch (Menu.SubItem){
                  case 0: // The AP is now off.
                    break;
                  case 1: // Turn on AP.
                    if (WiFi.getMode() != WIFI_AP || (millis() - OTATimer > 4000 && OTATry < 3)){
                      OTATimer=millis();
                      OTATry++;
                      WiFi.setHostname(WiFi_AP_SSID);
                      RefreshCPU(GSR_CPUMAX);
                      OTAEnd |= (!WiFi.softAP(WiFi_AP_SSID, WiFi_AP_PSWD, 1, WiFi_AP_HIDE, WiFi_AP_MAXC));
                      if (!OTAEnd) { UpdateWiFiPower(); GSRWiFi.Slow = 2; }
                    }else if (WiFi.getMode() == WIFI_AP){
                      WatchyGSR::StartWeb();
                      Menu.SubItem++;
                      setStatus(String(WiFiTXT) + "AP");
                      APLoop=millis();
                      GSRWiFi.Slow = 2;
                    }else Menu.SubItem = 0; // Fail, something is amiss.
                    break;
                  case 5:  // SoftAP waiting to go off.
                    break;
                  default: // 2 to 5 is here.
                    if (Menu.SubItem > 1){
                      if(WiFi.getMode() == WIFI_STA){
                        Menu.SubItem = 0;
                        break;
                      }
                      server.handleClient();
                      /*if (Server.handleRequests()){
                        Menu.SubItem = 0;
                        break;
                      }*/
                      if (millis() - APLoop > 8000){
                        Menu.SubItem = roller(Menu.SubItem + 1, 2, 4);
                        UpdateDisp |= Showing();
                        APLoop=millis();
                      }
                    }
                }
              }

              if (Darkness.Went && Options.NeedsSaving) RecordSettings();
              if (HWVer = 3.0f) CheckButtons();

              if (!Updates.Init && !SoundActive() && !(InTurbo() || B)) DisplaySleep();
            }
/* WIFI */

            if (!UpdateDisp && GSRWiFi.Slow == 0 && !inBrownOut()) processWiFiRequest(); // Process any WiFi requests.

            SRTC.pauseDrift(TimerDown.Active || TimerUp.Active);
            Sensitive = (OTAUpdate && Menu.SubItem == 3);
            AlarmsOn = (Alarming || SoundActive() || !GetWebAvailable() || TimerAbuse());
            ActiveMode = (InTurbo() || B || NTPData.State || WeatherData.State || AlarmsOn || WatchyAPOn || OTAUpdate || GSRWiFi.Requested || GSRWiFi.Requests || OTAEnd || Button); /* (B && Up) */
            if (Options.Game != 255) { if (WatchStyles.AddOn[Options.Game] != nullptr) ActiveMode |= WatchStyles.AddOn[Options.Game]->InsertNeedAwake(!ActiveMode); }
            if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr) ActiveMode |= InsertNeedAwake(!ActiveMode); else ActiveMode |= WatchStyles.AddOn[Options.WatchFaceStyle]->InsertNeedAwake(!ActiveMode);
            WatchTime.NewMinute = false;
            RefreshCPU(GSR_CPUDEF);
            Since=50-(millis()-Since);
            if (Since <= 50 && Since > 0) delay(Since);
        }
        KeysCheckOn = false;
    }
    BrownOutDetect(true);
    KeysStop();
    deepSleep();
}

void WatchyGSR::StartWeb(){
    /*return index page which is stored in basicIndex */
    server.on("/", HTTP_GET, [=]() {
      server.sendHeader("Connection", "close");
      String S = basicIndex;
      S.replace("{%^%}",(OTA() ? basicOTA : ""));
      S = LGSR.LangString(S,true,Options.LanguageID,1,20);
      server.send(200, "text/html", S);
      ResetOTA();
    });
    server.on("/settings", HTTP_GET, [=]() {
      String S = LGSR.LangString(settingsIndex,true,Options.LanguageID,5,6);
      S.replace("{??}",GetSettings());
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", S);
      ResetOTA();
    });
    server.on("/wifi", HTTP_GET, [=]() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", buildWiFiAPPage());
      ResetOTA();
    });
    server.on("/update", HTTP_GET, [=]() {
      if (OTA()){
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", LGSR.LangString(updateIndex,true,Options.LanguageID,8,13));
        ResetOTA();
      }
    });
    server.on("/weather", HTTP_GET, [=]() {
      String S = LGSR.LangString(weatherIndex,true,Options.LanguageID,6,23);
      String T = (WeatherData.UseStaticPOS ? makeGeo(String(WeatherData.StaticLat,6),true) : "");
      S.replace("{%1%}",T);
      T = (WeatherData.UseStaticPOS ? makeGeo(String(WeatherData.StaticLon,6),false) : "");
      S.replace("{%2%}",T);
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", S);
      ResetOTA();
    });
    server.on("/", HTTP_POST, [=]() {
      server.sendHeader("Connection", "close");
      String S = basicIndex;
      S.replace("{%^%}",(OTA() ? basicOTA : ""));
      S = LGSR.LangString(S,true,Options.LanguageID,1,20);
      server.send(200, "text/html", S);
      ResetOTA();
    });
    server.on("/exit", HTTP_GET, [=]() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", LGSR.LangString(basicIndexDone,true,Options.LanguageID,1,21));
      if (WatchyAPOn && Menu.Item == GSR_MENU_WIFI) { Menu.SubItem = 5; UpdateDisp |= Showing(); }
      ResetOTA(); EndOTA(3);
    });
    server.on("/settings", HTTP_POST, [=](){
        if (server.argName(0) == "settings") { StoreSettings(server.arg(0)); if (Options.WatchFaceStyle > WatchStyles.Count - 1) Options.WatchFaceStyle = 0; Options.NeedsSaving = true ; WatchFaceStart(Options.WatchFaceStyle, true); }
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", LGSR.LangString(settingsDone,true,Options.LanguageID,5,19));
        ResetOTA();
    });
    server.on("/wifi", HTTP_POST, [=](){
        uint8_t I = 0;
        while (I < server.args()){
            parseWiFiPageArg(server.argName(I),server.arg(I)); I++;
        }
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", LGSR.LangString(wifiDone,true,Options.LanguageID,14,19));
        Options.NeedsSaving = true;
        ResetOTA();
    });
    server.on("/weather", HTTP_POST, [=](){
        String S = ""; if (server.argName(0) == "lat") S = makeGeo(server.arg(0),true);
        String T = ""; if (server.argName(1) == "lon") T = makeGeo(server.arg(1),false);
        server.sendHeader("Connection", "close"); ResetOTA();
        WeatherData.UseStaticPOS = (T >"" && S > "");
        if (WeatherData.UseStaticPOS)
        {
            WeatherData.StaticLat = S.toDouble();
            WeatherData.StaticLon = T.toDouble();
        }
        else
        {
            WeatherData.StaticLat = 0;
            WeatherData.StaticLon = 0;
        }
        Options.NeedsSaving = true;
        server.send(200, "text/html", LGSR.LangString(weatherDone,true,Options.LanguageID,1,21)); 
    });
    server.on("/update", HTTP_POST, [=](){
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "Upload Failed." : "Watchy will reboot!");
      Reboot();
    }, [=]() {
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        ResetOTA();

        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
          OTAEnd=true;
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/

        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) OTAEnd=true;

      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          OTAEnd=true;
        }
      }
    });
    RefreshCPU(GSR_CPUMAX);
    server.begin();
}

void WatchyGSR::ResetOTA(){ OTATimer=millis(); OTAFail = OTATimer + 600000; }
void WatchyGSR::EndOTA(uint8_t Seconds) { OTAOff = millis() + Seconds * 1000; }
bool WatchyGSR::IsEndOTA() { return (OTAOff > 0 && millis() > OTAOff); }
void WatchyGSR::ResetEndOTA() { OTAOff = 0; }

void WatchyGSR::showWatchFace(){
  int I;
  bool B = (Battery.Read > Battery.MinLevel);
  if (Options.Performance && B) if (Options.Performance == 1) RefreshCPU(GSR_CPUMID); else if (Options.Performance == 2) RefreshCPU(GSR_CPULOW);
  if (Options.Feedback && DoHaptic && B && AllowHaptic) { HapticMS = 5; SoundBegin(); }
  DisplayInit();
  display.setFullWindow();
  if (GuiMode == GSR_GAMEON) drawGame(); else drawWatchFace();
  display.display(!Updates.Full); //partial refresh
  if (!(InTurbo() || SoundActive() || !DarkWait())) DisplaySleep();
  DoHaptic=false;
  Updates.Full=false;
  Updates.Drawn=true;
  UpdateDisp=false;
  Darkness.Went=false;
  Darkness.Last = millis();
}

void WatchyGSR::drawWatchFace(){
    display.fillScreen(BackColor());
    display.setTextWrap(false);
    if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr) { if (!OverrideBitmap()) { if (Design.Face.Bitmap) display.drawBitmap(0, 0, Design.Face.Bitmap, 200, 200, ForeColor(), BackColor()); } } else { if (!WatchStyles.AddOn[Options.WatchFaceStyle]->OverrideBitmap()) { if (Design.Face.Bitmap) display.drawBitmap(0, 0, Design.Face.Bitmap, 200, 200, ForeColor(), BackColor()); } }
    display.setTextColor(ForeColor());

    drawWatchFaceStyle();

    if (!GetNoStatus()){
        drawChargeMe();
        // Show WiFi/AP/TZ/NTP if in progress.
        drawStatus();
    }
    if (GuiMode == GSR_MENUON) drawMenu();
}

void WatchyGSR::drawGame(){
    if (Options.Game != 255){
        display.fillScreen(BackColor());
        display.setTextWrap(false);
        display.setTextColor(ForeColor());
        if (WatchStyles.AddOn[Options.Game] != nullptr) WatchStyles.AddOn[Options.Game]->InsertDrawWatchStyle(Options.Game);
    }
}

void WatchyGSR::drawTime(uint8_t Flags){
    String O;
    bool PM = false;
    O = MakeTime(WatchTime.Local.Hour | (Flags & 96), WatchTime.Local.Minute, PM);
    display.setFont(Design.Face.TimeFont);
    setFontColor(Design.Face.TimeColor);

    drawData(O,Design.Face.TimeLeft,Design.Face.Time,Design.Face.TimeStyle, Design.Face.Gutter, ((Flags & 32) ? false : true), PM);
}

void WatchyGSR::drawDay(){
    String O = LGSR.GetFormatID(Options.LanguageID,0);
    O.replace("{W}",LGSR.GetWeekday(Options.LanguageID, WatchTime.Local.Wday));
    setFontFor(O,Design.Face.DayFont,Design.Face.DayFontSmall,Design.Face.DayFontSmaller,Design.Face.DayGutter);
    setFontColor(Design.Face.DayColor);
    drawData(O, Design.Face.DayLeft, Design.Face.Day, Design.Face.DayStyle, Design.Face.DayGutter);
}

void WatchyGSR::drawDate(bool Short){
    String O = LGSR.GetFormatID(Options.LanguageID,1);
    O.replace("{M}",(Short ? LGSR.GetShortMonth(Options.LanguageID, WatchTime.Local.Month) : LGSR.GetMonth(Options.LanguageID, WatchTime.Local.Month)));
    O.replace("{D}",String(WatchTime.Local.Day));
    setFontFor(O,Design.Face.DateFont,Design.Face.DateFontSmall,Design.Face.DateFontSmaller,Design.Face.DateGutter);
    setFontColor(Design.Face.DateColor);
    drawData(O, Design.Face.DateLeft, Design.Face.Date, Design.Face.DateStyle, Design.Face.DateGutter);
}

void WatchyGSR::drawYear(){
    display.setFont(Design.Face.YearFont);
    setFontColor(Design.Face.YearColor);
    drawData(String(WatchTime.Local.Year + SRTC.getLocalYearOffset()), Design.Face.YearLeft, Design.Face.Year, Design.Face.YearStyle, Design.Face.Gutter);  //1900
}

void WatchyGSR::drawMenu(){
    int16_t  x1, y1, z1;
    uint16_t D,L,B,C,P;
    uint16_t w, h;
    String O, S ,T, V;
    unsigned long U;
    tmElements_t TM;

    display.drawBitmap(0, Design.Menu.Top, (Menu.Style == GSR_MENU_INOPTIONS) ? OptionsMenuBackground : MenuBackground, GSR_MenuWidth, GSR_MenuHeight, ForeColor(), BackColor());
    display.setTextColor(Options.LightMode && Menu.Style != GSR_MENU_INNORMAL ? GxEPD_WHITE : GxEPD_BLACK);
    switch (Menu.Item){
        case GSR_MENU_STEPS:
            if (Menu.SubItem > 0 && Menu.SubItem < 4) O = LGSR.GetID(Options.LanguageID,2);
            else if (Menu.SubItem == 4) O = LGSR.GetID(Options.LanguageID,3);
            else O = LGSR.GetID(Options.LanguageID,4);
            break;
        case GSR_MENU_ALARMS:
            O = LGSR.GetID(Options.LanguageID,5);
            break;
        case GSR_MENU_TIMERS:
            O = LGSR.GetID(Options.LanguageID,6);
            break;
        case GSR_MENU_GAME:
            O = LGSR.GetID(Options.LanguageID,127);
            break;
        case GSR_MENU_OPTIONS:
            O = LGSR.GetID(Options.LanguageID,7);
            break;
        case GSR_MENU_ALARM1:
            if (Menu.SubItem > 0 && Menu.SubItem < 4) O = LGSR.GetID(Options.LanguageID,8);
            else if (Menu.SubItem == 4) O = LGSR.GetID(Options.LanguageID,9);
            else if (Menu.SubItem > 4) O = LGSR.GetID(Options.LanguageID,10);
            else O = LGSR.GetID(Options.LanguageID,11);
            break;
        case GSR_MENU_ALARM2:
            if (Menu.SubItem > 0 && Menu.SubItem < 4) O = LGSR.GetID(Options.LanguageID,12);
            else if (Menu.SubItem == 4) O = LGSR.GetID(Options.LanguageID,13);
            else if (Menu.SubItem > 4) O = LGSR.GetID(Options.LanguageID,14);
            else O = LGSR.GetID(Options.LanguageID,15);
            break;
        case GSR_MENU_ALARM3:
            if (Menu.SubItem > 0 && Menu.SubItem < 4) O = LGSR.GetID(Options.LanguageID,16);
            else if (Menu.SubItem == 4) O = LGSR.GetID(Options.LanguageID,17);
            else if (Menu.SubItem > 4) O = LGSR.GetID(Options.LanguageID,18);
            else O = LGSR.GetID(Options.LanguageID,19);
            break;
        case GSR_MENU_ALARM4:
            if (Menu.SubItem > 0 && Menu.SubItem < 4) O = LGSR.GetID(Options.LanguageID,20);
            else if (Menu.SubItem == 4) O = LGSR.GetID(Options.LanguageID,21);
            else if (Menu.SubItem > 4) O = LGSR.GetID(Options.LanguageID,22);
            else O = LGSR.GetID(Options.LanguageID,23);
            break;
        case GSR_MENU_TONES:
            O = LGSR.GetID(Options.LanguageID,24);
            break;
        case GSR_MENU_TIMEDN:
            O = LGSR.GetID(Options.LanguageID,25);
            break;
        case GSR_MENU_TMEDCF:
            O = LGSR.GetID(Options.LanguageID,119);
            break;
        case GSR_MENU_TIMEUP:
            O = LGSR.GetID(Options.LanguageID,26);
            break;
        case GSR_MENU_STYL:
            O = LGSR.GetID(Options.LanguageID,27);
            break;
        case GSR_MENU_DISP:
            O = LGSR.GetID(Options.LanguageID,28);
            break;
        case GSR_MENU_LANG:
            O = LGSR.GetID(Options.LanguageID,29);
            break;
        case GSR_MENU_BRDR:
            O = LGSR.GetID(Options.LanguageID,30);
            break;
        case GSR_MENU_SIDE:
            O = LGSR.GetID(Options.LanguageID,31);
            break;
        case GSR_MENU_SWAP:
            O = LGSR.GetID(Options.LanguageID,32);
            break;
        case GSR_MENU_ORNT:
            O = LGSR.GetID(Options.LanguageID,33);
            break;
        case GSR_MENU_MODE:
            O = LGSR.GetID(Options.LanguageID,34);
            break;
        case GSR_MENU_FEED:
            O = LGSR.GetID(Options.LanguageID,35);
            break;
        case GSR_MENU_TRBO:
            O = LGSR.GetID(Options.LanguageID,36);
            break;
        case GSR_MENU_DARK:
            switch(Menu.SubItem){
                case 0:
                    O = LGSR.GetID(Options.LanguageID,37);
                    break;
                case 1:
                    O = LGSR.GetID(Options.LanguageID,38);
                    break;
                case 2:
                    O = LGSR.GetID(Options.LanguageID,39);
                    break;
                case 3:
                    O = LGSR.GetID(Options.LanguageID,40);
                    break;
                case 4:
                    O = LGSR.GetID(Options.LanguageID,41);
                    break;
                case 5:
                    O = LGSR.GetID(Options.LanguageID,42);
            }
            break;
        case GSR_MENU_SAVE:
            O = LGSR.GetID(Options.LanguageID,43);
            break;
        case GSR_MENU_TPWR:
            O = LGSR.GetID(Options.LanguageID,44);
            break;
        case GSR_MENU_INFO:
            O = LGSR.GetID(Options.LanguageID,45);
            break;
        case GSR_MENU_TRBL:
            O = LGSR.GetID(Options.LanguageID,46);
            break;
        case GSR_MENU_SYNC:
            O = LGSR.GetID(Options.LanguageID,(Menu.SubItem != 4 ? 47 : 107));
            break;
        case GSR_MENU_WEAT: // Weather Interval
            if (Menu.SubItem == 0) O = LGSR.GetID(Options.LanguageID,122);
            else if (Menu.SubItem < 3) O = LGSR.GetID(Options.LanguageID,123);
            else if (Menu.SubItem == 3) O = LGSR.GetID(Options.LanguageID,124);
            else O = LGSR.GetID(Options.LanguageID,128);
            break;
        case GSR_MENU_WIFI:
            O = LGSR.GetID(Options.LanguageID,48);
            break;
        case GSR_MENU_OTAU:
            if (Menu.SubItem == 2 || Menu.SubItem == 3) O = LGSR.GetID(Options.LanguageID,49); else O = LGSR.GetID(Options.LanguageID,50);
            break;
        case GSR_MENU_OTAM:
            if (Menu.SubItem == 2 || Menu.SubItem == 3) O = LGSR.GetID(Options.LanguageID,51); else O = LGSR.GetID(Options.LanguageID,52);
            break;
        case GSR_MENU_SCRN:
            O = LGSR.GetID(Options.LanguageID,53);
            break;
        case GSR_MENU_RSET:
            O = LGSR.GetID(Options.LanguageID,54);
            break;
        case GSR_MENU_TOFF:
            /*
             * S H:MM:SS Begin/Calculate/Current/Toggle RTC/Adjust Drift
             * 0 1 23 45 6     7         8       9             11
             */
            if (Menu.SubItem < 6) O = LGSR.GetID(Options.LanguageID,56);
            else if (Menu.SubItem > 5 && Menu.SubItem < 9) O = LGSR.GetID(Options.LanguageID,121);
            else if (Menu.SubItem == 9) O = LGSR.GetID(Options.LanguageID,106);
            else if (Menu.SubItem == 11) O = LGSR.GetID(Options.LanguageID,129);
            break;
        case GSR_MENU_UNVS:
            switch (Menu.SubItem){
                case 0:
                    O = LGSR.GetID(Options.LanguageID,57);
                    break;
                case 1:
                    O = LGSR.GetID(Options.LanguageID,58);
                    break;
                case 2:
                    O = LGSR.GetID(Options.LanguageID,59);
                    break;
            }
    }
    setFontFor(O, Design.Menu.Font, Design.Menu.FontSmall, Design.Menu.FontSmaller, Design.Menu.Gutter);
    display.getTextBounds(O, 0, Design.Menu.Header + Design.Menu.Top, &x1, &y1, &w, &h);
    w = (196 - w) /2;
    display.setCursor(w + 2, Design.Menu.Header + Design.Menu.Top);
    display.print(O);
    display.setTextColor(GxEPD_BLACK);  // Only show menu in Light mode
    Updates.Highlighting.Active=false;
    Updates.Indexing.Active=false;
    if (Menu.Item == GSR_MENU_STEPS){  //Steps
        if (Menu.SubItem == 0) O = CurrentSteps(true);
        else if (Menu.SubItem < 4) {
            T = MakeHour(Steps.Hour);
            S = MakeMinutes(Steps.Minutes);
            O = T + ":" + S + MakeTOD(Steps.Hour, true);
            Updates.Indexing.Active = true;
            Updates.Indexing.Offset = Menu.SubItem - 1 + (Menu.SubItem > 1 ? T.length() : 0);
            Updates.Indexing.Width = (Menu.SubItem == 1 ? T.length() : 1);
        } else O = LGSR.GetID(Options.LanguageID,60);
    }else if (Menu.Item == GSR_MENU_ALARMS){
            O = LGSR.GetID(Options.LanguageID,61);
    }else if (Menu.Item >= GSR_MENU_ALARM1 && Menu.Item <= GSR_MENU_ALARM4){  // Alarms
        O = "";
        if (Menu.SubItem == 0) O = LGSR.GetID(Options.LanguageID,62);
        else if (Menu.SubItem < 5){
            S=MakeMinutes(Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1]);
            T=getReduce(Alarms_Repeats[Menu.Item - GSR_MENU_ALARM1]);
            O=MakeHour(Alarms_Hour[Menu.Item - GSR_MENU_ALARM1]);
            V=MakeTOD(Alarms_Hour[Menu.Item - GSR_MENU_ALARM1], false) + " ";
            Updates.Indexing.Active=true;
            Updates.Indexing.Offset = Menu.SubItem - 1 + (Menu.SubItem > 1 ? O.length() : 0) + (Menu.SubItem > 3 ? S.length() + V.length() - 2 : 0);
            Updates.Indexing.Width = (Menu.SubItem == 4 ? T.length() : (Menu.SubItem == 1 ? O.length() : 1));
            O += ":" + S + V + T;
        }else if (Menu.SubItem > 4 && Menu.SubItem < 14){
            O = ""; Updates.Highlighting.Active=true;
            Updates.Highlighting.Bits=0;
            for (L = 0; L < 7; L++){
                T = LGSR.GetWeekday(Options.LanguageID,L);
                O = O + T.charAt(0);
                Updates.Highlighting.Bits |= ((Alarms_Active[Menu.Item - GSR_MENU_ALARM1] & Bits[L]) == Bits[L] ? Bits[L] : 0);
            }
            T=LGSR.GetID(Options.LanguageID,63); O = O + " " + T.charAt(0);
            Updates.Highlighting.Bits |= ((Alarms_Active[Menu.Item - GSR_MENU_ALARM1] & GSR_ALARM_REPEAT) == GSR_ALARM_REPEAT) ? Bits[8] : 0;
            T=LGSR.GetID(Options.LanguageID,64); O = O + " " + T.charAt(0);
            Updates.Highlighting.Bits |= ((Alarms_Active[Menu.Item - GSR_MENU_ALARM1] & GSR_ALARM_ACTIVE) == GSR_ALARM_ACTIVE) ? BitValue(10) : 0;
            Updates.Indexing.Active=true;
            Updates.Indexing.Offset = Menu.SubItem - 5 + (Menu.SubItem > 11 ? 1 : 0) + (Menu.SubItem == 13 ? 1 : 0);
            Updates.Indexing.Width = 1;
        }
    }else if (Menu.Item == GSR_MENU_TONES){   // Repeats on Alarms.
        O = getReduce(Options.MasterRepeats) + " " + LGSR.GetID(Options.LanguageID,65);
    }else if (Menu.Item == GSR_MENU_TIMERS){  // Timers
        O = LGSR.GetID(Options.LanguageID,66);
    }else if (Menu.Item == GSR_MENU_TIMEDN){ // Countdown
        if (TimerDown.Active) UpdateUTC();
        S=MakeMinutes(TimerDown.Active ? TimerDown.Mins : TimerDown.MaxMins);
        T=MakeMinutes(TimerDown.Active ? TimerDown.Secs : TimerDown.MaxSecs);
        if (Menu.SubItem == 0) O = LGSR.GetID(Options.LanguageID,62);
        else {
            V = String(TimerDown.Active ? TimerDown.Hours : TimerDown.MaxHours);
            O = V + ":" + S + ":" + T + " " + CountdownTimerState();
            Updates.Indexing.Active=true;
            Updates.Indexing.Offset=Menu.SubItem - 1 + (Menu.SubItem > 1 ? V.length() : 0) + (Menu.SubItem > 3 ? 1 : 0) + (Menu.SubItem > 5 ? 1 : 0);
            Updates.Indexing.Width=(Menu.SubItem == 1 ? V.length() : (Menu.SubItem > 5 ? CountdownTimerState().length() : 1));
        }
    }else if (Menu.Item == GSR_MENU_TMEDCF){ // Countdown Settings
        if (Menu.SubItem == 0) O = LGSR.GetID(Options.LanguageID,62);
        else {
            V = LGSR.GetID(Options.LanguageID,(TimerDown.Repeats ? 118 : 117));
            T = getReduce(TimerDown.MaxTones);
            O = V + " " + T;
            Updates.Indexing.Offset = (Menu.SubItem == 1 ? 0 : (Menu.SubItem == 2 ? V.length() + 1 : 0));
            Updates.Indexing.Width = (Menu.SubItem == 1 ? V.length() : (Menu.SubItem == 2 ? T.length() : 0));
            Updates.Indexing.Active = true;
        }
    }else if (Menu.Item == GSR_MENU_TIMEUP){ // Elapsed
        switch (Menu.SubItem){
            case 0:
                O = LGSR.GetID(Options.LanguageID,62);
                break;
            case 1:
                if (TimerUp.Active) { UpdateUTC(); U = (WatchTime.UTC_RAW - TimerUp.SetAt); } else U = (TimerUp.StopAt - TimerUp.SetAt);
                y1 = U / 3600; x1 = (U - (y1 * 3600)) / 60; z1 = U % 60;
                V = String(y1) + ":" + MakeMinutes(x1) + ":" + MakeMinutes(z1) + " ";
                S = ElapsedTimerState(); O = V + S;
                Updates.Indexing.Offset = V.length() + 1;
                Updates.Indexing.Width = S.length();
                Updates.Indexing.Active = true;
        }
    }else if (Menu.Item == GSR_MENU_GAME){ // Game Menu
        switch (Menu.SubItem){
            case 0:
                O = LGSR.GetID(Options.LanguageID,67);
                break;
            case 1:
                O = LGSR.LangString(CurrentGameStyle(),false,Options.LanguageID,0,1);
        }
    }else if (Menu.Item == GSR_MENU_OPTIONS){ // Options Menu
        O = LGSR.GetID(Options.LanguageID,70);
    }else if (Menu.Item == GSR_MENU_STYL){  // Switch Watch Style
        O = LGSR.LangString(CurrentWatchStyle(),false,Options.LanguageID,0,1); // Only translate the onboard ones.
    }else if (Menu.Item == GSR_MENU_LANG){  // Show current Language
          O = LGSR.GetLangName(Options.LanguageID);
    }else if (Menu.Item == GSR_MENU_DISP){  // Switch Mode
          O = LGSR.GetID(Options.LanguageID,(Options.LightMode ? 71 : 72));
    }else if (Menu.Item == GSR_MENU_SIDE){  // Dexterity Mode
          O = LGSR.GetID(Options.LanguageID,(Options.Lefty ? 73 : 74));
    }else if (Menu.Item == GSR_MENU_SWAP){  // Swap Menu/Back Buttons
          O = LGSR.GetID(Options.LanguageID,(Options.Swapped ? 75 : 76));
    }else if (Menu.Item == GSR_MENU_BRDR){  // Border Mode
          O = LGSR.GetID(Options.LanguageID,(Options.Border) ? 72 : 71);
    }else if (Menu.Item == GSR_MENU_ORNT){  // Watchy Orientation.
          O = LGSR.GetID(Options.LanguageID,(Options.Orientated ? 77 : 78));
    }else if (Menu.Item == GSR_MENU_MODE){  // 24hr Format Swap.
          O = LGSR.GetID(Options.LanguageID,(Options.TwentyFour ? 79 : 80));
    }else if (Menu.Item == GSR_MENU_FEED){  // Feedback
            O = LGSR.GetID(Options.LanguageID,(Options.Feedback ? 81 : 83));
    }else if (Menu.Item == GSR_MENU_TRBO){  // Turbo!
        if (Options.Turbo) O=String(Options.Turbo) + " " + MakeSeconds(Options.Turbo); else O = LGSR.GetID(Options.LanguageID,68);
    }else if (Menu.Item == GSR_MENU_DARK){  // Dark Running.
            if (Menu.SubItem == 0) O = LGSR.GetID(Options.LanguageID,67);
            else if (Menu.SubItem == 2) O = String(Options.SleepMode) + " " + MakeSeconds(Options.SleepMode);
            else if (Menu.SubItem == 5) O = LGSR.GetID(Options.LanguageID,(Options.BedTimeOrientation ? 77 : 78));
            else if (Menu.SubItem == 1){
                switch (Options.SleepStyle){
                    case 0:
                        O = LGSR.GetID(Options.LanguageID,83);
                        break;
                    case 1:
                        O = LGSR.GetID(Options.LanguageID,84);
                        break;
                    case 2:
                        O = LGSR.GetID(Options.LanguageID,85);
                        break;
                    case 3:
                        O = LGSR.GetID(Options.LanguageID,86);
                        break;
                    case 4:
                        O = LGSR.GetID(Options.LanguageID,87);
                }
            } else {
                S = MakeHour(Options.SleepStart) + MakeTOD(Options.SleepStart, true);
                T = " " + LGSR.GetID(Options.LanguageID,88) + " ";
                V = MakeHour(Options.SleepEnd) + MakeTOD(Options.SleepEnd, true);
                O = S + T + V;
                Updates.Indexing.Active = true;
                Updates.Indexing.Offset = (Menu.SubItem == 4 ? S.length() + T.length() : 0);
                Updates.Indexing.Width = (Menu.SubItem == 3 ? S.length() : T.length());
            }
    }else if (Menu.Item == GSR_MENU_TPWR){  // WiFi Tx Power
        O = IDWiFiTX[getTXOffset(GSRWiFi.TransmitPower)];
    }else if (Menu.Item == GSR_MENU_INFO){  // Information
        switch (Menu.SubItem){
            case 0:
                O = LGSR.GetID(Options.LanguageID,89) + ": " + String(Build);
                break;
            case 1:
                O = LGSR.GetID(Options.LanguageID,90) + ": " + String(Battery.Read - (Battery.Read > GSR_MaxBattery ? 1.00 : 0.00)) + "V";
                break;
            case 2:
                if (HWVer > 1.0f) { if (HWVer == 3.0f) O = "V3 ESP32S3"; else O = (HWVer == 2.0f) ? "V2.0 PCF8563" : "V1.5 PCF8563"; } else O = "V1 DS3231M";
                if (HWVer != 3.0f && SRTC.onESP32()) O += " ESP";
                break;
        }
    }else if (Menu.Item == GSR_MENU_SAVE){  // Performance
        if (Options.Performance == 2) O = LGSR.GetID(Options.LanguageID,91);
        else O = LGSR.GetID(Options.LanguageID,(Options.Performance == 1 ? 76 : 92));
    }else if (Menu.Item == GSR_MENU_TRBL){  // Troubleshooting.
        O = LGSR.GetID(Options.LanguageID,70);
    }else if (Menu.Item == GSR_MENU_SYNC){  // NTP
        if (Menu.SubItem > 4 && Menu.SubItem < 9){
            V = MakeHour(NTPData.SyncHour); S = MakeMinutes(NTPData.SyncMins);
            T = LGSR.GetID(Options.LanguageID, (NTPData.AutoSync ? 69 : 68));
            O = T + " " + V + ":" + S + MakeTOD(NTPData.SyncHour,true);
            Updates.Indexing.Offset = Menu.SubItem - 5 + (Menu.SubItem > 5 ? T.length() : 0) + (Menu.SubItem > 6 ? V.length() : 0);
            Updates.Indexing.Width = (Menu.SubItem == 5 ? T.length() : (Menu.SubItem == 6 ? V.length() : 1));
            Updates.Indexing.Active = true;
        }else if (Menu.SubItem > 8){
            O = ""; Updates.Highlighting.Active=true;
            Updates.Highlighting.Bits=0;
            for (L = 0; L < 7; L++){
                T = LGSR.GetWeekday(Options.LanguageID,L);
                O += T.charAt(0);
                Updates.Highlighting.Bits |= ((NTPData.SyncDays & Bits[L]) == Bits[L] ? Bits[L] : 0);
            }
            Updates.Indexing.Active=true;
            Updates.Indexing.Offset = Menu.SubItem - 9;
            Updates.Indexing.Width = 1;
        }else{
            switch (Menu.SubItem){
                case 0:
                    O = LGSR.GetID(Options.LanguageID,93);
                    break;
                case 1:
                    O = LGSR.GetID(Options.LanguageID,94);
                    break;
                case 2:
                    O = LGSR.GetID(Options.LanguageID,95);
                    break;
                case 3:
                    O = LGSR.GetID(Options.LanguageID,96);
                    break;
                case 4:
                    O = LGSR.GetID(Options.LanguageID,105);
            }
        }
    }else if (Menu.Item == GSR_MENU_WEAT){ // Weather Interval
        /* DO WEATHER TIME HERE */
        if (Menu.SubItem == 0) O = LGSR.GetID(Options.LanguageID,67);
        else if (Menu.SubItem > 0 && Menu.SubItem < 3){
                if (WeatherData.Interval == 0) O = LGSR.GetID(Options.LanguageID,83);
                else{
                    D = WeatherData.Interval * 300;
                    L = D/3600; O = ""; V=String(L); B = V.length();
                    Updates.Indexing.Offset = ((Menu.SubItem == 1 || L == 0) ? 0 : 2 + B);
                    Updates.Indexing.Width = ((Menu.SubItem == 1 && L) ? B : 2);
                    Updates.Indexing.Active = true;
                    if (L) O=MakeHour(L) + "H:";
                    D-=(L*3600); O+=MakeMinutes(D/60) + "M";
                }
        }else if (Menu.SubItem == 3) O = LGSR.GetID(Options.LanguageID,(IsMetric() ? 125 : 126));
        else O = LGSR.GetID(Options.LanguageID,97);
    }else if (Menu.Item == GSR_MENU_WIFI){ // WiFi Connect.
        if (Menu.SubItem == 0){
            O = LGSR.GetID(Options.LanguageID,97);
        }else if (Menu.SubItem == 1){
            O = LGSR.GetID(Options.LanguageID,98);
        }else if (Menu.SubItem == 2){
            O = WiFi_AP_SSID;
        }else if (Menu.SubItem == 3){
            O = WiFi.softAPIP().toString();
        }else if (Menu.SubItem == 4){
            O = LGSR.GetID(Options.LanguageID,99);
        }else if (Menu.SubItem == 5){
            O = LGSR.GetID(Options.LanguageID,130);
        }
    }else if (Menu.Item == GSR_MENU_OTAU || Menu.Item == GSR_MENU_OTAM){  // OTA Update.
        if (Menu.SubItem == 0){
            O = LGSR.GetID(Options.LanguageID,100);
        }else if (Menu.SubItem == 1){
            O = LGSR.GetID(Options.LanguageID,101);
        }else if (Menu.SubItem == 2 || Menu.SubItem == 3){
            O = GSRWiFi.LocalIP.toString();
        }
    }else if (Menu.Item == GSR_MENU_SCRN){  // Reset Screen.
        O = LGSR.GetID(Options.LanguageID,60);
    }else if (Menu.Item == GSR_MENU_RSET){  // Reboot Watchy
        switch (Menu.SubItem){
            case 1:
                O = LGSR.GetID(Options.LanguageID,102);
                break;
            default:
                O = LGSR.GetID(Options.LanguageID,103);
        }
    }else if (Menu.Item == GSR_MENU_TOFF){  // Time Travel detect.
        /*
         * S H:MM:SS Begin/Calculate/Current/Toggle RTC/Adjust Drift
         * 0 1 23 45 6     7         8       9             11
         */
        if (Menu.SubItem == 0) { O = LGSR.GetID(Options.LanguageID,62); NTPData.TravelTemp = 0; }
        else if (Menu.SubItem > 0 && Menu.SubItem < 6){
            UpdateUTC();
            TM = UTCtoLocal(WatchTime.UTC_RAW + NTPData.TravelTemp);
            S = MakeHour(TM.Hour);
            V = MakeMinutes(TM.Minute);
            T = MakeMinutes(TM.Second);
            O = S + ":" + V + ":" + T + MakeTOD(TM.Hour,true);
            Updates.Indexing.Active = true;
            Updates.Indexing.Offset = Menu.SubItem - 1 + (Menu.SubItem > 1 ? S.length() : 0) + (Menu.SubItem > 3 ? 1 : 0);
            Updates.Indexing.Width = (Menu.SubItem == 1 ? S.length() : 1);
        }else if (Menu.SubItem == 6){ // Drift Begin
            O = LGSR.GetID(Options.LanguageID,55);
        }else if (Menu.SubItem == 7){ // Drift Calculate
            O = LGSR.GetID(Options.LanguageID,120);
        }else if (Menu.SubItem == 8){ // Drift Current
            O = String(SRTC.getDrift(WatchTime.ESPRTC)) + "/" + (SRTC.isFastDrift(WatchTime.ESPRTC) ? "-1" : "1");
        }else if (Menu.SubItem == 9 && HWVer != 3.0f){ // Toggle ESP32 RTC
            O = LGSR.GetID(Options.LanguageID,(WatchTime.ESPRTC ? 81 : 83));  /* WatchTime.ESPRTC, really says use the ESP32 */
        }else if (Menu.SubItem == 11){ // Drift Current
            S = String(SRTC.getDrift(WatchTime.ESPRTC));
            Menu.SubSubItem = (Menu.SubSubItem < S.length() ? Menu.SubSubItem : S.length() - 1);
            Updates.Indexing.Active = true;
            Updates.Indexing.Offset = Menu.SubSubItem;
            Updates.Indexing.Width = 1;
            O = S + "/" + (SRTC.isFastDrift(WatchTime.ESPRTC) ? "-1" : "1");
        }
    }else if (Menu.Item == GSR_MENU_UNVS){  // USE NVS
        switch (Menu.SubItem){
            case 0:
                O = LGSR.GetID(Options.LanguageID,(OkNVS(GName) ? 108 : 109));
                break;
            case 1:
                O = LGSR.GetID(Options.LanguageID,(Menu.SubSubItem == 0 ? 110 : 67));
                break;
            case 2:
                O = LGSR.GetID(Options.LanguageID,111);
                break;
        }
    }

    if (O){
        setFontFor(O, Design.Menu.Font, Design.Menu.FontSmall, Design.Menu.FontSmaller, Design.Menu.Gutter);
        D = Design.Menu.Data + Design.Menu.Top;
        display.getTextBounds(O, 0, D, &x1, &y1, &w, &h);
        /*
         * D = Down (vertical)
         * L = Left (horizontal)
         * B = Begin (horizontal)
         * C = Count (characters horizontal)
         * P = Position (characters)
        */
        L = ((196 - w) /2) + 2; B = L; C = 0;
        display.setCursor(L, Design.Menu.Data + Design.Menu.Top);
        if (Updates.Indexing.Active || Updates.Highlighting.Active){
            for (P = 0; P < O.length(); P++){
                T = O.charAt(P);
                if (Updates.Indexing.Active && P == Updates.Indexing.Offset){ C = Updates.Indexing.Width; B = L; }
                if (Updates.Highlighting.Active) display.setTextColor(isBitOn(Updates.Highlighting.Bits,P) ? GxEPD_WHITE : GxEPD_BLACK);
                display.print(T);
                L = display.getCursorX();  /* Read the Cursor AFTER printing */
                if (Updates.Indexing.Active){
                    if (C > 0){
                        C--;
                        if (C == 0){
                            display.fillRect(B, D + 5, L - B, 3, GxEPD_BLACK);
                        }
                    }
                }
            }
        }else display.print(O);
    }
}

void WatchyGSR::drawData(String dData, byte Left, byte Bottom, WatchyGSR::DesOps Style, byte Gutter, bool isTime, bool PM){
    uint16_t w, Width, Height, Ind;
    int16_t X, Y;

    display.getTextBounds(dData, Left, Bottom, &X, &Y, &Width, &Height);

    Bottom = constrain(Bottom, Gutter, 200 - Gutter);
    switch (Style){
        case WatchyGSR::dLEFT:
            Left = Gutter;
            break;
        case WatchyGSR::dRIGHT:
            Left = constrain(200 - (Gutter + Width), Gutter, 200 - Gutter);
            break;
        case WatchyGSR::dSTATIC:
            Left = constrain(Left, Gutter, 200 - Gutter);
            break;
        case WatchyGSR::dCENTER:
            Left = constrain(4 + ((196 - (Gutter + Width)) / 2), Gutter, 200 - Gutter);
            break;
    };
    display.setCursor(Left, Bottom);
    display.print(dData);

    if (isTime && PM){
        if (Style == WatchyGSR::dRIGHT) Left = constrain(Left - 12, Gutter, 200 - Gutter);
        else Left = constrain(Left + Width + 6, Gutter, 190);
        display.drawBitmap(Left, Bottom - Design.Face.TimeHeight, PMIndicator, 6, 6, ForeColor());
    }
}

void WatchyGSR::setFontFor(String O, const GFXfont *Normal, const GFXfont *Small, const GFXfont *Smaller, byte Gutter){
    int16_t  x1, y1;
    uint16_t w, h;
    display.setTextWrap(false);
    byte wi = (200 - (2 * Gutter));
    display.setFont(Normal); display.getTextBounds(O, 0, 0, &x1, &y1, &w, &h);
    if (w > wi) { display.setFont(Small); display.getTextBounds(O, 0, 0, &x1, &y1, &w, &h); }
    if (w > wi) { display.setFont(Smaller); display.getTextBounds(O, 0, 0, &x1, &y1, &w, &h); }
 }

void WatchyGSR::setFontColor(uint16_t Color){
    bool B = (Color == GxEPD_BLACK || Color == GxEPD_WHITE);
    if (!B && Color == GSR_AutoFore) Color = ForeColor();
    display.setTextColor(Color);
}

void WatchyGSR::deepSleep(){
  uint8_t N, D, H; // 3.0
  uint64_t cI;
  bool BatOk, BT,B, DM, CD, M;
  struct tm * CT;

  DM = false; CD=TimerDown.Active;
  UpdateUTC(); UpdateClock(); M =(GuiMode != GSR_MENUON);
  if (CD && (TimerDown.StopAt - WatchTime.UTC_RAW) > 60) CD = false;

  B = false; VibeTo(false);
  UpdateBMA();
  DM = (Darkness.Went && !CD && M);

  if (DM){
    H = WatchTime.Local.Hour;
    BatOk = (Battery.Read == 0 || Battery.Read > Battery.LowLevel);
    BT = (Options.SleepStyle == 2 && WatchTime.BedTime);
    B = ((Options.SleepStyle == 1 || (Options.SleepStyle > 2 && Options.SleepStyle != 4)) || BT);
    if (Battery.Direction == 1 || Battery.State == 3) N = (WatchTime.UTC.Minute - (WatchTime.UTC.Minute % 5) + 5) % 60; else N = (WatchTime.UTC.Minute < 30 ? 30 : 60);
    if (WatchTime.NextAlarm != 99){ if (Alarms_Minutes[WatchTime.NextAlarm] > WatchTime.Local.Minute && Alarms_Minutes[WatchTime.NextAlarm] < N) N = Alarms_Minutes[WatchTime.NextAlarm]; }
    if (TimerDown.Active){
      CT=localtime(&TimerDown.StopAt);
      if (CT->tm_hour == H && CT->tm_min < N && CT->tm_min > WatchTime.Local.Minute) N = CT->tm_min;
    }
    if (WeatherData.LastCall != 0 && GetWantWeather()){
      CT=localtime(&WeatherData.LastCall);
      if (CT->tm_hour == H && CT->tm_min < N && CT->tm_min > WatchTime.Local.Minute) N = CT->tm_min;
    }
    if (Steps.Hour == H && Steps.Minutes < N && Steps.Minutes > WatchTime.Local.Minute) N = Steps.Minutes;
    if (!SRTC.checkingDrift(WatchTime.ESPRTC) && NTPData.AutoSync && (NTPData.SyncDays & Bits[WatchTime.Local.Wday]) && NTPData.SyncHour == H && NTPData.SyncMins < N && NTPData.SyncMins > WatchTime.Local.Minute) N = NTPData.SyncMins;
  }
  if (Options.NeedsSaving) RecordSettings();
  GoDark(M); DisplaySleep();
  if (DM) SRTC.atMinuteWake(N); else SRTC.nextMinuteWake();
  ForceInputs();
#ifdef STABLEBMA_H_INCLUDED
  esp_sleep_enable_ext1_wakeup((B && HWVer != 3.0f ? SBMA.WakeMask() : 0) | GSR_BTN_MASK, (esp_sleep_ext1_wakeup_mode_t)GSR_ESP_WAKEUP); //enable deep sleep wake on button press  ... |ACC_INT_MASK
#else
  esp_sleep_enable_ext1_wakeup(GSR_BTN_MASK, (esp_sleep_ext1_wakeup_mode_t)GSR_ESP_WAKEUP); //enable deep sleep wake on button press  ... |ACC_INT_MASK
#endif
  RefreshCPU(GSR_CPULOW);
  esp_deep_sleep_start();
}

void WatchyGSR::GoDark(bool DeepSleeping){
  bool B;
  if (Options.SleepStyle == 0 || (Options.SleepStyle == 2 && !WatchTime.BedTime) || GuiMode == GSR_MENUON) return;
  if ((Updates.Drawn || !Darkness.Went) && (DeepSleeping || !Showing()))
  {
    Darkness.Went=true;
    Darkness.Woke=false;
    Darkness.Tilt=0;
    Updates.Init=Updates.Drawn;
    display.setFullWindow();
    DisplayInit(true);  // Force it here so it fixes the border.
    display.fillScreen(GxEPD_BLACK);
    if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr) { if (!OverrideSleepBitmap()) { if (Design.Face.SleepBitmap) display.drawBitmap(0, 0, Design.Face.SleepBitmap, 200, 200, GxEPD_WHITE, GxEPD_BLACK); } } else { if (!WatchStyles.AddOn[Options.WatchFaceStyle]->OverrideSleepBitmap()) { if (Design.Face.SleepBitmap) display.drawBitmap(0, 0, Design.Face.SleepBitmap, 200, 200, GxEPD_WHITE, GxEPD_BLACK); else B = OverrideSleepBitmap(); } }
    drawChargeMe(true);
    drawStatus(true);
    Battery.DarkDirection = Battery.Direction;
    Battery.DarkState = Battery.State;
    display.display(true);
    Updates.Drawn=false;
    display.hibernate();
    LastHelp = 0;
  }
}

void WatchyGSR::espPinSetup(gpio_num_t pin, bool pullUp, bool bOutput){
  if (GSR_PIN_RTC == 255 && pin < 22) {
    rtc_gpio_set_direction(pin, (bOutput ? RTC_GPIO_MODE_OUTPUT_ONLY : RTC_GPIO_MODE_INPUT_ONLY));
    if (pullUp) {
      rtc_gpio_pulldown_dis(pin);
      rtc_gpio_pullup_en(pin);
    } else {
      rtc_gpio_pullup_dis(pin);
      rtc_gpio_pulldown_en(pin);
    }
  }
}

void WatchyGSR::ForceInputs(){
if (GSR_PIN_RTC == 255){
    espPinSetup((gpio_num_t)GSR_UP_PIN,true,false);
    espPinSetup((gpio_num_t)GSR_PIN_USB_DET,false,false);
    espPinSetup((gpio_num_t)GSR_PIN_ACC_INT1,true,false);
//    esp_sleep_enable_ext0_wakeup((gpio_num_t)GSR_PIN_USB_DET, digitalRead(GSR_PIN_USB_DET) ? HIGH : LOW);
    return;
}
uint8_t P = SRTC.getADCPin();
/* Unused GPIO PINS */
    pinMode(0,INPUT);   /* BTN3 v3 */
    pinMode(2,INPUT);   /*  ??  */
    if (GSR_PIN_ACC_INT2 != 13) pinMode(13,INPUT);  /*  ??  */
    pinMode(15,INPUT);  /*  ??  */
    pinMode(20,INPUT);  /*  ??  */
    pinMode(36,INPUT);  /*  ??  */
    pinMode(37,INPUT);  /*  ??  */
    pinMode(38,INPUT);  /*  ??  */
    pinMode(39,INPUT);  /*  ??  */
/* RS232 */
    pinMode(3,INPUT);   /*  RX  */
/* BUTTONS */
    pinMode(26,INPUT);  /* BTN1 */
    pinMode(25,INPUT);  /* BTN2 */
    if (GSR_UP_PIN != 32) pinMode(32,INPUT);  /* BTN3     */
    if (!(GSR_UP_PIN == 35 || P == 35)) pinMode(35,INPUT);  /* ADC/BTN3 */
    pinMode(4,INPUT);   /* BTN4 */
/* RTC */
    pinMode(21,INPUT);  /* SDA  */
    pinMode(22,INPUT);  /* SCL  */
/* DISPLAY */
    pinMode(5,INPUT);   /*  CS  */
    if (P != 9) pinMode(9,INPUT);   /* RES/ADC v3  */
    pinMode(10,INPUT);  /*  DC  */
    pinMode(18,INPUT);  /* SLCK */
    pinMode(19,INPUT);  /* BUSY */
    pinMode(23,INPUT);  /* MOSI */
/* BMA 423 */
    pinMode(12,INPUT);  /* INT2/SDA v3 */
    pinMode(14,INPUT);  /* INT1 */
}

void WatchyGSR::detectBattery(){
    float BATOff, Diff, Uping;
    uint8_t Mins;
    bool B = false;
    if ((Battery.ReadFloat[Battery.FloatTop].Stamp + 60 > WatchTime.UTC_RAW) && GSR_PIN_STAT == 255) return;
    if (GSR_PIN_STAT == 255){
        Battery.FloatTop = roller(Battery.FloatTop + 1, 0, 4);
        Battery.FloatBottom = roller(Battery.FloatBottom + 1, 0, 4);
        Battery.ReadFloat[Battery.FloatTop].Stamp = WatchTime.UTC_RAW;
        Battery.ReadFloat[Battery.FloatTop].Voltage = rawBatteryVoltage();
        Battery.Read = Battery.ReadFloat[Battery.FloatTop].Voltage / 100;
        Diff = (Battery.ReadFloat[Battery.FloatTop].Stamp - Battery.ReadFloat[Battery.FloatBottom].Stamp) / 60;
        Mins = floor(Diff);
        Diff = (Battery.ReadFloat[Battery.FloatTop].Voltage - Battery.ReadFloat[Battery.FloatBottom].Voltage);
        BATOff = (round(Diff) * 100) / 100;
        if (Mins < 1) return; // No minute.
        if (!Battery.Ugh){
            Uping = (Darkness.Went && Mins > 1 && Battery.ReadFloat[Battery.FloatTop].Voltage < 425.00) ? (Mins * 0.4) : 0.4;
            if (BATOff < -10) Battery.Level = -2;                                     /* Battery sees a massive jump due to it being unplugged */
            else if (BATOff < 0.00) { Battery.Level = gobig(Battery.Level - 1, -2); } /* Drop down to -2 for discharging */
            else if (BATOff > 10) Battery.Level = 2;                                 /* Battery sees a massive jump due to it being plugged in */
            else if (BATOff > Uping) Battery.Level = golow(Battery.Level + 1, 2);    /* Jump up to 3 for charging */
            if (Battery.ReadFloat[Battery.FloatTop].Voltage > 424 && Battery.ReadFloat[Battery.FloatBottom].Voltage > 424 && !Battery.Ugh) Battery.CState += gobig(Mins - 4, 1);
            if (Battery.CState > 150 && !Battery.Ugh) { Battery.Ugh = true; if (OkNVS(GName)) { B = NVS.setString(Bugh,Bugh); NVS.commit(); } }
            if (Battery.Ugh && Battery.Level > 0) Battery.Level = 0;
        }
    }else {
        B = Battery.Charge;
        Battery.Charge = (analogRead(GSR_PIN_STAT) * 0.80586 > 3200 && digitalRead(GSR_PIN_USB_DET));
        Battery.Read = rawBatteryVoltage() / 100; Battery.Level = (Battery.Charge ? 2 : -2);
        if (B != Battery.Charge) { Updates.Drawn = true; UpdateDisp |= Showing(); }
        B = false;
    }
    if (Battery.Level > 1){
        Battery.Direction = 1;
        // Check if the NTP has been done.
        B = (WatchTime.UTC_RAW - NTPData.Last > 14400);
    }else if (Battery.Level < 0){
        Battery.Direction = -1;
    }
    if (!SRTC.checkingDrift(WatchTime.ESPRTC)) B |= (WatchTime.Local.Hour == NTPData.SyncHour && WatchTime.Local.Minute == NTPData.SyncMins && NTPData.AutoSync && (NTPData.SyncDays & Bits[WatchTime.Local.Wday]) && WatchTime.UTC_RAW - NTPData.Last > 59);

    Mins = Battery.State; /* Set update for display if the battery state changes */

    // Do battery state here.
    if (WatchTime.UTC_RAW > Battery.LastState || GSR_PIN_STAT != 255){
        Battery.LastState = WatchTime.UTC_RAW + 10;
        if (Battery.Direction > 0) Battery.State = 3;
        else Battery.State = (Battery.Read > Battery.MinLevel ? (Battery.Read > Battery.LowLevel ? 0 : 1) : 2);
        if (Mins != Battery.State || (Darkness.Went && (Battery.DarkState != Battery.State || Battery.DarkDirection != Battery.Direction))) { Updates.Drawn = true; UpdateDisp |= Showing(); } // Fix the battery indicator.
        if (Battery.Read > Battery.RadioLevel && B && Battery.State == 3) StartNTP(true);
    }
}

bool WatchyGSR::inBrownOut() {
  float B = golow(getBatteryVoltage(),4.22);
  return (B && B < 3.36 && Battery.Start >= 3.4);
  }

/*  RTC_CNTL_BROWN_OUT_REG:
 *  CHIP_ESP32   = 1, //!< ESP32     3FF480D4
 *  CHIP_ESP32S2 = 2, //!< ESP32-S2  3F4080E4
 *  CHIP_ESP32S3 = 9, //!< ESP32-S3  600080E8
 *  CHIP_ESP32C3 = 5, //!< ESP32-C3  600080D8

#define CHIP_FEATURE_EMB_FLASH      BIT(0)      //!< Chip has embedded flash memory
#define CHIP_FEATURE_WIFI_BGN       BIT(1)      //!< Chip has 2.4GHz WiFi
#define CHIP_FEATURE_BLE            BIT(4)      //!< Chip has Bluetooth LE
#define CHIP_FEATURE_BT             BIT(5)      //!< Chip has Bluetooth Classic
#define CHIP_FEATURE_IEEE802154     BIT(6)      //!< Chip has IEEE 802.15.4
 
 */

void WatchyGSR::BrownOutDetect(bool On) { if (Watchy_Chip_Info.BrownOutDetection && GSR_PIN_RTC != 255) WRITE_PERI_REG(Watchy_Chip_Info.BrownOutDetection, (On ? 1 : 0)); }

void WatchyGSR::SetupESPValues(){
    uint32_t F; // Feature.
    esp_chip_info_t Chip_Info[sizeof(esp_chip_info_t)];
    esp_chip_info(Chip_Info);
    if (Chip_Info->model == CHIP_ESP32){
            F = Chip_Info->features;
            Watchy_Chip_Info.Base = 0x3FF48000;
            Watchy_Chip_Info.BrownOutDetection = Watchy_Chip_Info.Base + 0xD4;
    } else if (Chip_Info->model == CHIP_ESP32S2){
            F = Chip_Info->features;
            Watchy_Chip_Info.Base = 0x3F408000;
            Watchy_Chip_Info.BrownOutDetection = Watchy_Chip_Info.Base + 0xD8;
    } else if (Chip_Info->model == CHIP_ESP32S3){
            F = Chip_Info->features;
            Watchy_Chip_Info.Base = 0x60008000;
            Watchy_Chip_Info.BrownOutDetection = Watchy_Chip_Info.Base + 0xE8;
    } else if (Chip_Info->model == CHIP_ESP32C3){
            F = Chip_Info->features;
            Watchy_Chip_Info.Base = 0x60008000;
            Watchy_Chip_Info.BrownOutDetection = Watchy_Chip_Info.Base + 0xD8;
    }
    if (F){
        Watchy_Chip_Info.HasWiFi = (F & (CHIP_FEATURE_WIFI_BGN | CHIP_FEATURE_IEEE802154));
        Watchy_Chip_Info.HasBLE = (F & CHIP_FEATURE_BLE);
        Watchy_Chip_Info.HasBT = ((F & CHIP_FEATURE_BT) | Watchy_Chip_Info.HasBLE) ;
    }
}

void WatchyGSR::espPinModes() {
    if (setPinModes) return;
    setPinModes = true;
    pinMode(GSR_MENU_PIN,INPUT);
    pinMode(GSR_BACK_PIN,INPUT);
    pinMode(GSR_UP_PIN,INPUT);
    pinMode(GSR_DOWN_PIN,INPUT);
    if (GSR_PIN_USB_DET != 255) { pinMode(GSR_PIN_USB_DET,INPUT); pinMode(GSR_PIN_ADC,INPUT); }
}

uint16_t WatchyGSR::getDispCS()  { return (WatchyGSR::isESP32S3() ? 33 : 5 ); }
uint16_t WatchyGSR::getDispDC()  { return (WatchyGSR::isESP32S3() ? 34 : 10); }
uint16_t WatchyGSR::getDispRES() { return (WatchyGSR::isESP32S3() ? 35 : 9 ); }
uint16_t WatchyGSR::getDispBSY() { return (WatchyGSR::isESP32S3() ? 36 : 19); }

bool WatchyGSR::isESP32S3(){
    esp_chip_info_t chip_info[sizeof(esp_chip_info_t)];
    esp_chip_info(chip_info);
    return (chip_info->model == CHIP_ESP32S3);
}

// Obtains the SCL and SDA values before use (if not already present).
void WatchyGSR::startSetup(){
    if (GSR_PIN_SDA == 255){
        if(WatchyGSR::isESP32S3()){
            GSR_PIN_SCL = 11;
            GSR_PIN_SDA = 12;
            GSR_ESP_WAKEUP = 0;
            GSR_PIN_MISO = 46;
            GSR_PIN_SCK = 47;
            GSR_PIN_MOSI = 48;
            GSR_PIN_SS = 33;
        }else{
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
    if (i2cIsInit(0)) Wire.end();
    Wire.begin(GSR_PIN_SDA,GSR_PIN_SCL);
}

 // Sets up the pin definitions for the current Watchy hardware.
void WatchyGSR::getPins(float Version){
    uint8_t V = (Version * 10);
    GSR_MENU_PIN = 26;
    GSR_BACK_PIN = 25;
    GSR_UP_PIN = 32;
    GSR_DOWN_PIN = 4;
    GSR_PIN_ADC = 33;
    GSR_PIN_SCL = 22;
    GSR_PIN_SDA = 21;
    GSR_PIN_ACC_INT1 = 14;
    GSR_MASK_ACC_INT = ((uint64_t)(((uint64_t)1)<<14)); //GPIO_SEL_14;
    GSR_PIN_ACC_INT2 = 12;
    GSR_PIN_VIB_PWM = 13;
    switch (V){
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
    GSR_MENU_MASK = ((uint64_t)(((uint64_t)1)<<GSR_MENU_PIN));
    GSR_BACK_MASK = ((uint64_t)(((uint64_t)1)<<GSR_BACK_PIN));
    GSR_UP_MASK = ((uint64_t)(((uint64_t)1)<<GSR_UP_PIN));
    GSR_DOWN_MASK = ((uint64_t)(((uint64_t)1)<<GSR_DOWN_PIN));
    GSR_BTN_MASK = GSR_MENU_MASK | GSR_BACK_MASK | GSR_UP_MASK | GSR_DOWN_MASK;
}

void WatchyGSR::StartNTP(bool TimeSync, bool TimeZone){
  if (NTPData.State == 0 && !SRTC.checkingDrift(WatchTime.ESPRTC) && WeatherData.State < 2 && Battery.Read > Battery.RadioLevel){
    NTPData.TimeZone = (OP.getCurrentPOSIX() == OP.TZMISSING) || TimeZone;   // No Timezone, go get one!
    NTPData.UpdateUTC = TimeSync;
    if (NTPData.TimeZone || NTPData.UpdateUTC){
      NTPData.State = 1;
      NTPData.Pause = 2;
      AskForWiFi();
    }
  }
}

void WatchyGSR::ProcessNTP(){
  bool B;
  if (UpdateDisp || GSRWiFi.Slow > 0 && !inBrownOut()) return;
  // Do ProgressNTP here.
  switch (NTPData.State){
    // Start WiFi and Connect.
    case 1:{
      if (WiFiInProgress() && !GSRWiFi.Results){
          if(NTPData.Wait > 2){
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
    case 2:{
      if (WiFiStatus() != WL_CONNECTED){
          if(currentWiFi() == WL_CONNECT_FAILED || NTPData.Wait > 2){
              NTPData.Pause = 0;
              NTPData.State = 99;
              break;
          }
      }
      if (NTPData.TimeZone == false){
          NTPData.State = 5;
          NTPData.Pause = 0;
          break;
      }

      HTTP.setUserAgent(UserAgent);
      NTPData.State++;
      setStatus("TZ");
      // Do the next part.
      OP.beginOlsonFromWeb();
      NTPData.Wait = 0;
      NTPData.tWait = millis() + 4000;
      NTPData.Pause = 1;
      break;
  }

    case 3:{
      // Get GeoLocation
      if (WiFiStatus() == WL_DISCONNECTED){
          NTPData.Pause = 0;
          NTPData.State = 99;
          break;
      }
      if (OP.gotOlsonFromWeb()) {
        WatchTime.TimeZone = OP.getCurrentOlson();
        OP.endOlsonFromWeb();
        NTPData.Wait = 0;
        NTPData.Pause = 0;
        NTPData.State++;   // Test
      }else if (NTPData.tWait < millis()){
          NTPData.Pause = 0;
          NTPData.Wait = 0;
          NTPData.State = 5;
          OP.endOlsonFromWeb();
      }
      if(OP.getOlsonWebError()) LastWebError = OP.getOlsonWebError();
      break;
    }

    case 4:{
      // Process Timezone POSIX.
      String NewTZ = OP.getPOSIX(WatchTime.TimeZone);
      if(NewTZ == OP.TZMISSING){
          NTPData.Pause = 0;
          NTPData.State = 99;
          break;
      }
      if (OkNVS(GName)) { B = NVS.setString(GTZ,NewTZ); NVS.commit(); }
      OP.setCurrentPOSIX(NewTZ);
      NTPData.Wait = 0;
      NTPData.Pause = 0;
      NTPData.State++;
      if (!NTPData.UpdateUTC) UpdateDisp|=Showing();
      break;
    }

    case 5:{
    if (NTPData.UpdateUTC == false || WiFiStatus() == WL_DISCONNECTED || NTPData.Wait > 0){
          NTPData.State = 99;
          NTPData.Pause = 0;
          break;
      }
      setStatus("NTP");
      if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr) SNTP.Begin(InsertNTPServer()); else SNTP.Begin(WatchStyles.AddOn[Options.WatchFaceStyle]->InsertNTPServer());
      NTPData.Wait = 0;
      NTPData.tWait = millis() + 4000;
      NTPData.Pause = 01;
      NTPData.State++;
      break;
    }

    case 6:{
      if (!SNTP.Query()){
        if (NTPData.tWait < millis()){
            NTPData.Pause = 0;
            NTPData.State = 99;
        }
        break;
      }
      WatchTime.UTC_RAW = SNTP.Results;
      WatchTime.UTC = SNTP.tmResults;
      SRTC.set(WatchTime.UTC);
      NTPData.Pause = 0;
      NTPData.State = 99;
      UpdateClock();
      break;
    }

    case 99:{
      SNTP.End();
      endWiFi();
      NTPData.Wait = 0;
      NTPData.Pause = 0;
      NTPData.State = 0;
      NTPData.Last = WatchTime.UTC_RAW; // Moved from section 6 to here, to limit the atttempts.
      NTPData.UpdateUTC = false;
      NTPData.TimeZone = false;
      setStatus("");
    }
  }
}

void WatchyGSR::drawChargeMe(bool Dark){
  // Shows Battery Direction indicators.
  bool B = Dark;
  uint16_t C = (Dark ? GxEPD_WHITE : ForeColor());
  if (Design.Status.BatteryInverted && !B) C = BackColor();
  if (Battery.Direction == 1){
      display.drawBitmap(Design.Status.BATTx, Design.Status.BATTy, Charging, 40, 17, C);   // Show Battery charging bitmap.
  }else if (Battery.Read < Battery.MinLevel){
      display.drawBitmap(Design.Status.BATTx, Design.Status.BATTy, (Battery.Read < Battery.LowLevel ? ChargeMeBad : ChargeMe), 40, 17, C);   // Show Battery needs charging bitmap.
  }
}

void WatchyGSR::drawStatus(bool Dark){
  uint8_t X = Design.Status.WIFIx;
  uint16_t C = (Dark ? GxEPD_WHITE : (Design.Status.Inverted ? BackColor() : ForeColor()));
  bool Ok = true;
  display.setFont(&Bronova_Regular13pt7b);
  display.setTextColor(C);
  if (WatchyStatus > "" && !Dark){
      Ok = false;
      if (WatchyStatus.startsWith(WiFiTXT)){
          display.drawBitmap(X, Design.Status.WIFIy - 18, iWiFi, 19, 19, C);
          if (WatchyStatus.length() > 4){
              display.setCursor(X + 17, Design.Status.WIFIy);
              display.print(WatchyStatus.substring(4));
          }
      }
      else if (WatchyStatus == "TZ")  display.drawBitmap(X, Design.Status.WIFIy - 18, iTZ, 19, 19, C);
      else if (WatchyStatus == "NTP") display.drawBitmap(X, Design.Status.WIFIy - 18, iSync, 19, 19, C);
      else if (WatchyStatus == "GL")  display.drawBitmap(X, Design.Status.WIFIy - 18, iGeo, 19, 19, C);
      else if (WatchyStatus == "WE")  display.drawBitmap(X, Design.Status.WIFIy - 18, iWeather, 19, 19, C);
      else if (WatchyStatus == "ESP") display.drawBitmap(X, Design.Status.WIFIy - 18, iSync, 19, 19, C);
      else if (WatchyStatus == "NC")  display.drawBitmap(X, Design.Status.WIFIy - 18, iNoClock, 19, 19, C);
      else if (Alarms_Times[1] > 0 || Alarms_Times[2] > 0 || Alarms_Times[3] > 0 || Alarms_Times[4] > 0 || TimerDown.ToneLeft > 0) Ok = true;
      else{
          display.setCursor(X, Design.Status.WIFIy);
          display.print(WatchyStatus);
      }
  }

  if (Ok){
    if (TimerDown.ToneLeft > 0 && !Dark) { display.drawBitmap(X, Design.Status.WIFIy - 15, Tone_C, 12, 14, C); X += 13; Ok = false; }
    if (Alarms_Times[0] > 0 && !Dark) { display.drawBitmap(X, Design.Status.WIFIy - 15, Tone_1, 12, 14, C); X += 13; Ok = false; }
    if (Alarms_Times[1] > 0 && !Dark) { display.drawBitmap(X, Design.Status.WIFIy - 15, Tone_2, 12, 14, C); X += 13; Ok = false; }
    if (Alarms_Times[2] > 0 && !Dark) { display.drawBitmap(X, Design.Status.WIFIy - 15, Tone_3, 12, 14, C); X += 13; Ok = false; }
    if (Alarms_Times[3] > 0 && !Dark) { display.drawBitmap(X, Design.Status.WIFIy - 15, Tone_4, 12, 14, C); Ok = false; }
    if (Ok && Options.GameStatus) { display.drawBitmap(X, Design.Status.WIFIy - 18, GameIcon, 19, 18, C); Ok = false; }
    if (Ok && !Dark && WeatherData.Ready && GetWantWeather()) drawWeather(true);
  }
}

void WatchyGSR::setStatus(String Status){
    if (Status == "" && !SRTC.onESP32() && !SRTC.isOperating()) Status = "NC";
    if (GSRWiFi.Requests > 0){
        if (WatchyStatus.startsWith(WiFiTXT) && Status != "") WatchyOStatus = WatchyStatus;
        else if (WatchyOStatus.startsWith(WiFiTXT) && Status == "") Status = WatchyOStatus;
    } else if (WatchyOStatus != "") WatchyOStatus = "";
    if (WatchyStatus != Status){
      WatchyStatus = Status;
      UpdateDisp|=Showing();
    }
}

bool WatchyGSR::IsBatteryHidden() { return (Battery.State == 0); }

void WatchyGSR::VibeTo(bool Mode){
    if (Mode != VibeMode){
        if (Mode){
            pinMode(GSR_PIN_VIB_PWM, OUTPUT);
            digitalWrite(GSR_PIN_VIB_PWM, true);
        }else{
            digitalWrite(GSR_PIN_VIB_PWM, false);
            pinMode(GSR_PIN_VIB_PWM, INPUT);
        }
        VibeMode = Mode;
    }
}

void WatchyGSR::SoundBegin() { if (SoundHandle == NULL) { SoundRet = xTaskCreate(WatchyGSR::SoundAlarms,"WatchyGSR_Alarming",20480,NULL,(configMAX_PRIORITIES - 2),&SoundHandle); SoundStart = false; } }
bool WatchyGSR::SoundActive() { return (SoundHandle != NULL); }

void WatchyGSR::SoundAlarms(void * parameter){
    bool WaitForNext, Pulse, Used, Alarms;
    unsigned long M;
    uint8_t AlarmIndex = 0;
    WaitForNext=false;
    Pulse=false;
    AlarmReset=0;
    Used=true;
    vTaskDelay(10/portTICK_PERIOD_MS);
    Alarms=(getToneTimes(0) | getToneTimes(1) | getToneTimes(2) | getToneTimes(3) | getToneTimes(4));
                          // Here, do the alarm buzzings by way of which one is running.
    while (Used || Alarms){
        M = millis();
        Used = false;
        if (!OTAUpdate){
            if (WaitForNext){
                // Wait for the next second to change between any alarms still to play.
                if (M - AlarmReset > 999){ // Time changed.
                    AlarmReset = M;
                    WaitForNext = false;
                    Alarms = false; // Do this to make sure that the loop keeps running until this is ready.
                    AlarmIndex = roller(AlarmIndex + 1, 0, 4);
                    if (getToneTimes(AlarmIndex) == 0){
                        AlarmIndex = roller(AlarmIndex + 1, 0, 4);
                        if (getToneTimes(AlarmIndex) == 0){
                            AlarmIndex = roller(AlarmIndex + 1, 0, 4);
                            if (getToneTimes(AlarmIndex) == 0){
                                AlarmIndex = roller(AlarmIndex + 1, 0, 4);
                                if (getToneTimes(AlarmIndex) == 0){
                                    AlarmIndex = roller(AlarmIndex + 1, 0, 4);
                                }
                            }
                        }
                    }
                }
            }
            if(AlarmIndex == 4){
                if (TimerDown.ToneLeft > 0){
                    if (TimerDown.Tone == 0){
                        WaitForNext=true;
                        TimerDown.ToneLeft--;
                        if (TimerDown.ToneLeft > 0){
                            Used = true;
                            TimerDown.Tone = 22;
                        }else UpdateDisp=true;
                    }else{
                        Pulse = (((TimerDown.Tone / 2) & 1) != 0);
                        TimerDown.Tone--;
                        if (TimerDown.Tone == 0 && TimerDown.ToneLeft == 0) UpdateDisp=true;
                        Used = true;
                        if (!Pulse && TimerDown.Tone > 0) TimerDown.Tone--;
                        VibeTo(Pulse);   // Turns Vibe on or off depending on bit state.
                        if (Pulse) { Darkness.Last=M; HapticMS = 0; }
                    }
                }else WaitForNext=true;
            }else if (Alarms_Times[AlarmIndex] > 0){
                if (Alarms_Playing[AlarmIndex] > 0){
                    Alarms_Playing[AlarmIndex]--;
                    if (Menu.SubItem > 0 && Menu.Item - GSR_MENU_ALARM1 == AlarmIndex){
                        VibeTo(false);
                        Alarms_Playing[AlarmIndex]=0;
                        Alarms_Times[AlarmIndex]=0;
                        UpdateDisp=true;
                    }else{
                        Used = true;
                        Pulse = ((AlarmVBs[AlarmIndex] & Bits[Alarms_Playing[AlarmIndex] / 3]) != 0);
                        VibeTo(Pulse);   // Turns Vibe on or off depending on bit state.
                        if (Pulse) { Darkness.Last=M; HapticMS = 0; }
                    }
                    if (Alarms_Playing[AlarmIndex] == 0 && Alarms_Times[AlarmIndex] > 0){
                        Alarms_Times[AlarmIndex]--;   // Decrease count, eventually this will all stop on it's own.
                        WaitForNext = true;
                        if (Alarms_Times[AlarmIndex] > 0) { Darkness.Last=M; Alarms_Playing[AlarmIndex] = 30; Used = true; } else UpdateDisp=true;
                    }
                }else WaitForNext = true;
            }else WaitForNext = true;
        }
        AllowHaptic = !Used;
        if (Used) HapticMS = 0;
        M = (50-(millis() - M));
        M += millis();
        while (M > millis()) {
            if (AllowHaptic && HapticMS > 0){
              HapticMS--; VibeTo((HapticMS > 0));
            }
            vTaskDelay(10/portTICK_PERIOD_MS);
        }
        Used |= (HapticMS > 0);
    }
    AllowHaptic = true;
    SoundHandle = NULL;
    vTaskDelete(NULL);
}

void WatchyGSR::handleButtonPress(uint8_t Pressed){
  uint8_t I;
  int ml, mh;
  int32_t Diff;
  String S = "";
  if (Darkness.Went && Options.SleepStyle == 4 && !Updates.Tapped) return; // No buttons unless a tapped happened.
  if (!UpRight()) return; // Don't do buttons if not upright.
  if (Pressed < 5 && LastButton > 0 && (millis() - LastButton) < Debounce()) return;
  if (Darkness.Went && !Darkness.Woke) { Darkness.Woke=true; Darkness.Last=millis(); Darkness.Tilt = Darkness.Last; UpdateUTC(); UpdateClock(); UpdateDisp=true; return; }  // Don't do the button, just exit.
  if ((OTAUpdate) && (Pressed == 3 || Pressed == 4)) return;  // Up/Down don't work in these modes.

  switch (Pressed){
    case 1:
          if (GuiMode != GSR_MENUON && GuiMode != GSR_GAMEON && !GetMenuOverride()){ // If MenuOverride is on, it will not let the menu work, meaning, unless it is open already, it won't open.
            GuiMode = GSR_MENUON;
            DoHaptic = true;
            UpdateDisp = true;  // Quick Update.
            SetTurbo();
          }else if (GuiMode == GSR_MENUON){
              if (Menu.Item == GSR_MENU_OPTIONS && Menu.SubItem == 0){  // Options
                  Menu.Item = GSR_MENU_STYL;
                  Menu.Style = GSR_MENU_INOPTIONS;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_TRBL && Menu.SubItem == 0){  // Troubleshooting.
                  Menu.Style = GSR_MENU_INTROUBLE;
                  Menu.Item = GSR_MENU_SCRN;
                  DoHaptic = true;
                  UpdateDisp = true;
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_STEPS){ // Steps
                  if (Menu.SubItem == 4){
#ifdef STABLEBMA_H_INCLUDED
                      SBMA.resetStepCounter();
#endif
                      Steps.Cached = 0;
                      Menu.SubItem = 0;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }else if (Menu.SubItem < 4){
                      Menu.SubItem++;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }
              }else if (Menu.Item == GSR_MENU_ALARMS){  // Alarms menu.
                  Menu.Style = GSR_MENU_INALARMS;
                  Menu.Item = GSR_MENU_ALARM1;
                  DoHaptic = true;
                  UpdateDisp = true;
                  SetTurbo();
              }else if (Menu.Item >= GSR_MENU_ALARM1 && Menu.Item <= GSR_MENU_ALARM4){  // Alarms
                  if (Menu.SubItem < 5){
                      Menu.SubItem++;
                      if (Menu.SubItem == 5) Menu.SubItem += WatchTime.Local.Wday; // Jump ahead to the day.
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }else if (Menu.SubItem > 4 && Menu.SubItem < 12){
                      Alarms_Active[Menu.Item - GSR_MENU_ALARM1] ^= Bits[Menu.SubItem - 5];  // Toggle day.
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }else if (Menu.SubItem == 12){
                      Alarms_Active[Menu.Item - GSR_MENU_ALARM1] ^= GSR_ALARM_REPEAT;  // Toggle repeat.
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }else if (Menu.SubItem == 13){
                      Alarms_Active[Menu.Item - GSR_MENU_ALARM1] ^= GSR_ALARM_ACTIVE;  // Toggle Active.
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }
              }else if (Menu.Item == GSR_MENU_TONES){   // Tones.
                      Options.MasterRepeats = roller(Options.MasterRepeats + 1, 0, 4);
                      Alarms_Repeats[0] = Options.MasterRepeats;
                      Alarms_Repeats[1] = Options.MasterRepeats;
                      Alarms_Repeats[2] = Options.MasterRepeats;
                      Alarms_Repeats[3] = Options.MasterRepeats;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
              }else if (Menu.Item == GSR_MENU_TIMERS){  // Timers menu.
                  Menu.Style = GSR_MENU_INTIMERS;
                  Menu.Item = GSR_MENU_TIMEDN;
                  DoHaptic = true;
                  UpdateDisp = true;
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_TIMEDN){
                  if (Menu.SubItem == 6){
                      if (TimerDown.Active){
                          TimerDown.Active=false;
                          DoHaptic = true;
                          UpdateDisp = true;  // Quick Update.
                          SetTurbo();
                      }else if ((TimerDown.MaxSecs + TimerDown.MaxMins + TimerDown.MaxHours) > 0){
                          StartCD();
                          DoHaptic = true;
                          UpdateDisp = true;  // Quick Update.
                          SetTurbo();
                      }
                  }else{
                      Menu.SubItem++;
                      if (TimerDown.MaxSecs + TimerDown.MaxMins + TimerDown.MaxHours == 0 && Menu.SubItem == 6) Menu.SubItem = 4; //Stop it from being startable.
                      if (TimerDown.Active) Menu.SubItem = 6;
                      DoHaptic = true;
                      UpdateDisp = !TimerDown.Active;  // Quick Update.
                      SetTurbo();
                  }
              }else if (Menu.Item == GSR_MENU_TMEDCF){
                  if (Menu.SubItem < 2){
                      Menu.SubItem++;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }
              }else if (Menu.Item == GSR_MENU_TIMEUP){
                  if (Menu.SubItem == 0){
                      Menu.SubItem = 1;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }else{
                      if (TimerUp.Active){
                          UpdateUTC();
                          TimerUp.StopAt = WatchTime.UTC_RAW;
                          TimerUp.Active = false;
                      }else{
                          UpdateUTC();
                          TimerUp.SetAt = WatchTime.UTC_RAW;
                          TimerUp.StopAt = TimerUp.SetAt;
                          TimerUp.Active = true;
                      }
                      DoHaptic = true;
                      UpdateDisp = !TimerUp.Active;  // Quick Update.
                      SetTurbo();
                  }
              }else if (Menu.Item == GSR_MENU_GAME){
                  if (Menu.SubItem == 0){
                      Menu.SubItem++;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }else {
                      ShowGame();
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }
              }else if (Menu.Item == GSR_MENU_TPWR && !(GSRWiFi.Requests > 0 || WatchyAPOn)){
                  I = roller(getTXOffset(GSRWiFi.TransmitPower) + 1,0,10);
                  GSRWiFi.TransmitPower = RawWiFiTX[I];
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_INFO){ // Information
                  Menu.SubItem = roller(Menu.SubItem + 1, 0, 2);
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_SAVE){  // Battery Saver.
                  Options.Performance = roller(Options.Performance + 1,0,2);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_SYNC){  // Sync Time
                  if (Menu.SubItem == 0 || Menu.SubItem > 3){  // Start and Sync Menu
                      if (Menu.SubItem < 15){
                          Menu.SubItem++;
                          DoHaptic = true;
                          UpdateDisp = true;  // Quick Update.
                          SetTurbo();
                      }
                  }else{
                      switch (Menu.SubItem){
                        case 1:
                          NTPData.UpdateUTC = true;
                          NTPData.TimeZone = (OP.getCurrentPOSIX() == OP.TZMISSING);   // No Timezone, go get one!
                          break;
                        case 2:
                          NTPData.UpdateUTC = false;
                          NTPData.TimeZone = true;
                          break;
                        case 3:
                          NTPData.TimeZone = true;
                          NTPData.UpdateUTC = true;
                      }
                      GuiMode = GSR_WATCHON;
                      Menu.Item = GSR_MENU_STYL;
                      Menu.SubItem = 0;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      StartNTP(NTPData.UpdateUTC, NTPData.TimeZone);
                      SetTurbo();
                  }
              }else if (Menu.Item == GSR_MENU_WEAT){  // Weather
                  if (Menu.SubItem < 4){
                      if (Menu.SubItem == 0 || (WeatherData.Interval == 0 && Menu.SubItem < 2)) Menu.SubItem = 2; else Menu.SubItem++;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }else if (Menu.SubItem == 4){
                      StartWeather();
                      Menu.SubItem = 0;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }
              }else if (Menu.Item == GSR_MENU_STYL){  // Switch Watch Face
                  ChangeWatchFace();  /* Defaults to TRUE to go up */
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_LANG){  // Language switch.
                  I = roller(Options.LanguageID + 1,0,LGSR.MaxLangID());
                  if (I != Options.LanguageID){
                      Options.LanguageID = I;
                      initWatchFaceStyle();  // If fonts will need to be changed per language down the road, I don't know.
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }
              }else if (Menu.Item == GSR_MENU_DISP){  // Switch Mode
                  Options.LightMode = !Options.LightMode;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_SIDE){  // Dexterity Mode
                  Options.Lefty = !Options.Lefty;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_SWAP){  // Swap Menu/Back Buttons
                  Options.Swapped = !Options.Swapped;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_BRDR){  // Border Mode
#ifdef GxEPD2DarkBorder
                  Options.Border = !Options.Border;
                  Options.NeedsSaving = true;
                  Updates.Init = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
#endif
              }else if (Menu.Item == GSR_MENU_ORNT){  // Watchy Orientation
                  Options.Orientated = !Options.Orientated;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_MODE){  // Switch Time Mode
                  Options.TwentyFour = !Options.TwentyFour;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_FEED){  // Feedback.
                  Options.Feedback = !Options.Feedback;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_TRBO){  // Turbo
                  Options.Turbo = roller(Options.Turbo + 1, 0, 10);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_DARK){  // Sleep Mode.
                  if (Menu.SubItem < 5){
                      Menu.SubItem++;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }
              }else if (Menu.Item == GSR_MENU_SCRN){  // Reset Screen
                  GuiMode = GSR_WATCHON;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  Updates.Full = true;
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_WIFI && !WatchyAPOn){  // Watchy Connect
                  Menu.SubItem++;
                  WatchyAPOn = true;
                  ResetEndOTA();
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if ((Menu.Item == GSR_MENU_OTAU || Menu.Item == GSR_MENU_OTAM) && !(GSRWiFi.Requests > 0 || WatchyAPOn)){  // Watchy OTA
                  Menu.SubItem++;
                  OTAUpdate=true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_RSET){  // Watchy Reboot
                  if (Menu.SubItem == 1) Reboot(); else Menu.SubItem++;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.Item == GSR_MENU_TOFF && NTPData.State == 0){  // Drift Management
                  /*
                   * S H:MM:SS Begin/Calculate/Current/Toggle RTC/Adjust Drift
                   * 0 1 23 45 6     7         8       9             11
                   */
                  /* DO SETUP FOR Edit RTC MENU */
                  if (Menu.SubItem < 6){
                      Menu.SubItem++;
                      if (Menu.SubItem == 6){
                        if (SRTC.checkingDrift(WatchTime.ESPRTC)) Menu.SubItem++; // Move to Calculate.
                        else if (SRTC.getDrift(WatchTime.ESPRTC) != 0) Menu.SubItem = 8; // Move to show the value (so you don't ruin it).
                      }
                      DoHaptic = true;
                      UpdateDisp = (Menu.SubItem > 5);  // Quick Update. */
                      SetTurbo();
                  }else if (Menu.SubItem == 6){ // Drift Begin
                      UpdateUTC(); SRTC.setDrift(0,false,WatchTime.ESPRTC);
                      WatchTime.UTC_RAW += NTPData.TravelTemp;
                      NTPData.TravelTemp = 0;
                      SRTC.doBreakTime(WatchTime.UTC_RAW, WatchTime.UTC);
                      SRTC.beginDrift(WatchTime.UTC,WatchTime.ESPRTC);
                      UpdateUTC();
                      UpdateClock();
                      Menu.SubItem = 0;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }else if (Menu.SubItem == 7){ // Drift Finish
                      UpdateUTC(); Diff = 0;
                      WatchTime.UTC_RAW += NTPData.TravelTemp;
                      SRTC.doBreakTime(WatchTime.UTC_RAW, WatchTime.UTC);
                      SRTC.endDrift(WatchTime.UTC,WatchTime.ESPRTC);
                      UpdateUTC();
                      UpdateClock();
                      Options.NeedsSaving = true;
                      NTPData.TravelTemp = 0;
                      Menu.SubItem = 0;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }else if (Menu.SubItem == 8){ // Drift Value pushing Menu to edit.
                      Menu.SubItem = 11;
                      Menu.SubSubItem = 0;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }else if (Menu.SubItem == 9 && HWVer != 3.0f){
                      WatchTime.ESPRTC = !WatchTime.ESPRTC;
                      SRTC.useESP32(WatchTime.ESPRTC);
                      UpdateUTC();
                      UpdateClock();
                      Options.NeedsSaving = true;
                      Menu.SubItem = 0;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }else if (Menu.SubItem == 11){ // Drift Value MENU to advance to next digit.
                      S = String(SRTC.getDrift(WatchTime.ESPRTC));
                      if (Menu.SubSubItem + 1 < S.length()){
                          Menu.SubSubItem++;
                          DoHaptic = true;
                          UpdateDisp = true;  // Quick Update.
                          SetTurbo();
                      }
                  }
              }else if (Menu.Item == GSR_MENU_UNVS){  // USE NVS
                  if (Menu.SubItem == 0) Menu.SubItem++;
                  else if (Menu.SubItem == 1){
                      if (Menu.SubSubItem == 1){
                          if (OkNVS(GName)) Menu.SubItem == 2;  // delete, request
                          else { SetNVS(GName,true); Menu.SubItem = 0; Menu.SubSubItem = 0; }
                      }else Menu.SubItem = 0; // Keep, don't change.
                  }else if (Menu.SubItem == 2){
                      NVSEmpty();
                      Reboot();
                  }
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          } else Missed = 1;
          break;
    case 2:
      if (GuiMode == GSR_MENUON){   // Back Button [SW2]
          if (Menu.Item == GSR_MENU_STEPS && Menu.SubItem > 0) {  // Exit for Steps, back to Steps.
              if (Menu.SubItem == 4) Menu.SubItem = 2;  // Go back to the Hour, so it is the same as the alarms.
              Menu.SubItem--;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else if (Menu.Item >= GSR_MENU_ALARM1 && Menu.Item <= GSR_MENU_ALARM4 && Menu.SubItem > 0){
              if (Menu.SubItem < 5 && Menu.SubItem > 0){
                  Menu.SubItem--;
              }else if (Menu.SubItem > 4){
                  Menu.SubItem = 1;
              }
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else if (Menu.Item == GSR_MENU_TIMEDN && Menu.SubItem > 0){
              Menu.SubItem--;
              if (TimerDown.Active) Menu.SubItem = 0;
              DoHaptic = true;
              UpdateDisp = !TimerDown.Active;  // Quick Update.
              SetTurbo();
          }else if (Menu.Item == GSR_MENU_TMEDCF && Menu.SubItem > 0){
              Menu.SubItem--;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else if (Menu.Item == GSR_MENU_GAME && Menu.SubItem > 0){
              Menu.SubItem--;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else if (Menu.Item == GSR_MENU_TIMEUP && Menu.SubItem > 0){
              Menu.SubItem = 0;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else if (Menu.Item == GSR_MENU_GAME && Menu.SubItem > 0){
              Menu.SubItem--;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
        }else if (Menu.Item == GSR_MENU_DARK && Menu.SubItem > 0){  // Sleep Mode.
              Menu.SubItem--;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else if (Menu.Item == GSR_MENU_SYNC && Menu.SubItem > 0){
              if (Menu.SubItem == 5) Menu.SubItem = 4;
              else if (Menu.SubItem > 4 && Menu.SubItem < 9) Menu.SubItem = 5;
              else if (Menu.SubItem > 8) Menu.SubItem--;
              else Menu.SubItem = 0;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else if (Menu.Item == GSR_MENU_WEAT && Menu.SubItem > 0){  // Weather
              Menu.SubItem--;
              if (Menu.SubItem == 1 && ((WeatherData.Interval * 300)/3600) == 0) Menu.SubItem--;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else if (Menu.Item == GSR_MENU_WIFI && Menu.SubItem > 0){
              if (Menu.SubItem < 5){
                  EndOTA(3);
                  Menu.SubItem = 5;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if ((Menu.Item == GSR_MENU_OTAU || Menu.Item == GSR_MENU_OTAM) && Menu.SubItem > 0){
              break;    // DO NOTHING!
          }else if (Menu.Style == GSR_MENU_INALARMS){  // Alarms
              Menu.Style = GSR_MENU_INNORMAL;
              Menu.Item = GSR_MENU_ALARMS;
              DoHaptic = true;
              UpdateDisp = true;
              SetTurbo();
          }else if (Menu.Style == GSR_MENU_INTIMERS){  // Timers
              Menu.Style = GSR_MENU_INNORMAL;
              Menu.Item = GSR_MENU_TIMERS;
              DoHaptic = true;
              UpdateDisp = true;          
              SetTurbo();
          }else if (Menu.Style == GSR_MENU_INOPTIONS){  // Options
              Menu.SubItem = 0;
              Menu.SubSubItem = 0;
              Menu.Item=GSR_MENU_OPTIONS;
              Menu.Style=GSR_MENU_INNORMAL;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else if (Menu.Style == GSR_MENU_INTROUBLE && Menu.SubItem == 0){  // Troubleshooting.
              Menu.SubItem = 0;
              Menu.SubSubItem = 0;
              Menu.Item=GSR_MENU_TRBL;
              Menu.Style=GSR_MENU_INOPTIONS;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else if (Menu.Item == GSR_MENU_RSET && Menu.SubItem > 0){  // Watchy Reboot
              Menu.SubItem--;
          }else if (Menu.Item == GSR_MENU_TOFF && Menu.SubItem > 0){  // Drift Travel
              /*
               * S H:MM:SS Begin/Calculate/Current/Toggle RTC/Adjust Drift
               * 0 1 23 45 6     7         8       9             11
               */
              /* DO SETUP FOR Edit RTC BACK */
              if (Menu.SubItem == 11){ // Drift Value pushing Back to exit.
                  if (Menu.SubSubItem == 0) Menu.SubItem = 8;
                  else Menu.SubSubItem--;
              }else if (Menu.SubItem > 5) Menu.SubItem = 5;
              else Menu.SubItem--;
              UpdateDisp = true;  // Quick Update.
              DoHaptic = true;
              SetTurbo();
          }else if (Menu.Item == GSR_MENU_UNVS){  // USE NVS
              Menu.SubItem = 0;
              Menu.SubSubItem = 0;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else{
              GuiMode = GSR_WATCHON;
              Menu.SubItem = 0;
              Menu.SubSubItem = 0;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }
      } else Missed = 2;  // Missed a SW2.
      break;
    case 3:
      if (GuiMode == GSR_MENUON){     // Up Button [SW3]
          // Handle the sideways choices here.
          if (Menu.Item == GSR_MENU_STEPS && Menu.SubItem > 0){
              switch (Menu.SubItem){
              case 1: // Hour
                  Steps.Hour=roller(Steps.Hour + 1, 0,23);
                  Options.NeedsSaving = true;
                  Steps.Reset = false;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
                  break;
              case 2: //  x0 Minutes
                  mh = (Steps.Minutes / 10);
                  ml = Steps.Minutes - (mh * 10);
                  mh = roller(mh + 1, 0, 5);
                  Steps.Minutes = (mh * 10) + ml;
                  Steps.Reset = false;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
                  break;
              case 3: //  x0 Minutes
                  mh = (Steps.Minutes / 10);
                  ml = Steps.Minutes - (mh * 10);
                  ml = roller(ml + 1, 0, 9);
                  Steps.Minutes = (mh * 10) + ml;
                  Steps.Reset = false;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item >= GSR_MENU_ALARM1 && Menu.Item <= GSR_MENU_ALARM4 && Menu.SubItem > 0){
              if (Menu.SubItem == 1){ // Hour
                  Alarms_Hour[Menu.Item - GSR_MENU_ALARM1]=roller(Alarms_Hour[Menu.Item - GSR_MENU_ALARM1] + 1, 0,23);
                  Alarms_Active[Menu.Item - GSR_MENU_ALARM1] &= GSR_ALARM_NOTRIGGER;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.SubItem == 2){ //  x0 Minutes
                  mh = (Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1] / 10);
                  ml = Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1] - (mh * 10);
                  mh = roller(mh + 1, 0, 5);
                  Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1] = (mh * 10) + ml;
                  Alarms_Active[Menu.Item - GSR_MENU_ALARM1] &= GSR_ALARM_NOTRIGGER;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.SubItem == 3){ //  x0 Minutes
                  mh = (Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1] / 10);
                  ml = Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1] - (mh * 10);
                  ml = roller(ml + 1, 0, 9);
                  Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1] = (mh * 10) + ml;
                  Alarms_Active[Menu.Item - GSR_MENU_ALARM1] &= GSR_ALARM_NOTRIGGER;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.SubItem == 4){ //  Repeats.
                  Alarms_Repeats[Menu.Item - GSR_MENU_ALARM1] = roller(Alarms_Repeats[Menu.Item - GSR_MENU_ALARM1] - 1, 0, 4);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.SubItem > 4){
                  Menu.SubItem = roller(Menu.SubItem + 1, 5, 13);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          } else if (Menu.Item == GSR_MENU_TIMEDN && Menu.SubItem > 0){
              switch (Menu.SubItem){
              case 1: // Hour
                  TimerDown.MaxHours=roller(TimerDown.MaxHours + 1, 0,23);
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
                  break;
              case 2: //  x0 Minutes
                  mh = (TimerDown.MaxMins / 10);
                  ml = TimerDown.MaxMins - (mh * 10);
                  mh = roller(mh + 1, 0, 5);
                  TimerDown.MaxMins = (mh * 10) + ml;
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !TimerDown.Active;  // Quick Update.
                  SetTurbo();
                  break;
              case 3: //  x0 Minutes
                  mh = (TimerDown.MaxMins / 10);
                  ml = TimerDown.MaxMins - (mh * 10);
                  ml = roller(ml + 1, 0, 9);
                  TimerDown.MaxMins = (mh * 10) + ml;
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !TimerDown.Active;  // Quick Update.
                  SetTurbo();
                  break;
              case 4: //  x0 Seconds
                  mh = (TimerDown.MaxSecs / 10);
                  ml = TimerDown.MaxSecs - (mh * 10);
                  mh = roller(mh + 1, 0, 5);
                  TimerDown.MaxSecs = (mh * 10) + ml;
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !TimerDown.Active;  // Quick Update.
                  SetTurbo();
                  break;
              case 5: //  x0 Seconds
                  mh = (TimerDown.MaxSecs / 10);
                  ml = TimerDown.MaxSecs - (mh * 10);
                  ml = roller(ml + 1, 0, 9);
                  TimerDown.MaxSecs = (mh * 10) + ml;
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !TimerDown.Active;  // Quick Update.
                  SetTurbo();
              }
          } else if (Menu.Item == GSR_MENU_TMEDCF && Menu.SubItem > 0){
              switch (Menu.SubItem){
              case 1:   // Repeatable
                  TimerDown.Repeats = !TimerDown.Repeats;
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
                  break;
              case 2:   // Tone Repeats
                  TimerDown.MaxTones = roller(TimerDown.MaxTones - 1, 0, 4);
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item == GSR_MENU_GAME && Menu.SubItem > 0){
              if (ChangeGame()){
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item == GSR_MENU_DARK && Menu.SubItem > 0){  // Sleep Mode.
              switch (Menu.SubItem){
                  case 1: // Style.
#ifdef STABLEBMA_H_INCLUDED
                      if (HWVer = 3.0f){
                          if (Options.SleepStyle != 2) Options.SleepStyle = 2;
                          else Options.SleepStyle = 0;
                      }else Options.SleepStyle = roller(Options.SleepStyle + 1, 0, 4);
#else
                      if (Options.SleepStyle != 2) Options.SleepStyle = 2;
                      else Options.SleepStyle = 0;
#endif
                      Updates.Tapped = true;
                      Options.NeedsSaving = true;
                      break;
                  case 2: // SleepMode (0=off, 10 seconds)
                      Options.SleepMode = roller(Options.SleepMode + 1, 1, 10);
                      Options.NeedsSaving = true;
                      break;
                  case 3: // SleepStart (hour)
                      if (Options.SleepStart + 1 != Options.SleepEnd) { Options.SleepStart = roller(Options.SleepStart + 1, 0,23); Options.NeedsSaving = true; } else return;
                      break;
                  case 4: // SleepStart (hour)
                      if (Options.SleepEnd + 1 != Options.SleepStart) { Options.SleepEnd = roller(Options.SleepEnd + 1, 0,23); Options.NeedsSaving = true; } else return;
                      break;
                  case 5: // Orientation.
                      Options.BedTimeOrientation = !Options.BedTimeOrientation;
                      Options.NeedsSaving = true;
              }
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else if (Menu.Item == GSR_MENU_SYNC && Menu.SubItem > 0){
              if (Menu.SubItem > 4){
                /* On HH:MM
                   5  67 89 */

                  switch (Menu.SubItem){
                  case 5: // On/Off
                      NTPData.AutoSync = !NTPData.AutoSync;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                      break;
                  case 6: // Hour
                      NTPData.SyncHour=roller(NTPData.SyncHour + 1, 0,23);
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                      break;
                  case 7: //  x0 Minutes
                      mh = (NTPData.SyncMins / 10);
                      ml = NTPData.SyncMins - (mh * 10);
                      mh = roller(mh + 1, 0, 5);
                      NTPData.SyncMins = (mh * 10) + ml;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                      break;
                  case 8: //  x0 Minutes
                      mh = (NTPData.SyncMins / 10);
                      ml = NTPData.SyncMins - (mh * 10);
                      ml = roller(ml + 1, 0, 9);
                      NTPData.SyncMins = (mh * 10) + ml;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                      break;
                  case 9:  // Sunday.
                  case 10: // Monday.
                  case 11: // Tuesday.
                  case 12: // Wednesday.
                  case 13: // Thursday.
                  case 14: // Friday.
                  case 15: // Saturday.
                      I = NTPData.SyncDays ^ Bits[Menu.SubItem - 9];
                      if (I){
                          NTPData.SyncDays = I;
                          Options.NeedsSaving = true;
                          DoHaptic = true;
                          UpdateDisp = true;  // Quick Update.
                          SetTurbo();
                      }
                  }
              }
              else {
                  Menu.SubItem = roller(Menu.SubItem - 1, 1, 4);
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item == GSR_MENU_WEAT && Menu.SubItem > 0){  // Weather
              if (Menu.SubItem == 3) SetWeatherScale(!IsMetric());
              else if (Menu.SubItem < 3){
                  I = (Menu.SubItem == 1 ? 12 : 1);
                  WeatherData.Interval = roller(WeatherData.Interval + I, 0, 144);
                  WeatherData.LastCall = WatchTime.UTC_RAW - (WatchTime.UTC_RAW % 60) + (WeatherData.Interval * 300); // 15 minutes.
              }
              if (Menu.SubItem < 4){
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item == GSR_MENU_WIFI && Menu.SubItem > 0){
              // Do nothing!
          }else if (Menu.Item == GSR_MENU_TOFF && Menu.SubItem > 0){
              /*
               * S H:MM:SS Begin/Calculate/Current/Toggle RTC/Adjust Drift
               * 0 1 23 45 6     7         8       9             11
               */
              /* DO SETUP FOR Edit RTC UP */
              if (Menu.SubItem < 6){
                  NTPData.TravelTemp += ((Menu.SubItem == 1 ? 3600 : 0) + (Menu.SubItem == 2 ? 600 : 0) + (Menu.SubItem == 3 ? 60 : 0) + (Menu.SubItem == 4 ? 10 : 0) + (Menu.SubItem == 5 ? 1 : 0));
                  DoHaptic = true;
                  /* TEST  UpdateDisp = true;  // Quick Update.*/
                  SetTurbo();
              }else if (Menu.SubItem == 11){ // Drift Value edit..
                  Diff = SRTC.getDrift(WatchTime.ESPRTC); S = String(Diff);
                  Diff += pow(10,(S.length() - (1 + Menu.SubSubItem)));
                  if (Diff < 0xFFFFFFFF){
                      SRTC.setDrift(Diff, SRTC.isFastDrift(WatchTime.ESPRTC), WatchTime.ESPRTC);
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }
              }else{
                  Menu.SubItem = roller(Menu.SubItem + 1,6,9); if (Menu.SubItem == 7 && !SRTC.checkingDrift(WatchTime.ESPRTC)) Menu.SubItem++;
                  if (Menu.SubItem == 8 && SRTC.getDrift(WatchTime.ESPRTC) == 0) Menu.SubItem++;
                  //if (WatchTime.ESPRTC) Menu.SubItem == 9;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item == GSR_MENU_UNVS && Menu.SubItem == 1){  // USE NVS
              if (Menu.SubSubItem == 1){
                  Menu.SubSubItem = 0;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
              return;
          }else{
              if (Menu.Style == GSR_MENU_INOPTIONS){
                  Menu.Item = roller(Menu.Item - 1, GSR_MENU_STYL, (Watchy_Chip_Info.HasWiFi & (NTPData.State > 0 || WatchyAPOn || OTAUpdate || Battery.Read < Battery.RadioLevel)) ? GSR_MENU_TRBL : GSR_MENU_OTAM);
                  if (Menu.Item == GSR_MENU_WEAT && !GetWantWeather()) Menu.Item = roller(Menu.Item - 1, GSR_MENU_STYL, (Watchy_Chip_Info.HasWiFi & (NTPData.State > 0 || WatchyAPOn || OTAUpdate || Battery.Read < Battery.RadioLevel)) ? GSR_MENU_TRBL : GSR_MENU_OTAM);
              }else if (Menu.Style == GSR_MENU_INALARMS){
                  Menu.Item = roller(Menu.Item - 1, GSR_MENU_ALARM1, GSR_MENU_TONES);
              }else if (Menu.Style == GSR_MENU_INTIMERS){
                  Menu.Item = roller(Menu.Item - 1, GSR_MENU_TIMEDN, GSR_MENU_TIMEUP);
              }else if (Menu.Style == GSR_MENU_INTROUBLE){
                  Menu.Item = roller(Menu.Item - 1, GSR_MENU_SCRN, GSR_MENU_UNVS);
              }else{
#ifndef STABLEBMA_H_INCLUDED
                  Menu.Item = roller(Menu.Item - 1, GSR_MENU_ALARMS, GSR_MENU_OPTIONS);
                  if (Menu.Item == GSR_MENU_GAME && Options.Game == 255) Menu.Item = roller(Menu.Item - 1, GSR_MENU_ALARMS, GSR_MENU_OPTIONS);
#else
                  Menu.Item = roller(Menu.Item - 1, GSR_MENU_STEPS, GSR_MENU_OPTIONS);
                  if (Menu.Item == GSR_MENU_GAME && Options.Game == 255) Menu.Item = roller(Menu.Item - 1, GSR_MENU_STEPS, GSR_MENU_OPTIONS);
#endif
              }
              Menu.SubItem=0;
              Menu.SubSubItem=0;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }
      } else Missed = 3;  // Missed a SW3.
      break;
    case 4:
      if (GuiMode == GSR_MENUON){   // Down Button [SW4]
          // Handle the sideways choices here.
          if (Menu.Item == GSR_MENU_STEPS && Menu.SubItem > 0){
              switch (Menu.SubItem){
              case 1: // Hour
                  Steps.Hour=roller(Steps.Hour - 1, 0,23);
                  Options.NeedsSaving = true;
                  Steps.Reset = false;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
                  break;
              case 2: //  x0 Minutes
                  mh = (Steps.Minutes / 10);
                  ml = Steps.Minutes - (mh * 10);
                  mh = roller(mh - 1, 0, 5);
                  Steps.Minutes = (mh * 10) + ml;
                  Steps.Reset = false;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
                  break;
              case 3: //  x0 Minutes
                  mh = (Steps.Minutes / 10);
                  ml = Steps.Minutes - (mh * 10);
                  ml = roller(ml - 1, 0, 9);
                  Steps.Minutes = (mh * 10) + ml;
                  Steps.Reset = false;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item >= GSR_MENU_ALARM1 && Menu.Item <= GSR_MENU_ALARM4 && Menu.SubItem > 0){
              if (Menu.SubItem == 1){ // Hour
                  Alarms_Hour[Menu.Item - GSR_MENU_ALARM1]=roller(Alarms_Hour[Menu.Item - GSR_MENU_ALARM1] - 1, 0,23);
                  Alarms_Active[Menu.Item - GSR_MENU_ALARM1] &= GSR_ALARM_NOTRIGGER;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
             }else if (Menu.SubItem == 2){ //  x0 Minutes
                  mh = (Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1] / 10);
                  ml = Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1] - (mh * 10);
                  mh = roller(mh - 1, 0, 5);
                  Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1] = (mh * 10) + ml;
                  Alarms_Active[Menu.Item - GSR_MENU_ALARM1] &= GSR_ALARM_NOTRIGGER;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.SubItem == 3){ //  x0 Minutes
                  mh = (Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1] / 10);
                  ml = Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1] - (mh * 10);
                  ml = roller(ml - 1, 0, 9);
                  Alarms_Minutes[Menu.Item - GSR_MENU_ALARM1] = (mh * 10) + ml;
                  Alarms_Active[Menu.Item - GSR_MENU_ALARM1] &= GSR_ALARM_NOTRIGGER;
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.SubItem == 4){ //  Repeats.
                  Alarms_Repeats[Menu.Item - GSR_MENU_ALARM1] = roller(Alarms_Repeats[Menu.Item - GSR_MENU_ALARM1] + 1, 0, 4);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }else if (Menu.SubItem > 4){
                  Menu.SubItem = roller(Menu.SubItem - 1, 5, 13);
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          } else if (Menu.Item == GSR_MENU_TIMEDN && Menu.SubItem > 0){
              switch (Menu.SubItem){
              case 1: // Hour
                  TimerDown.MaxHours=roller(TimerDown.MaxHours - 1, 0,23);
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
                  break;
              case 2: //  x0 Minutes
                  mh = (TimerDown.MaxMins / 10);
                  ml = TimerDown.MaxMins - (mh * 10);
                  mh = roller(mh - 1, 0, 5);
                  TimerDown.MaxMins = (mh * 10) + ml;
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !TimerDown.Active;  // Quick Update.
                  SetTurbo();
                  break;
              case 3: //  x0 Minutes
                  mh = (TimerDown.MaxMins / 10);
                  ml = TimerDown.MaxMins - (mh * 10);
                  ml = roller(ml - 1, 0, 9);
                  TimerDown.MaxMins = (mh * 10) + ml;
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !TimerDown.Active;  // Quick Update.
                  SetTurbo();
                  break;
              case 4: //  x0 Seconds
                  mh = (TimerDown.MaxSecs / 10);
                  ml = TimerDown.MaxSecs - (mh * 10);
                  mh = roller(mh - 1, 0, 5);
                  TimerDown.MaxSecs = (mh * 10) + ml;
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !TimerDown.Active;  // Quick Update.
                  SetTurbo();
                  break;
              case 5: //  x0 Secondss
                  mh = (TimerDown.MaxSecs / 10);
                  ml = TimerDown.MaxSecs - (mh * 10);
                  ml = roller(ml - 1, 0, 9);
                  TimerDown.MaxSecs = (mh * 10) + ml;
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = !TimerDown.Active;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item == GSR_MENU_TMEDCF && Menu.SubItem > 0){
              switch (Menu.SubItem){
              case 1:   // Repeatable
                  TimerDown.Repeats = !TimerDown.Repeats;
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
                  break;
              case 2:   // Tone Repeats
                  TimerDown.MaxTones = roller(TimerDown.MaxTones + 1, 0, 4);
                  StopCD();
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item == GSR_MENU_GAME && Menu.SubItem > 0){
              if (ChangeGame(false)){
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item == GSR_MENU_DARK && Menu.SubItem > 0){  // Sleep Mode.
              switch (Menu.SubItem){
                  case 1: // Style.
#ifdef STABLEBMA_H_INCLUDED
                      if (HWVer == 3.0f){
                      if (Options.SleepStyle != 2) Options.SleepStyle = 2;
                      else Options.SleepStyle = 0;
                      }else Options.SleepStyle = roller(Options.SleepStyle - 1, 0, 4);
#else
                      if (Options.SleepStyle != 2) Options.SleepStyle = 2;
                      else Options.SleepStyle = 0;
#endif
                      Updates.Tapped = true;
                      Options.NeedsSaving = true;
                      break;
                  case 2: // SleepMode (0=off, 10 seconds)
                      Options.SleepMode = roller(Options.SleepMode - 1, 1, 10);
                      Options.NeedsSaving = true;
                      break;
                  case 3: // SleepStart (hour)
                      if (Options.SleepStart - 1 != Options.SleepEnd) { Options.SleepStart = roller(Options.SleepStart - 1, 0,23); Options.NeedsSaving = true; } else return;
                      break;
                  case 4: // SleepStart (hour)
                      if (Options.SleepEnd - 1 != Options.SleepStart) { Options.SleepEnd = roller(Options.SleepEnd - 1, 0,23); Options.NeedsSaving = true; } else return;
                      break;
                  case 5: // Orientation.
                      Options.BedTimeOrientation = !Options.BedTimeOrientation;
                      Options.NeedsSaving = true;
              }
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }else if (Menu.Item == GSR_MENU_SYNC && Menu.SubItem > 0){
              if (Menu.SubItem > 4){
                /* On HH:MM
                   5  67 89 */
                  switch (Menu.SubItem){
                  case 5: // On/Off
                      NTPData.AutoSync = !NTPData.AutoSync;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                      break;
                  case 6: // Hour
                      NTPData.SyncHour=roller(NTPData.SyncHour - 1, 0,23);
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                      break;
                  case 7: //  x0 Minutes
                      mh = (NTPData.SyncMins / 10);
                      ml = NTPData.SyncMins - (mh * 10);
                      mh = roller(mh - 1, 0, 5);
                      NTPData.SyncMins = (mh * 10) + ml;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                      break;
                  case 8: //  x0 Minutes
                      mh = (NTPData.SyncMins / 10);
                      ml = NTPData.SyncMins - (mh * 10);
                      ml = roller(ml - 1, 0, 9);
                      NTPData.SyncMins = (mh * 10) + ml;
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                      break;
                  case 9:  // Sunday.
                  case 10: // Monday.
                  case 11: // Tuesday.
                  case 12: // Wednesday.
                  case 13: // Thursday.
                  case 14: // Friday.
                  case 15: // Saturday.
                      I = NTPData.SyncDays ^ Bits[Menu.SubItem - 9];
                      if (I){
                          NTPData.SyncDays = I;
                          Options.NeedsSaving = true;
                          DoHaptic = true;
                          UpdateDisp = true;  // Quick Update.
                          SetTurbo();
                      }
                  }
              }
              else {
                  Menu.SubItem = roller(Menu.SubItem + 1, 1, 4);
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item == GSR_MENU_WEAT && Menu.SubItem > 0){  // Weather
              if (Menu.SubItem == 3) SetWeatherScale(!IsMetric());
              else if (Menu.SubItem < 3){
                  I = (Menu.SubItem == 1 ? 12 : 1);
                  WeatherData.Interval = roller(WeatherData.Interval - I, 0, 144);
                  WeatherData.LastCall = WatchTime.UTC_RAW - (WatchTime.UTC_RAW % 60) + (WeatherData.Interval * 300); // 15 minutes.
              }
              if (Menu.SubItem < 4){
                  Options.NeedsSaving = true;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item == GSR_MENU_WIFI && Menu.SubItem > 0){
              // Do nothing!
          }else if (Menu.Item == GSR_MENU_TOFF && Menu.SubItem > 0){
              /*
               * S H:MM:SS Begin/Calculate/Current/Toggle RTC/Adjust Drift
               * 0 1 23 45 6     7         8       9             11
               */
              /* DO SETUP FOR Edit RTC DOWN */
              if (Menu.SubItem < 6){
                  NTPData.TravelTemp -= ((Menu.SubItem == 1 ? 3600 : 0) + (Menu.SubItem == 2 ? 600 : 0) + (Menu.SubItem == 3 ? 60 : 0) + (Menu.SubItem == 4 ? 10 : 0) + (Menu.SubItem == 5 ? 1 : 0));
                  DoHaptic = true;
                  /* TEST  UpdateDisp = true;  // Quick Update.*/
                  SetTurbo();
              }else if (Menu.SubItem == 11){ // Drift Value edit..
                  Diff = SRTC.getDrift(WatchTime.ESPRTC); S = String(Diff);
                  Diff -= pow(10,(S.length() - (1 + Menu.SubSubItem)));
                  if (Diff > 999){
                      SRTC.setDrift(Diff, SRTC.isFastDrift(WatchTime.ESPRTC), WatchTime.ESPRTC);
                      Options.NeedsSaving = true;
                      DoHaptic = true;
                      UpdateDisp = true;  // Quick Update.
                      SetTurbo();
                  }
              }else{
                  Menu.SubItem = roller(Menu.SubItem - 1,6,9); if (Menu.SubItem == 8 && SRTC.getDrift(WatchTime.ESPRTC) == 0) Menu.SubItem--;
                  if (Menu.SubItem == 7 && !SRTC.checkingDrift(WatchTime.ESPRTC)) Menu.SubItem--;
                  if (WatchTime.ESPRTC) Menu.SubItem == 9;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
          }else if (Menu.Item == GSR_MENU_UNVS && Menu.SubItem == 1){  // USE NVS
              if (Menu.SubSubItem == 0){
                  Menu.SubSubItem = 1;
                  DoHaptic = true;
                  UpdateDisp = true;  // Quick Update.
                  SetTurbo();
              }
              return;
          }else{
              if (Menu.Style == GSR_MENU_INOPTIONS){
                  Menu.Item = roller(Menu.Item + 1, GSR_MENU_STYL, (Watchy_Chip_Info.HasWiFi & (NTPData.State > 0 || WatchyAPOn || OTAUpdate || Battery.Read < Battery.RadioLevel)) ? GSR_MENU_TRBL : GSR_MENU_OTAM);
                  if (Menu.Item == GSR_MENU_WEAT && !GetWantWeather()) Menu.Item = roller(Menu.Item + 1, GSR_MENU_STYL, (Watchy_Chip_Info.HasWiFi & (NTPData.State > 0 || WatchyAPOn || OTAUpdate || Battery.Read < Battery.RadioLevel)) ? GSR_MENU_TRBL : GSR_MENU_OTAM);
              }else if (Menu.Style == GSR_MENU_INALARMS){
                  Menu.Item = roller(Menu.Item + 1, GSR_MENU_ALARM1, GSR_MENU_TONES);
              }else if (Menu.Style == GSR_MENU_INTIMERS){
                  Menu.Item = roller(Menu.Item + 1, GSR_MENU_TIMEDN, GSR_MENU_TIMEUP);
              }else if (Menu.Style == GSR_MENU_INTROUBLE){
                  Menu.Item = roller(Menu.Item + 1, GSR_MENU_SCRN, GSR_MENU_UNVS);
              }else{
#ifndef STABLEBMA_H_INCLUDED
                  Menu.Item = roller(Menu.Item + 1, GSR_MENU_ALARMS, GSR_MENU_OPTIONS);
                  if (Menu.Item == GSR_MENU_GAME && Options.Game == 255) Menu.Item = roller(Menu.Item + 1, GSR_MENU_ALARMS, GSR_MENU_OPTIONS);
#else
                  Menu.Item = roller(Menu.Item + 1, GSR_MENU_STEPS, GSR_MENU_OPTIONS);
                  if (Menu.Item == GSR_MENU_GAME && Options.Game == 255) Menu.Item = roller(Menu.Item + 1, GSR_MENU_STEPS, GSR_MENU_OPTIONS);
#endif
              }
              Menu.SubItem=0;
              Menu.SubSubItem=0;
              DoHaptic = true;
              UpdateDisp = true;  // Quick Update.
              SetTurbo();
          }
      } else Missed = 4;  // Missed a SW4.
      break;
    default:
      if (Pressed < 9) Missed = Pressed;  // Pass off other buttons not handled here.
  }
}

void WatchyGSR::UpdateTimerDown(){
    uint16_t T;
    bool B = (TimerDown.Active && Menu.Item == GSR_MENU_TIMEDN);
    if (TimerDown.Active){
        UpdateUTC(); T = (TimerDown.StopAt - WatchTime.UTC_RAW);
        if (T > -1){
            TimerDown.Hours = (T / 3600);
            T = T - (TimerDown.Hours * 3600);
            TimerDown.Mins = (T / 60);
            if (TimerDown.Secs != (T % 60) && B) UpdateDisp |= Showing();
            TimerDown.Secs = (T % 60);
        }else{
            TimerDown.Hours = 0;
            if (TimerDown.Secs != 0 && B) UpdateDisp |= Showing();
            TimerDown.Mins = 0;
            TimerDown.Secs = 0;
        }
        if (TimerDown.Hours == 0 && TimerDown.Mins == 0 && TimerDown.Secs == 0){
            if (TimerDown.Secs == 0){
                TimerDown.Tone = 22;
                TimerDown.ToneLeft = 255;
                TimerDown.Active = false;
                SoundStart = true;
                if (TimerDown.Repeats) StartCD();
                Darkness.Last=millis();
                /* FIX DISPLAY */
                DisplayWake(true);
            }
        }
    }
}

bool WatchyGSR::TimerAbuse(){
    bool A = ((TimerUp.Active && Menu.Item == GSR_MENU_TIMEUP) || (Menu.SubItem > 0 && Menu.SubItem < 6 && Menu.Item == GSR_MENU_TOFF));
    if (A) UpdateDisp|=Showing();
    return (A || (TimerDown.Active && Menu.Item == GSR_MENU_TIMEDN) || (TimerDown.Active && ((TimerDown.Hours * 3600) + (TimerDown.Mins * 60) + TimerDown.Secs) < 60) || (Menu.SubItem > 0 && Menu.SubItem < 6 && Menu.Item == GSR_MENU_TOFF));
}

void WatchyGSR::UpdateUTC(){
    SRTC.read(WatchTime.UTC);
    WatchTime.UTC_RAW = SRTC.doMakeTime(WatchTime.UTC);
    SRTC.doBreakTime(WatchTime.UTC_RAW,WatchTime.UTC);
}

void WatchyGSR::UpdateClock(){
    WatchTime.Local = UTCtoLocal(WatchTime.UTC_RAW);
    bool B = WatchTime.BedTime;
    if (Options.SleepEnd > Options.SleepStart) WatchTime.BedTime = (WatchTime.Local.Hour >= Options.SleepStart && WatchTime.Local.Hour < Options.SleepEnd);
    else WatchTime.BedTime = (WatchTime.Local.Hour >= Options.SleepStart || WatchTime.Local.Hour < Options.SleepEnd);
/* SORT OUT THE NIGHTTIME Wallpaper Change */
    if (WatchTime.BedTime != B) { Updates.Drawn = true; } // Fake we messed up the screen.
    monitorSteps(); // Moved here for accuracy during Deep Sleep.
}

tmElements_t WatchyGSR::UTCtoLocal(time_t Incoming){
    struct tm * TM;
    tmElements_t T;

    OP.setCurrentTimeZone();
    TM = localtime(&Incoming);
    T.Second = TM->tm_sec;
    T.Minute = TM->tm_min;
    T.Hour = TM->tm_hour;
    T.Wday = TM->tm_wday;
    T.Day = TM->tm_mday;
    T.Month = TM->tm_mon;
    T.Year = TM->tm_year;
    return T;
}

time_t WatchyGSR::getISO8601(String inTime){
    tmElements_t sunT;
    String s;
    uint i;
    s=inTime.charAt(i);
    if (s == "[") i++;
    s=inTime.charAt(i);
    if (s == " ") i++;
    s = inTime.substring(i,i + 4);
    sunT.Year = s.toInt();
    i+=5;
    s = inTime.substring(i,i + 2);
    sunT.Month = s.toInt();
    i+=3;
    s = inTime.substring(i,i + 2);
    sunT.Day = s.toInt();
    i+=3;
    s = inTime.substring(i,i + 2);
    sunT.Hour = s.toInt();
    i+=3;
    s = inTime.substring(i,i + 2);
    sunT.Minute = s.toInt();
    return SRTC.doMakeTime(sunT);
}

// Manage time will determine if the RTC is in use, will also set a flag to "New Minute" for the loop functions to see the minute change.
void WatchyGSR::ManageTime(){
    tmElements_t TM;  //    struct tm * tm;
    uint32_t Test = millis();
    int I;
    bool B;

    if (SRTC.isNewMinute()){
        UpdateUTC();
        WatchTime.NewMinute=true;
        WfNM = false;
    }

    if (WatchTime.NewMinute){
        UpdateDisp|=Showing();
        UpdateUTC();
        UpdateClock();
        if (Options.Game != 255) if (WatchStyles.AddOn[Options.Game] != nullptr) WatchStyles.AddOn[Options.Game]->InsertOnMinute();
        if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr) InsertOnMinute(); else WatchStyles.AddOn[Options.WatchFaceStyle]->InsertOnMinute();
    }
}

// Functions for Override usage.
String WatchyGSR::CountdownTimer(){
    if (TimerDown.Active) UpdateUTC();
    return String(TimerDown.Active ? TimerDown.Hours : TimerDown.MaxHours) + ":" + MakeMinutes(TimerDown.Active ? TimerDown.Mins : TimerDown.MaxMins) + ":" + MakeMinutes(TimerDown.Active ? TimerDown.Secs : TimerDown.MaxSecs);
}

String WatchyGSR::CountdownTimerState(){ return LGSR.GetID(Options.LanguageID, (TimerDown.Active ? 68 : 69)); }
bool WatchyGSR::CountdownTimerActive() { return TimerDown.Active; }

String WatchyGSR::ElapsedTimer(){
    unsigned long U;
    int16_t H, M, S;
    if (TimerUp.Active) { UpdateUTC(); U = (WatchTime.UTC_RAW - TimerUp.SetAt); } else U = (TimerUp.StopAt - TimerUp.SetAt);
    H = U / 3600; M = (U - (H * 3600)) / 60; S = U % 60;
    return String(H) + ":" + MakeMinutes(M) + ":" + MakeMinutes(S);
}

String WatchyGSR::ElapsedTimerState() { return LGSR.GetID(Options.LanguageID, (TimerUp.Active ? 68 : 69)); }
bool WatchyGSR::ElapsedTimerActive() { return TimerUp.Active; }

String WatchyGSR::CurrentSteps(bool Yesterday){
    String S = YesterdaySteps();
    return MakeSteps(CurrentStepCount()) + ((S > "" && Yesterday) ? " (" + S + ")" : "");
}
#ifdef STABLEBMA_H_INCLUDED
uint32_t WatchyGSR::CurrentStepCount() { return SBMA.getCounter() + Steps.Cached; }
#else
uint32_t WatchyGSR::CurrentStepCount() { return Steps.Cached; }
#endif

String WatchyGSR::YesterdaySteps(){
    if (Steps.Yesterday > 0) return MakeSteps(Steps.Yesterday);
    return "";
}
uint32_t WatchyGSR::YesterdayStepCount() { return Steps.Yesterday; }
bool WatchyGSR::BMAAvailable(){
#ifdef STABLEBMA_H_INCLUDED
    return true;
#else
    return false;
#endif  
}
float WatchyGSR::BMATemperature(bool Metric){
#ifdef STABLEBMA_H_INCLUDED
    if (BMAAvailable()) return (Metric ? SBMA.readTemperature() : SBMA.readTemperatureF());
#endif  
    return -1;
}
String WatchyGSR::CurrentWatchStyle(){
    uint8_t X;
    char O[32];
    memcpy(&O[0],&WatchStyles.Style[Options.WatchFaceStyle * 32],32);
  return String(O);
}

String WatchyGSR::CurrentGameStyle(){
    uint8_t X;
    char O[32];
    if (Options.Game != 255) memcpy(&O[0],&WatchStyles.Style[Options.Game * 32],32);
  return String(O);
}

void WatchyGSR::_bmaConfig() {
uint8_t Type = SRTC.getType();

#ifdef STABLEBMA_H_INCLUDED
  if (SBMA.begin(_readRegister, _writeRegister, delay, Type) == false) {
    //fail to init BMA
    return;
  }

  if (!SBMA.defaultConfig(HWVer != 3.0)) return;  // Failed.
  // Enable BMA423 isStepCounter feature
  SBMA.enableFeature(BMA423_STEP_CNTR, true);
#endif
}

void WatchyGSR::UpdateBMA(){
    bool BT = (Options.SleepStyle == 2 && WatchTime.BedTime);
    bool B = (Options.SleepStyle > 2 && Options.SleepStyle != 4);
    bool A = (Options.SleepStyle == 1);

#ifdef STABLEBMA_H_INCLUDED
    if (HWVer != 3.0f){
        SBMA.enableDoubleClickWake(B | BT);
        SBMA.enableTiltWake((A | B) & !WatchTime.BedTime);
    }
#endif
}

float WatchyGSR::getBatteryVoltage(){ return ((BatteryRead() - 0.0125) +  (BatteryRead() - 0.0125) + (BatteryRead() - 0.0125) + (BatteryRead() - 0.0125)) / 4; }
float WatchyGSR::BatteryRead(){ return analogReadMilliVolts(Battery.ADCPin) / Battery.Divider2; }
float WatchyGSR::rawBatteryVoltage(){ return (((analogReadMilliVolts(Battery.ADCPin) / Battery.Divider) + (analogReadMilliVolts(Battery.ADCPin) / Battery.Divider) + (analogReadMilliVolts(Battery.ADCPin) / Battery.Divider) + (analogReadMilliVolts(Battery.ADCPin) / Battery.Divider)) / 4.0f); }

uint16_t WatchyGSR::_readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)address, (uint8_t)len);
  uint8_t i = 0;
  while (Wire.available()) {
    data[i++] = Wire.read();
  }
  return 0;
}

uint16_t WatchyGSR::_writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data, len);
  return (0 !=  Wire.endTransmission());
}

// Override functions.
uint16_t WatchyGSR::ForeColor(){ return (Options.LightMode ? GxEPD_BLACK : GxEPD_WHITE); }

uint16_t WatchyGSR::BackColor(){ return (Options.LightMode ? GxEPD_WHITE : GxEPD_BLACK); }

uint16_t WatchyGSR::Debounce() { if (Options.Performance == 0) return 575; else if (Options.Performance == 1) return 645; else return 875; }
uint8_t WatchyGSR::CurrentStyleID() { if (GuiMode == GSR_GAMEON) return Options.Game; else return Options.WatchFaceStyle; }
uint8_t WatchyGSR::CurrentGameID() { return Options.Game; }
void WatchyGSR::InsertPost() {}
bool WatchyGSR::OverrideBitmap() { return false; }
bool WatchyGSR::OverrideSleepBitmap() { return false; }
void WatchyGSR::InsertDefaults() {}
void WatchyGSR::InsertOnMinute() {}
void WatchyGSR::InsertWiFi() {}
void WatchyGSR::InsertWiFiEnding() {}
void WatchyGSR::InsertAddWatchStyles() {}
void WatchyGSR::InsertInitWatchStyle(uint8_t StyleID) {}
void WatchyGSR::InsertDrawWatchStyle(uint8_t StyleID) {}
void WatchyGSR::SaveProgress() {}
void WatchyGSR::NeedsSaving() { Options.NeedsSaving = true; }
void WatchyGSR::InsertDrawWeather(uint8_t StyleID, bool Status) {}
bool WatchyGSR::InsertNeedAwake(bool GoingAsleep) { return false; }
bool WatchyGSR::InsertHandlePressed(uint8_t SwitchNumber, bool &Haptic, bool &Refresh) { return false; }
void WatchyGSR::OverrideDefaultMenu(bool Override) { WatchStyles.Options[Options.WatchFaceStyle] = (WatchStyles.Options[Options.WatchFaceStyle] & ~GSR_MOV) | (Override ? GSR_MOV : 0 ); }
void WatchyGSR::WantWeather(bool Wanting) { WatchStyles.Options[Options.WatchFaceStyle] = (WatchStyles.Options[Options.WatchFaceStyle] & ~GSR_iWW) | (Wanting ? GSR_iWW : 0 ); }
void WatchyGSR::NoStatus(bool NoStats) { WatchStyles.Options[Options.WatchFaceStyle] = (WatchStyles.Options[Options.WatchFaceStyle] & ~GSR_NOS) | (NoStats ? GSR_NOS : 0 ); }
void WatchyGSR::SetAskWiFi(bool SetWiFi) { if (GuiMode != GSR_GAMEON) WatchStyles.Options[Options.WatchFaceStyle] = (WatchStyles.Options[Options.WatchFaceStyle] & ~GSR_AFW) | (SetWiFi ? GSR_AFW : 0 ); }
bool WatchyGSR::GetAskWiFi() { return (GuiMode != GSR_GAMEON ? (WatchStyles.Options[Options.WatchFaceStyle] & GSR_AFW) : false); }
bool WatchyGSR::GetWantWeather() { return (WatchStyles.Options[Options.WatchFaceStyle] & GSR_iWW); }
bool WatchyGSR::GetNoStatus() { return (WatchStyles.Options[Options.WatchFaceStyle] & GSR_NOS); }
bool WatchyGSR::GetMenuOverride() { return (WatchStyles.Options[Options.WatchFaceStyle] & GSR_MOV); }
void WatchyGSR::RegisterWatchFaces(){}
void WatchyGSR::ShowDefaultMenu() { if (GetMenuOverride() && (GuiMode == GSR_WATCHON || GuiMode == GSR_GAMEON)) { GuiMode = GSR_MENUON; DoHaptic = true; UpdateDisp = true; SetTurbo(); } }
void WatchyGSR::ShowGame() { if (Options.Game != 255) { Menu.SubItem = 0; GuiMode = GSR_GAMEON; UpdateDisp = true; } }
void WatchyGSR::HideGame() { if (GuiMode = GSR_GAMEON) { Menu.SubItem = 0; GuiMode = GSR_WATCHON; UpdateDisp = true; } }
void WatchyGSR::UpdateScreen() { UpdateDisp |= Showing(); Updates.Drawn = true; }
void WatchyGSR::GameStatus(bool NeedAttention) { if (NeedAttention != Options.GameStatus) UpdateScreen(); Options.GameStatus = NeedAttention; }
bool WatchyGSR::GameStatusOn() { return Options.GameStatus; }
void WatchyGSR::initAddOn(WatchyGSR *NewAddOn){
  uint8_t I = 0;
  for (I = 0; I < BootAddOns.Count; I++){
     if (NewAddOn == BootAddOns.Inits[I]) return; // Found it, exit.
  }
  if (BootAddOns.Count < GSR_MaxStyles){
    BootAddOns.Inits[BootAddOns.Count] = NewAddOn;
    BootAddOns.Count++;
  }
}

uint8_t WatchyGSR::AddWatchStyle(String StyleName, WatchyGSR *AddOn, bool IsGame){
  int I, O;
    O = WatchStyles.Count;
    if (O >= GSR_MaxStyles || StyleName.length() > 30) return 255;  // Full / too long..
    for (I = 0; I < O; I++)
        if (String(WatchStyles.Style[I * 32]) == StyleName) return 255;  // Error, alrady there.

    strncpy(&WatchStyles.Style[O * 32], StyleName.c_str(),32);
    WatchStyles.Options[O] = (IsGame ? GSR_GAM : 0);
    WatchStyles.AddOn[O] = AddOn;
    WatchStyles.Count++;
    if (IsGame) Options.GameCount++;
    return O;
}
uint8_t WatchyGSR::AddWatchStyle(String StyleName){ return WatchyGSR::AddWatchStyle(StyleName, nullptr, false); }
String WatchyGSR::InsertNTPServer() { return "pool.ntp.org"; }
void WatchyGSR::AllowDefaultWatchStyles(bool Allow) { DefaultWatchStyles = Allow; }

bool WatchyGSR::IsDark(){ return Darkness.Went; }
bool WatchyGSR::IsAM() { return (!Options.TwentyFour && WatchTime.Local.Hour < 12); }
bool WatchyGSR::IsPM() { return (!Options.TwentyFour && WatchTime.Local.Hour > 11); }
String WatchyGSR::GetLangWebID() { return LGSR.GetWebLang(Options.LanguageID); }

String WatchyGSR::MakeTime(int Hour, int Minutes, bool& Alarm){  // Use variable with Alarm, if set to False on the way in, returns PM indication.
    int H;
    String AP = "";
    H = (Hour & 31);
    if (!Options.TwentyFour){
        if (H > 11){
          AP = " " + LGSR.GetID(Options.LanguageID,114);
          if (!Alarm){
              Alarm = true;   // Tell the clock to use the PM indicator.
          }
      }else{
          AP = " " + LGSR.GetID(Options.LanguageID,115);
        }
        if (H > 12){
            H -= 12;
        }else if (H == 0){
            H = 12;
        }
    }
    return (((Hour & 64) && H < 10) ? " " : "") + String(H) + (Minutes < 10 ? ":0" : ":") + String(Minutes) + ((Hour & 32) ? "" : AP);
}

String WatchyGSR::MakeHour(uint8_t Hour){
    int H;
    H = Hour;
    if (!Options.TwentyFour){
        if (H > 12){
          H -= 12;
        }
        if (H == 0){
          H = 12;
        }
    }
    return String(H);
}

String WatchyGSR::MakeSeconds(uint8_t Seconds){ return LGSR.GetID(Options.LanguageID,(Seconds > 1 ? 112 : 113)) + "."; }

String WatchyGSR::MakeTOD(uint8_t Hour, bool AddZeros){
    if(Options.TwentyFour){
        if (AddZeros) return ":00";
        return "";
    }
    return " " + LGSR.GetID(Options.LanguageID,(Hour > 11 ? 114 : 115));
}

String WatchyGSR::MakeMinutes(uint8_t Minutes){
    return (Minutes < 10 ? "0" : "") + String(Minutes);
}

void WatchyGSR::ClockSeconds(){ UpdateUTC(); UpdateClock(); }

String WatchyGSR::MakeSteps(uint32_t uSteps){
    String S, T, X;
    int I, C;

    S = String(uSteps);
    I = S.length();
    C = 0; T = "";

    while (I > 0){
        X = (I > 1 && C == 2) ? "," : "";
        T = X + S.charAt(I - 1) + T;
        C = roller(C + 1, 0, 2); I--;
    }
    return T;
}

void WatchyGSR::CheckAlarm(int I){
    uint16_t  B;
    bool bA;
    B = (GSR_ALARM_ACTIVE | Bits[WatchTime.Local.Wday]);
    bA = (Alarms_Hour[I] == WatchTime.Local.Hour && Alarms_Minutes[I] == WatchTime.Local.Minute);
    if (!bA && Alarms_Times[I] == 0 && (Alarms_Active[I] & GSR_ALARM_TRIGGERED) != 0){
        Alarms_Active[I] &= GSR_ALARM_NOTRIGGER;
    }else if ((Alarms_Active[I] & B) == B && (Alarms_Active[I] & GSR_ALARM_TRIGGERED) == 0){  // Active and Active Day.
        // Check alarm listed to see if it is earlier than the one slated.
        if (Alarms_Hour[I] == WatchTime.Local.Hour && Alarms_Minutes[I] > WatchTime.Local.Minute && !bA){
            if (WatchTime.NextAlarm == 99) WatchTime.NextAlarm = I; else if (Alarms_Minutes[I] < Alarms_Minutes[WatchTime.NextAlarm]) WatchTime.NextAlarm = I;
        }
        if (bA && Alarms_Times[I] == 0){
            Alarms_Times[I] = 255;
            Alarms_Playing[I] = 30;
            Darkness.Last=millis();
            SoundStart = true;
            /* FIX DISPLAY */
            DisplayWake(true);
            Alarms_Active[I] |= GSR_ALARM_TRIGGERED;
            if ((Alarms_Active[I] & GSR_ALARM_REPEAT) == 0){
                Alarms_Active[I] &= (GSR_ALARM_ALL - Bits[WatchTime.Local.Wday]);
                if ((Alarms_Active[I] & GSR_ALARM_DAYS) == 0){
                    Alarms_Active[I] ^= GSR_ALARM_ACTIVE; // Turn it off, not repeating.
                }
            }
        }
    }
}

// Counts the active (255) alarms/timers and after 3, sets them to lower values.
void WatchyGSR::CalculateTones(){
    uint8_t Count = 0;
    float Downgrade = 1.0;
    uint32_t TimeDown;
    WatchTime.NextAlarm = 99; CheckAlarm(0); CheckAlarm(1); CheckAlarm(2); CheckAlarm(3); UpdateTimerDown();
    TimeDown = TimerDown.Hours * 3600 + TimerDown.Mins * 60 + TimerDown.Secs;
    if (TimeDown < 11 && TimerDown.Repeats) Downgrade = gobig(0.2, TimeDown * 0.25);
    if (Alarms_Times[0] > 0) Count++;
    if (Alarms_Times[1] > 0) Count++;
    if (Alarms_Times[2] > 0) Count++;
    if (Alarms_Times[3] > 0) Count++;
    if (TimerDown.ToneLeft > 0) Count++;
    if (Count == 5){
        if (Alarms_Times[0] == 255) Alarms_Times[0] = 7 * Reduce[Alarms_Repeats[0]];
        if (Alarms_Times[1] == 255) Alarms_Times[1] = 7 * Reduce[Alarms_Repeats[1]];
        if (Alarms_Times[2] == 255) Alarms_Times[2] = 7 * Reduce[Alarms_Repeats[2]];
        if (Alarms_Times[3] == 255) Alarms_Times[3] = 7 * Reduce[Alarms_Repeats[3]];
        if (TimerDown.ToneLeft == 255) TimerDown.ToneLeft = (10 * Downgrade) * Reduce[TimerDown.MaxTones];
    }else if (Count == 4){
        if (Alarms_Times[0] == 255) Alarms_Times[0] = 8 * Reduce[Alarms_Repeats[0]];
        if (Alarms_Times[1] == 255) Alarms_Times[1] = 8 * Reduce[Alarms_Repeats[1]];
        if (Alarms_Times[2] == 255) Alarms_Times[2] = 8 * Reduce[Alarms_Repeats[2]];
        if (Alarms_Times[3] == 255) Alarms_Times[3] = 8 * Reduce[Alarms_Repeats[3]];
        if (TimerDown.ToneLeft == 255) TimerDown.ToneLeft = (12 * Downgrade) * Reduce[TimerDown.MaxTones];
    }else{
        if (Alarms_Times[0] == 255) Alarms_Times[0] = 10 * Reduce[Alarms_Repeats[0]];
        if (Alarms_Times[1] == 255) Alarms_Times[1] = 10 * Reduce[Alarms_Repeats[1]];
        if (Alarms_Times[2] == 255) Alarms_Times[2] = 10 * Reduce[Alarms_Repeats[2]];
        if (Alarms_Times[3] == 255) Alarms_Times[3] = 10 * Reduce[Alarms_Repeats[3]];
        if (TimerDown.ToneLeft == 255) TimerDown.ToneLeft = (15 * Downgrade) * Reduce[TimerDown.MaxTones];
    }
    Alarming = (Alarms_Times[0] || Alarms_Times[1] || Alarms_Times[2] || Alarms_Times[3] || TimerDown.ToneLeft);
}

void WatchyGSR::StartCD(){
    TimerDown.Secs = TimerDown.MaxSecs;
    TimerDown.Mins = TimerDown.MaxMins;
    TimerDown.Hours = TimerDown.MaxHours;
    UpdateUTC();
    TimerDown.LastUTC = WatchTime.UTC_RAW;
    TimerDown.StopAt = TimerDown.LastUTC + (3600 * TimerDown.Hours) + (60 * TimerDown.Mins) + TimerDown.Secs;
    TimerDown.Repeating = TimerDown.Repeats;
    TimerDown.Active = true;
}

void WatchyGSR::StopCD(){
    if (TimerDown.ToneLeft > 0){
        TimerDown.ToneLeft = 1;
        TimerDown.Tone = 1;
    }
    TimerDown.Active = false;
    TimerDown.Repeating = false;
}

uint8_t WatchyGSR::getToneTimes(uint8_t ToneIndex){
    if (ToneIndex > 3) return TimerDown.ToneLeft;
    return Alarms_Times[ToneIndex];
}

String WatchyGSR::getReduce(uint8_t Amount){
    switch (Amount){
        case 0:
          return LGSR.GetID(Options.LanguageID,116);
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
void WatchyGSR::monitorSteps(){
    bool B;
    if (Steps.Hour == WatchTime.Local.Hour && Steps.Minutes == WatchTime.Local.Minute){
        if (!Steps.Reset){
            Steps.Yesterday=CurrentStepCount();
#ifdef STABLEBMA_H_INCLUDED
            SBMA.resetStepCounter();
#endif
            Steps.Cached=0;
            Steps.Reset=true;
        }
    }else if (Steps.Reset) Steps.Reset=false;
    /* Store steps in NVS if it has been a minute+ since last store */
    if (WatchTime.UTC_RAW - Steps.Stored > 59) {
        Steps.Stored = WatchTime.UTC_RAW;
        if (OkNVS(GName)){
            B = NVS.setString(STP,String(CurrentStepCount()));
            B = NVS.setString(YSTP,String(Steps.Yesterday));
            NVS.commit();
        }
    }
}

void WatchyGSR::KeysStart() { if (KeysHandle == NULL) { KeysRet = xTaskCreate(WatchyGSR::KeysCheck,"WatchyGSR_KeysCheck",20480,NULL,(configMAX_PRIORITIES - 2),&KeysHandle); } }
void WatchyGSR::KeysStop() { while (KeysHandle != NULL) { KeysCheckOn = false; vTaskDelay(20/portTICK_PERIOD_MS); } }

void WatchyGSR::KeysCheck(void * parameter){
  bool Ok;
  uint8_t B;
    KeysCheckOn = true;
    while (KeysCheckOn) {
        Ok = !(Options.SleepStyle == 4 && !Updates.Tapped);
        if (Ok && (LastButton == 0 || (millis() - LastButton) > Debounce()) && Missed == 0 && Button == 0){
            B = WatchyGSR::getButtonPins();
            if (B) Button = B;
        }
        vTaskDelay(50/portTICK_PERIOD_MS);    // 100ms pauses.
    }
    KeysHandle = NULL;
    vTaskDelete(NULL);
}

void WatchyGSR::CheckButtons(){
    uint8_t B = WatchyGSR::getButtonPins();
    if (Options.SleepStyle == 4 && !Updates.Tapped) return;   // Screen didn't permit buttons to work.
    if (!UpdateDisp && B && (LastButton == 0 || (millis() - LastButton) > Debounce()) && Missed == 0 && Button == 0) Button = B;
}

uint8_t WatchyGSR::getButtonPins(){ return getSWValue((digitalRead(GSR_MENU_PIN) == GSR_ESP_WAKEUP), (digitalRead(GSR_BACK_PIN) == GSR_ESP_WAKEUP), (digitalRead(GSR_UP_PIN) == GSR_ESP_WAKEUP), (digitalRead(GSR_DOWN_PIN) == GSR_ESP_WAKEUP)); }

uint8_t WatchyGSR::getButtonMaskToID(uint64_t HW){
    uint8_t HB = getSWValue((HW & GSR_MENU_MASK), (HW & GSR_BACK_MASK), (HW & GSR_UP_MASK), (HW & GSR_DOWN_MASK));
    uint8_t LB = WatchyGSR::getButtonPins();
#ifdef STABLEBMA_H_INCLUDED
    if (SBMA.didBMAWakeUp(HW)) {           // Acccelerometer.
        if (SBMA.isDoubleClick()) return 9;  // Double Tap.
        else if (SBMA.isTilt()) return 10;  // Wrist Tilt.
    }
#endif
    if (LB > 0 && LB != HB && GSR_PIN_RTC != 255) HB=LB; // V3 doesn't allow custom buttons at boot time.
    return HB;
}

uint8_t WatchyGSR::getSWValue(bool SW1, bool SW2, bool SW3, bool SW4){
    bool T;
    if (Options.Swapped) { T = SW1; SW2=SW1; SW1=T; }
    if (Options.Lefty){
        T = SW1; SW1=SW4; SW4=T;
        T = SW3; SW3=SW2; SW2=T;
    }
if ((SW1 | SW2 | SW3 | SW4)){
}
    if (SW3 && SW1) return 5;
    if (SW3 && SW2 && HWVer != 3.0f) return 6;
    if (SW4 && SW1) return 7;
    if (SW4 && SW2) return 8;
    if (SW1) return 1;
    if (SW2) return 2;
    if (SW3) return 3;
    if (SW4) return 4;
    return 0;
}

void WatchyGSR::AskForWiFi(){ if (Battery.Read > Battery.RadioLevel) { SetAskWiFi(Watchy_Chip_Info.HasWiFi); GSRWiFi.Requests++; } }
void WatchyGSR::endWiFi(){
    if (!GetAskWiFi() && (GSRWiFi.Powered || GSRWiFi.Requests > 0)) GSRWiFi.Requests = 0;
    if (GSRWiFi.Requests > 0) GSRWiFi.Requests--;
    if (GSRWiFi.Powered && GSRWiFi.Requests == 0){
        WiFi.disconnect();
        if(GSRWiFi.WiFiEventID) WiFi.removeEvent(GSRWiFi.WiFiEventID);
        GSRWiFi.WiFiEventID = 0;
        WiFi.mode(WIFI_OFF);
        GSRWiFi.Requests = 0;
        GSRWiFi.Requested = false;
        GSRWiFi.Working = false;
        GSRWiFi.Results = false;
        GSRWiFi.Index = 0;
        GSRWiFi.Powered = false;
        SetAskWiFi(false);
        if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr) InsertWiFiEnding(); else WatchStyles.AddOn[Options.WatchFaceStyle]->InsertWiFiEnding();
        WatchyOStatus = "";
        setStatus("");
    }
}

bool WatchyGSR::HasIPAddress() {
    if (WiFi.status() == WL_CONNECTED){ GSRWiFi.LocalIP = WiFi.localIP(); return !(GSRWiFi.LocalIP[0] == 0 || GSRWiFi.LocalIP[3] == 0 || (GSRWiFi.LocalIP[0] == 169 && GSRWiFi.LocalIP[1] == 254)); }
    return false;
}

bool WatchyGSR::WiFiInProgress() { return (GSRWiFi.Requests > 0 && (GSRWiFi.Requested || GSRWiFi.Working || GSRWiFi.Results)); }

void WatchyGSR::processWiFiRequest(){
    wl_status_t WiFiE = WL_CONNECT_FAILED;
    wifi_config_t conf;
    String AP, PA, O;
    uint8_t I;

    if (GSRWiFi.Requests > 0){
        if (!GSRWiFi.Requested){
            RefreshCPU(GSR_CPUMAX);
            ResetOTA();
            GSRWiFi.Powered = true;
#ifdef ARDUINO_ESP32_RELEASE_1_0_6
            if (GSRWiFi.WiFiEventID == 0) GSRWiFi.WiFiEventID = WiFi.onEvent(WatchyGSR::WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);  // ARDUINO_EVENT_WIFI_STA_DISCONNECTED
#else
            if (GSRWiFi.WiFiEventID == 0) GSRWiFi.WiFiEventID = WiFi.onEvent(WatchyGSR::WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);  // SYSTEM_EVENT_STA_DISCONNECTED
#endif
            GSRWiFi.Index = 0;
            GSRWiFi.Tried = false;
            GSRWiFi.Last = 0;
            GSRWiFi.Slow = 3;
            GSRWiFi.Requested = true;
            GSRWiFi.Working = true;
            return;
        }
    }

    if (WiFiInProgress()) {
        if (WatchyGSR::getButtonPins() != 2) OTATimer = millis(); // Not pressing "BACK".
        if (millis() - OTATimer > 10000 || millis() > OTAFail || IsEndOTA()) OTAEnd = true; // Fail if holding back for 10 seconds OR 600 seconds has passed.
    }

    if (GSRWiFi.Working) {
        if (HasIPAddress()) { GSRWiFi.Working = false; setStatus(WiFiIndicator(GSRWiFi.Index ? GSRWiFi.Index : 24)); GSRWiFi.Results = true; return; } // We got connected.
        if (SoundHandle != NULL) return;  // Don't do this while the buzzer is on (trying to avoid brownouts).
        if (millis() > GSRWiFi.Last){
            RefreshCPU(GSR_CPUMAX);
            if (GSRWiFi.Tried && !GSRWiFi.Prepped){
                WiFi.disconnect();
                WiFi.mode(WIFI_STA);
                GSRWiFi.Index++;
                GSRWiFi.Tried = false;
                GSRWiFi.Prepped = true;
                GSRWiFi.Slow = 1;
                WiFi.setHostname(WiFi_AP_SSID);
                return;
            }
            if (GSRWiFi.Index < 11){
                GSRWiFi.Prepped = false; WatchyOStatus = "";
                if (GSRWiFi.Index == 0){
                    GSRWiFi.Tried = true;
                    GSRWiFi.Slow = 3;
                    if (WiFi_DEF_SSID > ""){
                        WiFiE = WiFi.begin(WiFi_DEF_SSID,WiFi_DEF_PASS);
                        UpdateWiFiPower(WiFi_DEF_SSID,WiFi_DEF_PASS);
                        esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
                        setStatus(WiFiIndicator(GSRWiFi.Index ? GSRWiFi.Index : 24) + "!");
                    }else{
                        WiFiE = WiFi.begin();
                        esp_wifi_get_config((wifi_interface_t)WIFI_IF_STA, &conf);
                        UpdateWiFiPower(reinterpret_cast<const char*>(conf.sta.ssid),reinterpret_cast<const char*>(conf.sta.password));
                        esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
                        setStatus(WiFiIndicator(GSRWiFi.Index ? GSRWiFi.Index : 24) + "!");
                    }
                    GSRWiFi.Last = millis() + 9000;
                    return;
                }
                if (GSRWiFi.Index > 0){
                    AP = APIDtoString(GSRWiFi.Index - 1); PA = PASStoString(GSRWiFi.Index - 1);
                    if (AP.length() > 0){
                        GSRWiFi.Tried = true;
                        WiFiE = WiFi.begin(AP.c_str(),PA.c_str());
                        setStatus(WiFiIndicator(GSRWiFi.Index ? GSRWiFi.Index : 24) + "!");
                        UpdateWiFiPower(GSRWiFi.AP[GSRWiFi.Index].TPWRIndex);
                        esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
                        GSRWiFi.Last = millis() + 9000;
                    }else GSRWiFi.Index++;
                }
            }else endWiFi();
        }
    }
}

String WatchyGSR::WiFiIndicator(uint8_t Index){ return String(WiFiTXT) + String((char) ('@' + Index)); }

void WatchyGSR::UpdateWiFiPower(String SSID, String PSK){
    uint8_t I;
    for (I = 0; I < 11; I++){ if (APIDtoString(I) == SSID && PASStoString(I) == PSK) { UpdateWiFiPower(GSRWiFi.AP[I].TPWRIndex); return; } }
    UpdateWiFiPower();
}

void WatchyGSR::UpdateWiFiPower(String SSID){
    uint8_t I;
    for (I = 0; I < 11; I++){ if (APIDtoString(I) == SSID) { UpdateWiFiPower(GSRWiFi.AP[I].TPWRIndex); return; } }
    UpdateWiFiPower();
}

void WatchyGSR::UpdateWiFiPower(uint8_t PWRIndex){
    wifi_power_t CW=WiFi.getTxPower();
    bool B;

    if (PWRIndex > 0) B = WiFi.setTxPower(RawWiFiTX[PWRIndex - 1]);
    else if (CW != GSRWiFi.TransmitPower && GSRWiFi.TransmitPower != 0) B = WiFi.setTxPower(GSRWiFi.TransmitPower);
}

wl_status_t WatchyGSR::currentWiFi(){
    if (!GetAskWiFi()) return WL_CONNECT_FAILED;
    if (HasIPAddress()) return WL_CONNECTED;
    if (GSRWiFi.Working) return WL_IDLE_STATUS;  // Make like it is relaxing doing nothing.
    return WL_CONNECT_FAILED;
}
wl_status_t WatchyGSR::WiFiStatus(){ if(GetAskWiFi()) return WiFi.status(); else return WL_DISCONNECTED; }

void WatchyGSR::WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
    GSRWiFi.Last = millis() + 1000;
}

String WatchyGSR::buildWiFiAPPage(){
    String S = LGSR.LangString(wifiIndexA,true,Options.LanguageID,14,14);
    String T;
    uint8_t I, J;

    for (I = 0; I < 10; I++){
        T = wifiIndexB;
        T.replace("$",String(char(65 + I)));
        T.replace("#",String(char(48 + I)));
        T.replace("?",APIDtoString(I));
        S += T;

        T = LGSR.LangString(wifiIndexC,true,Options.LanguageID,15,16);
        T.replace("#",String(char(48 + I)));
        T.replace("$",PASStoString(I));
        S += T;

        for (J = 0; J < 12; J++){
            T = wifiIndexC1;
            T.replace("#",String(J));
            T.replace("%",(J == GSRWiFi.AP[I].TPWRIndex ? " selected" : ""));
            T.replace("$",(J > 0 ? IDWiFiTX[J - 1] : "* " + LGSR.GetWebID(Options.LanguageID,0) + " *"));
            S += T;
        }
        T = wifiIndexC2;
        S += (T + (I < 9 ? "</tr>" : ""));
    }
    return S + LGSR.LangString(wifiIndexD,true,Options.LanguageID,6,6);
}

void WatchyGSR::parseWiFiPageArg(String ARG, String DATA){
    uint8_t I = String(ARG.charAt(2)).toInt();
    String S = ARG.substring(0,2);

    if      (S == "AP") strncpy(&GSRWiFi.AP[I].APID[0], DATA.c_str(),33);
    else if (S == "PA") strncpy(&GSRWiFi.AP[I].PASS[0], DATA.c_str(),65);
    else if (S == "TX") GSRWiFi.AP[I].TPWRIndex = DATA.toInt();
}

String WatchyGSR::APIDtoString(uint8_t Index){
    String S = "";
    uint8_t I = 0;
    while (GSRWiFi.AP[Index].APID[I] != 0 && I < 32) { S += char(GSRWiFi.AP[Index].APID[I]); I++; }
    return S;
}

String WatchyGSR::PASStoString(uint8_t Index){
    String S = "";
    uint8_t I = 0;
    while (GSRWiFi.AP[Index].PASS[I] != 0 && I < 63) { S += char(GSRWiFi.AP[Index].PASS[I]); I++; }
    return S;
}

void WatchyGSR::initZeros(){
    String S = "";
    uint8_t I, J;
    GuiMode = GSR_WATCHON;
    VibeMode = false;
    WatchyStatus = "";
    WatchTime.TimeZone = "";
    OP.init();
    Menu.Style = GSR_MENU_INNORMAL;
    Menu.Item = 0;
    Menu.SubItem = 0;
    Menu.SubSubItem = 0;
    NTPData.Pause = 0;
    NTPData.Wait = 0;
    Battery.Read = (rawBatteryVoltage() + 100) / 100;
    Battery.Level = -2;
    ActiveMode = false;
    OTATry = 0;
    OTAEnd = false;
    OTAUpdate = false;
    OTATimer = millis();
    WatchyAPOn = false;
    DoHaptic = false;
    Steps.Reset=false;
    Steps.Yesterday = 0;
    Steps.Cached = 0;
    Alarms_Active[0]=0;
    Alarms_Active[1]=0;
    Alarms_Active[2]=0;
    Alarms_Active[3]=0;
    Alarms_Times[0]=0;
    Alarms_Times[1]=0;
    Alarms_Times[2]=0;
    Alarms_Times[3]=0;
    Alarms_Playing[0]=0;
    Alarms_Playing[1]=0;
    Alarms_Playing[2]=0;
    Alarms_Playing[3]=0;
    Alarms_Repeats[0]=0;
    Alarms_Repeats[1]=0;
    Alarms_Repeats[2]=0;
    Alarms_Repeats[3]=0;
    GSRWiFi.Requested=false;
    GSRWiFi.Working=false;
    GSRWiFi.Results=false;
    GSRWiFi.Index=0;
    Updates.Full=true;
    Updates.Drawn=true;
    for (I = 0; I < 10; I++)
    {
      GSRWiFi.AP[I].TPWRIndex=0;
      memset(&GSRWiFi.AP[I].APID[0], '\0',33);
      memset(&GSRWiFi.AP[I].PASS[0], '\0',65);
    }
    NTPData.TimeZone=false;
    NTPData.UpdateUTC=true;
    NTPData.State=0;
    NTPData.TravelTemp = 0;
    TimerUp.SetAt = WatchTime.UTC_RAW;
    TimerUp.StopAt = TimerUp.SetAt;
    TimerDown.MaxMins = 0;
    TimerDown.MaxHours = 0;
    TimerDown.Mins = 0;
    TimerDown.Hours = 0;
    TimerDown.MaxTones = 0;
    TimerDown.Active = false;
    DefaultWatchStyles = true;
    WatchStyles.Count = 0;
    BasicWatchStyles = -1;
    for (I = 0; I < GSR_MaxStyles; I++) { WatchStyles.Options[I]=0; WatchStyles.AddOn[I] = nullptr; BootAddOns.Inits[I] = nullptr; } BootAddOns.Count = 0;
    WeatherData.Ready = false;
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
    WeatherData.LastLon = 0;
    WeatherData.LastLat = 0;
    WeatherData.StaticLon = 0;
    WeatherData.StaticLat = 0;
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

String WatchyGSR::GetSettings(){
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

    I[J] = 139; J++; // New Version.
    I[J] = (Steps.Hour); J++;
    I[J] = (Steps.Minutes); J++;
    K  = Options.TwentyFour ? 1 : 0;
    K |= Options.LightMode ? 2 : 0;
    K |= Options.Feedback ? 4 : 0;
    K |= Options.Border ? 8 : 0;
    K |= Options.Lefty ? 16 : 0;
    K |= Options.Swapped ? 32 : 0;
    K |= Options.Orientated ? 64 : 0;
    //K |= Options.UsingDrift ? 128 : 0;
    I[J] = (K); J++;
    // Version 129, 137
    DWordtoUSC(SRTC.getDrift(false),I[J + 3], I[J + 2], I[J + 1], I[J]); J+=4; /* 129 */
    // Version 137
    DWordtoUSC(SRTC.getDrift(true),I[J + 3], I[J + 2], I[J + 1], I[J]);  J+=4; /* 137 */

    W = ((Options.Performance & 15) << 4);
    I[J] = (Options.SleepStyle | W); J++;
    I[J] = (Options.SleepMode); J++;
    I[J] = (Options.SleepStart); J++;
    I[J] = (Options.SleepEnd); J++;
    // End Version 129.
    // Version 130.
    I[J] = (getTXOffset(GSRWiFi.TransmitPower)); J++;
    K  = (Options.BedTimeOrientation ? 1 : 0) | (WeatherData.Metric ? 2 : 0);
    I[J] = (K); J++;
    // End Version 130.
    // Version 131.
    I[J] = Options.WatchFaceStyle; J++;
    I[J] = Options.Game; J++;  /* 136 */
    I[J] = Options.LanguageID; J++;
    // End Version 131.

    V = (Options.MasterRepeats << 5); I[J] = (Options.Turbo | V); J++;

    V = (TimerDown.MaxTones << 5);
    I[J] = ((TimerDown.MaxHours) | V); J++;
    I[J] = (TimerDown.MaxMins); J++;
    I[J] = ((TimerDown.MaxSecs) | (TimerDown.Repeats ? 128 : 0)); J++;

    for (K = 0; K < 4; K++){
        V = (Alarms_Repeats[K] << 5);
        I[J] = (Alarms_Hour[K] | V); J++;
        I[J] = (Alarms_Minutes[K]); J++;
        V = (Alarms_Active[K] & GSR_ALARM_NOTRIGGER);
        WordtoUSC(V,I[J + 1],I[J]); J+=2; //  I[J] = (V & 255); J++; I[J] = ((V >> 8) & 255); J++;
    }

    I[J] = (SRTC.isFastDrift(false) ? 1 : 0); J++;
    I[J] = (NTPData.AutoSync ? 2 : 0) + (WatchTime.ESPRTC ? 4 : 0) + (SRTC.isFastDrift(true) ? 8 : 0); J++;  /* The New Drift */
    I[J] = NTPData.SyncHour; J++;
    I[J] = NTPData.SyncMins; J++;
    I[J] = NTPData.SyncDays; J++;
    // Version 139
    S = String(WeatherData.StaticLat,6); X = S.length();
    for (Y = 0; Y < 13; Y++){
        if (Y < X) I[J] = S.charAt(Y); else I[J] = 0;
        J++;
    }

    S = String(WeatherData.StaticLon,6); X = S.length();
    for (Y = 0; Y < 13; Y++){
        if (Y < X) I[J] = S.charAt(Y); else I[J] = 0;
        J++;
    }
    // End Version 139
    I[J]=WeatherData.Interval; J++;

    for (X = 0; X < 10; X++){
        S = APIDtoString(X);
        if (S > "") {
            I[J] = GSRWiFi.AP[X].TPWRIndex; J++; // Store the WiFi power per AP.
            W = S.length();
            I[J] = W; J++;
            for (Y = 0; Y < W; Y++){
                I[J] = S.charAt(Y); J++;
            }
            S = PASStoString(X);
            W = S.length();
            I[J] = W; J++;
            for (Y = 0; Y < W; Y++){
                I[J] = S.charAt(Y); J++;
            }
        }
    }
    I[J] = 0; J++;

    mbedtls_base64_encode(&O[0], 2047, &L, &I[0], J);

    O[L]=0;
    S = reinterpret_cast<const char *>(O);
    return S;
}

void WatchyGSR::StoreSettings(String FromUser){
    unsigned char O[2048], E[2048];
    int K = 0;
    int J = 0;
    uint16_t V;
    size_t L;
    bool Ok;
    int64_t DI, DX;
    bool FI, FX;
    uint8_t I, A, W, NewV;  // For WiFi 
    String S, T;

    DI = 0; DX = 0;
    J = FromUser.length(); if (J < 5) return;
    for (K = 0; K < J; K++) E[K] = FromUser.charAt(K); NewV = 0;

    mbedtls_base64_decode(&O[0], 2047, &L, &E[0], J); L--;  // Take dead zero off end.

    J = 0; if (L > J && O[J] > 128) {NewV = O[J]; J++; }  // Detect NewVersion and go past.
           if (L > J) Steps.Hour = constrain(O[J],0,23);
    J++;   if (L > J) Steps.Minutes = constrain(O[J],0,59);
    J++;
    if (L > J) {
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
    }
    if (NewV > 128){
         J++; if (L > J + 1){
              if (NewV > 136){  // 137 introduces 32bit drifts for both external and internal.
                DX = USCtoDWord(O[J + 3], O[J + 2], O[J + 1], O[J]); J+=4;
                DI = USCtoDWord(O[J + 3], O[J + 2], O[J + 1], O[J]); J+=3; /* 137 */
              }else{
                if (NewV < 133) J++;  // 129 had 16bit drift.
                if (NewV > 133 && NewV < 137) J++;  // 133 introduced 24bit drift.
              }
         }
         J++; if (L > J){
            V = ((O[J] & 240) >> 4); Options.Performance = constrain(V,0,2);
            Options.SleepStyle = constrain((O[J] & 7),0,4);
            if (Options.SleepStyle && HWVer == 3.0f) Options.SleepStyle = 2;
#ifndef STABLEBMA_H_INCLUDED
            if (Options.SleepStyle) Options.SleepStyle = 2;
#endif
         }
         J++; if (L > J) Options.SleepMode = constrain(O[J],1,10);
         J++; if (L > J) Options.SleepStart = constrain(O[J],0,23);
         J++; if (L > J) Options.SleepEnd = constrain(O[J],0,23);
         if (NewV > 129){
            J++; if (L > J){ V = constrain(O[J],0,10); GSRWiFi.TransmitPower = RawWiFiTX[V]; }
            J++; if (L > J){ V = O[J]; Options.BedTimeOrientation = (V & 1) ? true : false; WeatherData.Metric = (V & 2) ? true : false; }
         }
         if (NewV > 130){
            J++; if (L > J){ V = constrain(O[J],0,WatchStyles.Count - 1); Options.WatchFaceStyle = V; }
            if (NewV > 135){
                J++; if (L > J){ V = constrain(O[J],0,WatchStyles.Count - 1); Options.Game = V; if (!(WatchStyles.Options[V] & GSR_GAM)) Options.Game = 255; }
            }
         }
         if (NewV > 131){
            J++; if (L > J) { V = constrain(O[J],0,LGSR.MaxLangID()); Options.LanguageID = V; }
         }
    }
    J++; if (L > J){
        V = ((O[J] & 224) >> 5);
        Options.MasterRepeats = constrain(V,0,4);
        Options.Turbo = constrain((O[J] & 31),0,10);
    }
    J++; if (L > J){
        V = ((O[J] & 224) >> 5);
        TimerDown.MaxTones = constrain(V,0,4);
        TimerDown.MaxHours = constrain((O[J] & 31),0,23);
    }
    J++; if (L > J) TimerDown.MaxMins = constrain(O[J],0,59);
    if (NewV > 132){
        J++; if (L > J) { TimerDown.MaxSecs = constrain((O[J] & 127),0,59); TimerDown.Repeats = ((O[J] & 128) == 128); }
    }

    for (K = 0; K < 4; K++){
        J++; if (L > J){
            V = ((O[J] & 224) >> 5);
            Alarms_Repeats[K] = constrain(V,0,4);
            Alarms_Hour[K] = constrain((O[J] & 31),0,23);
        }
        J++; if (L > J) Alarms_Minutes[K] = constrain(O[J],0,59);
        J++; if (L > J + 1){
            Alarms_Active[K] = (USCtoWord(O[J + 1], O[J]) & GSR_ALARM_NOTRIGGER); J++;
        }
    }

    if (NewV > 133){
        J++; if (L > J) {
            if ((O[J] & 1) == 1) DX = 0 - DX;
            J++; if (NewV > 136) if ((O[J] & 8) == 8) DI = 0 - DI;
            NTPData.AutoSync = ((O[J] & 2) == 2); (FX = (O[J] & 1) == 1); (FI = (O[J] & 8) == 8); WatchTime.ESPRTC = ((O[J] & 4) == 4); SRTC.useESP32(WatchTime.ESPRTC);
        }
        J++; if (L > J) { NTPData.SyncHour = O[J]; J++; } // Test
        if (L > J) { NTPData.SyncMins = O[J]; J++; } // Test
        if (NewV >137 && L > J) { NTPData.SyncDays = O[J]; J++; }
        if (!NTPData.SyncDays) NTPData.SyncDays = 1;
        if (NewV > 134){
            if (NewV > 138){
                S = ""; for (I = 0; I < 13; I++) { if (O[J]) S += String(char(O[J])); J++; }
                T = ""; for (I = 0; I < 13; I++) { if (O[J]) T += String(char(O[J])); J++; }
                WeatherData.UseStaticPOS = (S.length() && T.length());
                WeatherData.StaticLat = S.toDouble();
                WeatherData.StaticLon = T.toDouble();
            } else J+=32;
            WeatherData.Interval = constrain(O[J],0,144);
        }
    }

    S = ""; for (I = 0; I < 10; I++) { memset(&GSRWiFi.AP[I].APID[0],'\0',33); memset(&GSRWiFi.AP[I].PASS[0],'\0',65); }

    J++; A = 0;
    while (L > J){
        Ok = false;
        if (L > J){ // get APx
            if (NewV > 129) { GSRWiFi.AP[A].TPWRIndex=constrain(O[J],0,10); J++; }
            W = O[J]; J++; S = "";
            if (L > J + (W - 1)) {  // Read APx.
                for (I = 0; L > J && I < W; I++){
                     S += String(char(O[J])); J++;
                }
                memcpy(&GSRWiFi.AP[A].APID,S.c_str(),strlen(S.c_str()));
                Ok = true;
            }
            if (L > J){ // get APx
                W = O[J]; J++; S = "";
                if (L > J + (W - 1)) {  // Read PAx.
                    for (I = 0; L > J && I < W; I++){
                        S += String(char(O[J])); J++;
                    }
                    memcpy(&GSRWiFi.AP[A].PASS,S.c_str(),strlen(S.c_str()));
                    Ok = true;
                }
            }
        }
        if (Ok) A++;
    }

if (DX < 0) DX = 0 - DX;
if (DI < 0) DI = 0 - DI;
    SRTC.setDrift(DI,FI,true);
    SRTC.setDrift(DX,FX,false);
}

uint16_t WatchyGSR::USCtoWord(unsigned char High, unsigned char Low){
  unsigned char B[2];
  uint16_t V = 0;
  B[1]=High; B[0]=Low; memcpy(&V,&B[0],2);
  return V;
}
void WatchyGSR::WordtoUSC(uint16_t Word, unsigned char &High, unsigned char &Low){
  unsigned char B[2];
  memcpy(&B,&Word,2); High = B[1]; Low = B[0];
}
uint32_t WatchyGSR::USCtoDWord(unsigned char HHigh, unsigned char HLow, unsigned char LHigh, unsigned char LLow){
  unsigned char B[4];
  uint32_t V = 0;
  B[3]=HHigh; B[2]=HLow; B[1]=LHigh; B[0]=LLow; memcpy(&V,&B[0],4);
  return V;
}
void WatchyGSR::DWordtoUSC(uint32_t DWord, unsigned char &HHigh, unsigned char &HLow, unsigned char &LHigh, unsigned char &LLow){
  unsigned char B[4];
  memcpy(&B[0],&DWord,4); HHigh = B[3]; HLow = B[2]; LHigh = B[1]; LLow = B[0];
}

// NVS code.
void WatchyGSR::RetrieveSettings(){
    if (OkNVS(GName)){
        String S = NVS.getString(GSettings);
        StoreSettings(S);
    }
    Options.NeedsSaving = false;
}

void WatchyGSR::RecordSettings(){
    bool B = true;
    if (OkNVS(GName)) {
      if (Options.Game != 255 && WatchStyles.AddOn[Options.Game] != nullptr) WatchStyles.AddOn[Options.Game]->SaveProgress();
      B = NVS.setString(GSettings,GetSettings()); NVS.commit();
    }
    Options.NeedsSaving = !B;
}

bool WatchyGSR::OkNVS(String FaceName){
    String S = NVS.getString("NoNVS");
    String R = ".#.";
    R.replace("#",FaceName);
    return (S.indexOf(R) < 0);
}

void WatchyGSR::SetNVS(String FaceName, bool Enabled){
    String S = NVS.getString("NoNVS");
    String R = ".#.";
    bool B;
    R.replace("#",FaceName);
    int I = S.indexOf(R);
    if (!Enabled) {
        if (I == 0) {
            S += R;
            B = NVS.setString("NoNVS",S);
        }else if (I > -1){
            S.replace(R,"");
            B = NVS.setString("NoNVS",S);
        }
        if (B) NVS.commit();
    }
}

void WatchyGSR::NVSEmpty(){
    NVS.erase(GSettings);
    NVS.erase(GTZ);
    NVS.commit();
}

// Turbo Mode!
void WatchyGSR::SetTurbo(){ LastButton = millis(); if (Battery.Read > Battery.MinLevel) TurboTime = LastButton; }
bool WatchyGSR::InTurbo() { return (Options.Turbo > 0 && TurboTime != 0 && millis() - TurboTime < (Options.Turbo * 1000)); }

bool WatchyGSR::UpRight() {
#ifdef STABLEBMA_H_INCLUDED
    if (Options.Orientated || (WatchTime.BedTime && Options.BedTimeOrientation)) return SBMA.IsUp();
#endif
    return true;  // Fake it til you make it.
}

bool WatchyGSR::DarkWait(){
    bool B = AlarmsOn || (Options.SleepStyle > 0 && Darkness.Last != 0 && (millis() - Darkness.Last) < (Options.SleepMode * 1000));
        if (Options.SleepStyle == 2){
            if (!WatchTime.BedTime) return AlarmsOn;
            return B;
        }else if (GuiMode != GSR_MENUON && Options.SleepStyle > 0 && Options.SleepStyle != 4) return B;
    return AlarmsOn;
}

bool WatchyGSR::Showing() {
    bool B = (Updates.Full || Alarming || SoundHandle != NULL || SoundStart);
    if (Options.SleepStyle > 0){
        B |= (Darkness.Last > 0 && (millis() - Darkness.Last) < (Options.SleepMode * 1000));
        if (Options.SleepStyle == 1){
            return (B | (GuiMode == GSR_MENUON)); // Hide because it isn't checking the rest.
        }
        if (Options.SleepStyle == 2){
            if (B) return B;
            if (Options.SleepEnd > Options.SleepStart) { if (WatchTime.Local.Hour >= Options.SleepStart && WatchTime.Local.Hour < Options.SleepEnd) return false; }
            else if (WatchTime.Local.Hour >= Options.SleepStart || WatchTime.Local.Hour < Options.SleepEnd) return false;
        }else if (Options.SleepStyle > 2 && Options.SleepStyle != 4) return B;
    }
    return true;
}

void WatchyGSR::RefreshCPU(){ RefreshCPU(0); }
void WatchyGSR::RefreshCPU(int Value){
    uint32_t C = 80;
    if (Battery.Read > Battery.MinLevel) {
        if (Value == GSR_CPUMAX) CPUSet.Locked = true;
        if (Value == GSR_CPUDEF) CPUSet.Locked = false;
        if (!CPUSet.Locked && Options.Performance != 2 && Value != GSR_CPULOW) C = (InTurbo() || Value == GSR_CPUMID) ? 160 : 80;
        if (WatchyAPOn || OTAUpdate || GSRWiFi.Requests > 0 || CPUSet.Locked || (Options.Performance == 0 && Value != GSR_CPULOW)) C = 240;
    }
    if (C != CPUSet.Freq) if (setCpuFrequencyMhz(C)); CPUSet.Freq = C;
}

/* This is here so that all ESP.restarts store the step counter */
void WatchyGSR::Reboot(){
    bool B;
    if (OkNVS(GName)){
        if (Options.NeedsSaving) RecordSettings();
        B = NVS.setString(STP,String(CurrentStepCount()));
        B = NVS.setString(YSTP,String(Steps.Yesterday));
        NVS.commit();
        NVS.close();
    }
    KeysStop();
    if (GSRHandle != NULL) vTaskDelete(GSRHandle);
    if (SoundHandle != NULL) vTaskDelete(SoundHandle);
    vTaskDelay(100/portTICK_PERIOD_MS);
    ESP.restart();
}

bool WatchyGSR::OTA(){
    esp_partition_iterator_t IT = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);
    if (IT != NULL){
      const esp_partition_t *Part = esp_partition_get(IT);
      uint64_t Size = Part->size;
      esp_partition_iterator_release(IT);
      return (Size >= 0x1E0000);
    }
    return false;
}

// Function to find the existing WiFi power in the static index.

uint8_t WatchyGSR::getTXOffset(wifi_power_t Current){
    uint8_t I;
    for (I = 0; I < 11; I++){ if (RawWiFiTX[I] == Current) return I; }
    return 0;
}

void WatchyGSR::DisplayInit(bool ForceDark){
#ifdef GxEPD2DarkBorder
  display.epd2.setDarkBorder(Options.Border | ForceDark);
#endif
  if (Updates.Init){
    WatchyGSR::hspi.begin(GSR_PIN_SCK,GSR_PIN_MISO,GSR_PIN_MOSI,GSR_PIN_SS);
    display.epd2.selectSPI(hspi, SPISettings(20000000, MSBFIRST, SPI_MODE0));
    display.init(0,Rebooted,10,true);  // Force it here so it fixes the border.
    Updates.Init=false;
    Rebooted=false;
  }
}

void WatchyGSR::DisplayWake(bool Tapped){ Darkness.Woke=true; Darkness.Last=millis(); Darkness.Tilt = Darkness.Last; UpdateDisp = true; if (Tapped) Updates.Tapped=true; }

void WatchyGSR::DisplaySleep(){ if (!Updates.Init) { Updates.Init = true; display.hibernate(); } }

bool WatchyGSR::SafeToDraw() { return (!(OTAUpdate || WatchyAPOn || (Menu.Item == GSR_MENU_TOFF && Menu.SubItem > 0 && Menu.SubItem < 6))); }
bool WatchyGSR::NoMenu() { return (GuiMode == GSR_WATCHON || GuiMode == GSR_GAMEON); };
bool WatchyGSR::InGame() { return (GuiMode == GSR_GAMEON); }

/* Weather Functions */

void WatchyGSR::StartWeather(){
  if (WeatherData.State == 0 && NTPData.State < 2 && GSRHandle == NULL && GetWantWeather() && Battery.Read > Battery.RadioLevel){
    WeatherData.State = 1;
    WeatherData.Pause = 2;
    AskForWiFi();
  }
}

bool WatchyGSR::IsWeatherAvailable() { return WeatherData.Ready; }
int WatchyGSR::GetWeatherTemperature(){
  int T = 255;
  if (WeatherData.Ready) T = int(WeatherData.Weather.Temperature.Current); else T = SRTC.temperature();
  if (T != 255 && !WeatherData.Metric) T = (T * 1.8) + 32;
  return T;
}
int WatchyGSR::GetWeatherTemperatureFeelsLike(){
  int T = 255;
  if (WeatherData.Ready) T = int(WeatherData.Weather.Temperature.FeelsLike); else T = SRTC.temperature();
  if (T != 255 && !WeatherData.Metric) T = (T * 1.8) + 32;
  return T;
}
void WatchyGSR::SetWeatherScale(bool Metric) { WeatherData.Metric = Metric; }
bool WatchyGSR::IsMetric() { return WeatherData.Metric; }
int WatchyGSR::GetWeatherID() { return WeatherData.Weather.ID; }
uint8_t WatchyGSR::GetWeatherHumidity() { return WeatherData.Weather.Humidity; }
uint8_t WatchyGSR::GetWeatherClouds() { return WeatherData.Weather.Clouds; }
time_t WatchyGSR::GetWeatherSunRise() { return WeatherData.Weather.SunRise; }
time_t WatchyGSR::GetWeatherSunSet() { return WeatherData.Weather.SunSet; }
uint16_t WatchyGSR::GetWeatherPressure() { return WeatherData.Weather.Pressure; }
uint32_t WatchyGSR::GetWeatherVisibility() { if (WeatherData.Metric) return WeatherData.Weather.Visibility; else return WeatherData.Weather.Visibility * 0.6214; }
float WatchyGSR::GetWeatherWindSpeed() { if (WeatherData.Metric) return WeatherData.Weather.WindSpeed; else return WeatherData.Weather.WindSpeed * 0.6214; }
float WatchyGSR::GetWeatherWindDirection() { return WeatherData.Weather.WindDirection; }
float WatchyGSR::GetWeatherWindGust() { if (WeatherData.Metric) return WeatherData.Weather.WindGust; else return WeatherData.Weather.WindGust * 0.6214; }
bool WatchyGSR::GetWebAvailable(){ return (GSRHandle == NULL); }
bool WatchyGSR::GetWebReady() { return GSRWebData.Ready; }
int WatchyGSR::GetWebResponse() { return GSRWebData.Response; }
String WatchyGSR::GetWebData() { return GSRWebData.Data; }
bool WatchyGSR::AskForWeb(String website, uint8_t Timeout){
  if (!GetWebAvailable()) return false;
  GSRWebData.webURL = website;
  GSRWebData.secTimeout = Timeout;
  GSRRet = xTaskCreate(WatchyGSR::GSRWebGet,"GSRWebGet",20480,NULL,(configMAX_PRIORITIES - 1),&GSRHandle);
  return (GSRHandle != NULL);
}

String WatchyGSR::CleanString(String Clean){
  Clean.replace('"',' ');
  Clean.trim();
  return Clean;
}

String WatchyGSR::makeGeo(String inGeo, bool isLat){
  double _g = inGeo.toDouble();
  String _s = String(_g, 6);
  if (inGeo.length() == 0) return inGeo;
  if (isLat){
     if (_g > 90.0f || _g < -90.0f) return "";
  }else{
     if (_g > 180.0f || _g < -180.0f) return "";
  }
  return _s;
}

void WatchyGSR::ProcessWeather(){
  String S,T , payload;
  struct tm sunTime;
  JSONVar root;
  if (UpdateDisp || GSRWiFi.Slow > 0 && !inBrownOut()) return;

  switch(WeatherData.State){

    case 1:{
      if (WiFiInProgress() && GSRWiFi.Results){
          if(WeatherData.Wait > 2){
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
    case 2:{
      if (WeatherData.UseStaticPOS){
          WeatherData.LastLon = WeatherData.StaticLon;
          WeatherData.LastLat = WeatherData.StaticLat;
          WeatherData.State = 4;
          break;
      }
      if (WiFiStatus() != WL_CONNECTED){
          if(currentWiFi() == WL_CONNECT_FAILED || WeatherData.Wait > 2){
              WeatherData.Pause = 0;
              WeatherData.State = 99;
              break;
          }
          WeatherData.Pause = 5;
          if (WeatherData.Wait > 2){
              WeatherData.Pause = 0;
              WeatherData.State = 99;
          }
          break;
      }
      if (!GetWebAvailable()) break;  // Break out incase a URL request is going.

      setStatus("GL");
      HTTP.setUserAgent(UserAgent);
      if (!AskForWeb("http://ip-api.com/json/?fields=lat,lon")){
          WeatherData.Pause = 0;
          WeatherData.State = 99;
      }else{
          WeatherData.State++;
          // Do the next part.
          WeatherData.Wait = 0;
          WeatherData.Pause = 1;
      }
      break;
  }

    case 3:{
      if (WiFiStatus() == WL_DISCONNECTED){
          WeatherData.Pause = 0;
          WeatherData.State = 99;
          break;
      }
      if (GetWebReady()) {
        WeatherData.Wait = 0;
        WeatherData.Pause = 0;
        payload = GetWebData();
        root = JSON.parse(payload);
        T = CleanString(JSON.stringify(root["lat"]));
        S = CleanString(JSON.stringify(root["lon"]));
        if (S && T){
          WeatherData.State++;
          WeatherData.LastLon = S.toDouble();
          WeatherData.LastLat = T.toDouble();
        } else WeatherData.State = 99;
      }else if (WeatherData.Wait > 0 || GetWebAvailable() || (GetWebResponse() > 0 && GetWebResponse() != 200)){
          WeatherData.Pause = 0;
          WeatherData.State = 99;
      }
      break;
    }

    // Am I Connected?  If so, ask for Weather now, since have GeoLocate.
    case 4:{
      if (WiFiStatus() != WL_CONNECTED){
          if(currentWiFi() == WL_CONNECT_FAILED || WeatherData.Wait > 1) WeatherData.State = 99;
          break;
      }

      if (!GetWebAvailable()) break;  // Break out incase a URL request is going.
      WeatherData.Pause = 1;
      setStatus("WE");
      if (!AskForWeb("http://api.open-meteo.com/v1/forecast?latitude=" + String(WeatherData.LastLat,6) + "&longitude=" + String(WeatherData.LastLon,6) + "&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code,cloud_cover,surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m&hourly=visibility&daily=sunrise,sunset&timezone=UTC&forecast_days=1&forecast_hours=1")){
          WeatherData.State = 99;
      }else{
          WeatherData.State++;
          // Do the next part.
          WeatherData.Wait = 0;
      }
      break;
  }

    case 5:{
      if (WiFiStatus() == WL_DISCONNECTED){
          WeatherData.Pause = 0;
          WeatherData.State = 99;
          break;
      }
      if (GetWebReady()) {
        WeatherData.Wait = 0;
        WeatherData.Pause = 0;
        payload = GetWebData();
        root = JSON.parse(payload);
        S = CleanString(JSON.stringify(root["error"]));
        if (S == "null"){
            S = CleanString(JSON.stringify(root["current"]["temperature_2m"]));
            WeatherData.Weather.Temperature.Current = (S.toFloat());
            S = CleanString(JSON.stringify(root["current"]["apparent_temperature"]));
            WeatherData.Weather.Temperature.FeelsLike = (S.toFloat());
            S = CleanString(JSON.stringify(root["current"]["relative_humidity_2m"]));
            WeatherData.Weather.Humidity = uint8_t(S.toInt());
            S = CleanString(JSON.stringify(root["current"]["cloud_cover"]));
            WeatherData.Weather.Clouds = uint8_t(S.toInt());
            S = CleanString(JSON.stringify(root["current"]["surface_pressure"]));
            WeatherData.Weather.Pressure = uint16_t(S.toInt());
            S = CleanString(JSON.stringify(root["current"]["weather_code"]));
            WeatherData.Weather.ID = S.toFloat();
            S = CleanString(JSON.stringify(root["current"]["wind_speed_10m"]));
            WeatherData.Weather.WindSpeed = S.toFloat();
            S = CleanString(JSON.stringify(root["current"]["wind_gusts_10m"]));
            WeatherData.Weather.WindGust = S.toFloat();
            S = CleanString(JSON.stringify(root["current"]["wind_direction_10m"]));
            WeatherData.Weather.WindDirection = S.toInt();
            S = CleanString(JSON.stringify(root["daily"]["sunrise"]));
            WeatherData.Weather.SunRise = getISO8601(S);
            S = CleanString(JSON.stringify(root["daily"]["sunset"]));
            WeatherData.Weather.SunSet = getISO8601(S);
            S = CleanString(JSON.stringify(root["hourly"]["visibility"]));
            WeatherData.Weather.Visibility = uint32_t(S.toInt());
            WeatherData.Ready = true;
        }
        WeatherData.Pause = 0;
        WeatherData.State = 99;
      }else if (WeatherData.Wait > 0 || GetWebAvailable() || (GetWebResponse() > 0 && GetWebResponse() != 200)){
          WeatherData.Pause = 0;
          WeatherData.State = 99;
      }
      break;
    }

    case 99:{
      WeatherData.Wait = 0;
      WeatherData.Pause = 0;
      WeatherData.State = 0;
      endWiFi();
      WeatherData.LastCall = WatchTime.UTC_RAW - (WatchTime.UTC_RAW % 60) + (WeatherData.Interval * 300); // 15 minutes.
      setStatus("");
    }
  }
}

void WatchyGSR::GSRWebGet(void * parameter){
vTaskDelay(10/portTICK_PERIOD_MS);
int I = 0;
GSRWebData.Response = 0;
GSRWebData.Ready = false;
GSRWebData.Data = "";
bool Good = ((WatchStyles.Options[Options.WatchFaceStyle] & GSR_AFW) && WiFi.status() == WL_CONNECTED);
bool Sent = false;
uint16_t webTimeout = (golow(GSRWebData.secTimeout, 1) + 1) * 1000;
unsigned long Stay = millis() + webTimeout;
    while (Good && millis() < Stay){
        Good = ((WatchStyles.Options[Options.WatchFaceStyle] & GSR_AFW) && WiFi.status() == WL_CONNECTED);
        if (!Sent && Good && !inBrownOut()) {
            vTaskDelay(10/portTICK_PERIOD_MS);    // 10ms pauses.
            Sent = true; HTTP.setConnectTimeout(webTimeout); HTTP.begin(WiFiC, GSRWebData.webURL); 
            vTaskDelay(10/portTICK_PERIOD_MS);    // 10ms pauses.
        }
        if (GSRWebData.Response == HTTP_CODE_OK) {
            vTaskDelay(10/portTICK_PERIOD_MS);    // 10ms pauses.
            GSRWebData.Data= HTTP.getString();
            vTaskDelay(10/portTICK_PERIOD_MS);    // 10ms pauses.
            GSRWebData.Ready = true;
            vTaskDelay(10/portTICK_PERIOD_MS);    // 10ms pauses.
            Good = false;
        }
        if (Good) vTaskDelay(100/portTICK_PERIOD_MS);    // 100ms pauses.
        if (Good && !inBrownOut()) {
            I = HTTP.GET(); if (I) { GSRWebData.Response = I; if (I != HTTP_CODE_OK) { LastWebError = I; Good = false; } }
        }
    }
    HTTP.end();
    GSRHandle = NULL;
    vTaskDelete(NULL);
}

void WatchyGSR::getAngle(uint16_t Angle, uint8_t Width, uint8_t Height, uint8_t &OutX, uint8_t &OutY){
    double A, mTan, mY, mX;
    uint8_t cX = Width/2; /* Thanks to Henny022 on the Watchy Discord for this code, I did neaten it up for readabilty more. */
    uint8_t cY = Height/2;
    Angle %= 360; // Trim angle (shouldn't ever happen, but hey).
    A = ((2 * M_PI) / 360) * Angle;
    mTan = tan(A);

    mY = (A < M_PI_2 || A > 3*M_PI_2) ? -cY : cY;
    mX = -mTan * mY;

    if(mX > cX || mX < -cX)
    {
        mX = (A < M_PI) ? cX : -cX;
        mY = -mX / mTan;
    }

    OutX = mX + cX;
    OutY = mY + cY;
    OutX = OutX - ((OutX == Width) ? 1:0);
    OutY = OutY - ((OutY == Height) ? 1:0);
}

void WatchyGSR::WatchFaceStart(uint8_t NewFace, bool NoEndWiFi){
    if (NoEndWiFi){
      GSRWiFi.Requests = 0;
      GSRWiFi.Requested = false;
      GSRWiFi.Index = 0;
    }else{
      GSRWiFi.Requests = 0; /* Fix to stop WiFi from running */
      endWiFi();
    }
    Options.WatchFaceStyle = NewFace;
    if (Options.WatchFaceStyle > WatchStyles.Count - 1) Options.WatchFaceStyle = 0;
    WatchStyles.Options[Options.WatchFaceStyle] = 0;
    if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr) initWatchFaceStyle(); else WatchStyles.AddOn[Options.WatchFaceStyle]->initWatchFaceStyle();
    if (NoEndWiFi && !GetAskWiFi()) AskForWiFi();
}

bool WatchyGSR::ChangeWatchFace(bool Up){
byte I = Options.WatchFaceStyle;
byte C = 0;
  while (C < WatchStyles.Count){
    I = roller(I + (Up ? 1 : -1),0,WatchStyles.Count - 1); C++;
    if (!(WatchStyles.Options[I] & GSR_GAM)) C = WatchStyles.Count;
  }
  if (I != Options.WatchFaceStyle && !(WatchStyles.Options[I] & GSR_GAM)){
    WatchFaceEnd();
    WatchFaceStart(I);
    Options.NeedsSaving = true;
    return true;
  }
  return false;
}

bool WatchyGSR::ChangeGame(bool Up){
byte I = (Options.Game == 255 ? 0 : Options.Game);
byte C = 0;
  while (C < WatchStyles.Count){
    I = roller(I + (Up ? 1 : -1),0,WatchStyles.Count - 1); C++;
    if (WatchStyles.Options[I] & GSR_GAM) C = WatchStyles.Count;
  }
  if (I != Options.Game && (WatchStyles.Options[I] & GSR_GAM)){ if (Options.NeedsSaving) RecordSettings(); initGame(I); return true; }
  return false;
}

void WatchyGSR::initGame(uint8_t GameID){
  if (GameID != 255 && (WatchStyles.Options[GameID] & GSR_GAM)){
    Options.GameStatus = false;
    //WatchFaceEnd();
    if (WatchStyles.AddOn[GameID] != nullptr) WatchStyles.AddOn[GameID]->InsertInitWatchStyle(GameID);
    Options.Game = GameID;
    Options.NeedsSaving = true;
  }
}

void WatchyGSR::WatchFaceEnd(){ if (GetAskWiFi()){ SetAskWiFi(false); endWiFi(); } }

// Watch Face designs are here.

void WatchyGSR::initWatchFaceStyle(){
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
   if (DefaultWatchStyles) { if (Options.WatchFaceStyle > BasicWatchStyles && Options.WatchFaceStyle != 255) { if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr) InsertInitWatchStyle(Options.WatchFaceStyle); else WatchStyles.AddOn[Options.WatchFaceStyle]->InsertInitWatchStyle(Options.WatchFaceStyle); return; } }
   else if (WatchStyles.Count > 0 && BasicWatchStyles == -1) { if (WatchStyles.AddOn[Options.WatchFaceStyle] == nullptr) InsertInitWatchStyle(Options.WatchFaceStyle); else WatchStyles.AddOn[Options.WatchFaceStyle]->InsertInitWatchStyle(Options.WatchFaceStyle); return; }
   InsertInitWatchStyle(Options.WatchFaceStyle);  // Allow for Overrides to do this.
}

void WatchyGSR::drawWatchFaceStyle(){
    uint8_t Style = Options.WatchFaceStyle;
    if (DefaultWatchStyles) { if (Style > BasicWatchStyles && Style != 255) { if (WatchStyles.AddOn[Style] == nullptr) InsertDrawWatchStyle(Style); else WatchStyles.AddOn[Style]->InsertDrawWatchStyle(Style); return; } }
    else if (WatchStyles.Count > 0 && BasicWatchStyles == -1) { if (WatchStyles.AddOn[Style] == nullptr) InsertDrawWatchStyle(Style); else WatchStyles.AddOn[Style]->InsertDrawWatchStyle(Style); return; }
    else Style = 0;
    switch (Style){
        default:
            if (SafeToDraw()){
                drawTime();
                drawDay();
                drawYear();
                drawWeather();
            }
            if (NoMenu()) drawDate();
            break;
    }
}

void WatchyGSR::drawWeather(bool Status){
    uint8_t Style = Options.WatchFaceStyle;
    if (!WeatherData.Ready) return;
    if (DefaultWatchStyles) { if (Style > BasicWatchStyles && Style != 255) { if (WatchStyles.AddOn[Style] == nullptr) InsertDrawWeather(Style,Status); else WatchStyles.AddOn[Style]->InsertDrawWeather(Style,Status); return; } }
    else if (WatchStyles.Count > 0 && BasicWatchStyles == -1) { if (WatchStyles.AddOn[Style] == nullptr) InsertDrawWeather(Style,Status); else WatchStyles.AddOn[Style]->InsertDrawWeather(Style,Status); return; }
    InsertDrawWeather(Style,Status);
}
