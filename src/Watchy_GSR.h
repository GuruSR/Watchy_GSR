#ifndef WATCHY_GSR_H
#define WATCHY_GSR_H

#include <core_version.h>
#include "Defines_GSR.h"
#include "Web-HTML.h"
#include <Arduino_JSON.h>
#include <esp_partition.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <HTTPClient.h>
#include <SmallRTC.h>
#include <SmallNTP.h>
#include <Olson2POSIX.h>
#include <GxEPD2_BW.h>
#include "Locale_GSR.h"
#include <mbedtls/base64.h>
#include <Wire.h>
#include <StableBMA.h>  /* Comment this line out for no BMA support */
#include "Fonts_GSR.h"
#include "Icons_GSR.h"
#include "ArduinoNvs.h"
#include <esp32-hal.h>
#include "soc/soc.h"
#include "soc/rtc.h"
#include "soc/rtc_cntl_reg.h"

class WatchyGSR{
    public:
        static SmallRTC SRTC;
        static SmallNTP SNTP;
        static GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display;
        static constexpr const char* Build = "1.4.7E";
        enum DesOps {dSTATIC, dLEFT, dRIGHT, dCENTER};

    public:
        WatchyGSR();
        virtual void init(String datetime = "") final;
        virtual void StartWeb() final;
        virtual void showWatchFace();
        virtual void drawWatchFace(); //override this method for different watch faces
        virtual void drawTime(uint8_t Flags = 0); // 32 means no AM/PM, 64 means add padding to <10 hour.
        virtual void drawDay();
        virtual void drawDate(bool Short = false);  // Short month can be used.
        virtual void drawYear();
        virtual void handleButtonPress(uint8_t Pressed) final;
        virtual void deepSleep() final;
        static float getBatteryVoltage();
        static float BatteryRead();
        virtual bool IsDark() final;
        virtual bool IsAM() final;
        virtual bool IsPM() final;
        virtual String GetLangWebID() final;
        virtual void CheckButtons() final;
        static uint8_t buttonHeld();
        static uint8_t getButtonPins();
        void drawChargeMe(bool Dark = false);
        void drawStatus(bool Dark = false);
        virtual bool IsBatteryHidden() final;
        static void VibeTo(bool Mode);
        virtual String MakeTime(int Hour, int Minutes, bool& Alarm) final; // For Hour | 32 means no AM/PM, | 64 means add padding to <10 hour.
        virtual String MakeHour(uint8_t Hour) final;
        virtual String MakeMinutes(uint8_t Minutes) final;
        virtual void ClockSeconds() final;
        virtual uint16_t ForeColor() final;
        virtual uint16_t BackColor() final;
        static uint16_t Debounce();
        virtual uint8_t CurrentStyleID() final;
        virtual uint8_t CurrentGameID() final;
        virtual void InsertPost();
        virtual bool OverrideBitmap();
        virtual bool OverrideSleepBitmap();
        virtual void InsertDefaults();
        virtual void InsertOnMinute();
        virtual void InsertWiFi();
        virtual void InsertWiFiEnding();
        virtual void InsertAddWatchStyles();
        virtual void InsertDrawWatchStyle(uint8_t StyleID);
        virtual void InsertInitWatchStyle(uint8_t StyleID);
        virtual void SaveProgress();
        virtual void NeedsSaving() final;
        virtual void InsertDrawWeather(uint8_t StyleID, bool Status = false);
        virtual bool InsertNeedAwake(bool GoingAsleep);
        virtual bool InsertHandlePressed(uint8_t SwitchNumber, bool &Haptic, bool &Refresh);
        virtual void OverrideDefaultMenu(bool Override);
        virtual void WantWeather(bool Wanting);
        virtual void NoStatus(bool NoStats);
        virtual void drawWeather(bool Status = false);
        virtual void RegisterWatchFaces();
        virtual void ShowDefaultMenu() final;
        virtual void ShowGame() final;
        virtual void HideGame() final;
        virtual void UpdateScreen() final;
        virtual void GameStatus(bool NeedAttention) final;
        virtual bool GameStatusOn() final;
        virtual uint8_t AddWatchStyle(String StyleName, WatchyGSR *AddOn, bool IsGame = false) final;
        virtual uint8_t AddWatchStyle(String StyleName) final;
        virtual String InsertNTPServer();
        virtual String CountdownTimer() final;
        virtual String CountdownTimerState() final;
        virtual bool CountdownTimerActive() final;
        virtual String ElapsedTimer() final;
        virtual String ElapsedTimerState() final;
        virtual bool ElapsedTimerActive() final;
        virtual String CurrentSteps(bool Yesterday = false) final;
        virtual uint32_t CurrentStepCount() final;
        virtual String YesterdaySteps() final;
        virtual uint32_t YesterdayStepCount() final;
        virtual bool BMAAvailable() final;
        virtual float BMATemperature(bool Metric = true) final;
        virtual String CurrentWatchStyle() final;
        virtual String CurrentGameStyle() final;
        virtual void AllowDefaultWatchStyles(bool Allow = true) final;
        virtual void AskForWiFi() final;
        virtual bool GetAskWiFi() final;
        virtual bool GetWantWeather() final;
        virtual bool GetNoStatus() final;
        virtual bool GetMenuOverride() final;
        virtual wl_status_t currentWiFi() final;
        virtual wl_status_t WiFiStatus() final;
        virtual void endWiFi() final;
        virtual void getAngle(uint16_t Angle, uint8_t Width, uint8_t Height, uint8_t &X, uint8_t &Y) final;
        virtual bool SafeToDraw() final;
        virtual bool NoMenu() final;
        virtual bool InGame() final;
        virtual void StartNTP(bool TimeSync = true, bool TimeZone = false) final;
        virtual void StartWeather() final;
        virtual bool IsWeatherAvailable() final;
        virtual int GetWeatherTemperature() final;
        virtual int GetWeatherTemperatureFeelsLike() final;
        virtual void SetWeatherScale(bool Metric) final;
        virtual bool IsMetric() final;
        virtual int GetWeatherID() final;
        virtual String GetWeatherIcon() final;
        virtual uint8_t GetWeatherHumidity() final;
        virtual uint8_t GetWeatherClouds() final;
        virtual time_t GetWeatherSunRise() final;
        virtual time_t GetWeatherSunSet() final;
        virtual uint16_t GetWeatherPressure() final;
        virtual uint32_t GetWeatherVisibility() final;
        virtual float GetWeatherWindSpeed() final;
        virtual float GetWeatherWindDirection() final;
        virtual float GetWeatherWindGust() final;
        virtual bool GetWebAvailable() final;
        virtual bool GetWebReady() final;
        virtual int GetWebResponse() final;
        virtual String GetWebData() final;
        virtual bool AskForWeb(String URL, uint8_t Timeout = 5) final;
        virtual String CleanString(String Clean) final;
        virtual void initWatchFaceStyle() final;
        virtual bool ChangeWatchFace(bool Up = true) final;
        virtual void drawWatchFaceStyle() final;
        virtual void drawGame() final;
        virtual void initAddOn(WatchyGSR *NewAddon) final;
        virtual void setFontColor(uint16_t Color) final;
        virtual tmElements_t UTCtoLocal(time_t Incoming) final;
   private:
        void setStatus(String Status);
        void drawMenu();
        void setFontFor(String O, const GFXfont *Normal, const GFXfont *Small, const GFXfont *Smaller, byte Gutter = 5);
        void drawData(String dData, byte Left, byte Bottom, WatchyGSR::DesOps Style, byte Gutter, bool isTime = false, bool PM = false);
        void GoDark(bool DeepSleeping = false);
        void ForceInputs();
        void detectBattery();
        static bool inBrownOut();
        static void BrownOutDetect(bool On = false);
        void SetupESPValues();
        static void StartSetup();
        static uint16_t getDispCS();
        void getPins(float Version);
        void ProcessNTP();
        void UpdateUTC();
        void UpdateClock();
        void UpdateTimerDown();
        bool TimerAbuse();
        void SoundBegin();
        bool SoundActive();
        void KeysStart();
        void KeysStop();
        static void SoundAlarms(void * parameter);
        static void KeysCheck(void * parameter);
        void ManageTime();
        void _rtcConfig();
        void _bmaConfig();
        void UpdateBMA();
        static uint16_t _readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
        static uint16_t _writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
        String MakeTOD(uint8_t Hour, bool AddZeros);
        String MakeSeconds(uint8_t Seconds);
        String MakeSteps(uint32_t uSteps);
        void CheckAlarm(int I);
        void CalculateTones();
        void StartCD();
        void StopCD();
        static uint8_t getToneTimes(uint8_t ToneIndex);
        String getReduce(uint8_t Amount);
        void monitorSteps();
        uint8_t getButtonMaskToID(uint64_t HW);
        static uint8_t getSWValue(bool SW1, bool SW2, bool SW3, bool SW4);
        void SetAskWiFi(bool SetWiFi);
        bool HasIPAddress();
        bool WiFiInProgress();
        void processWiFiRequest();
        String WiFiIndicator(uint8_t Index);
        void UpdateWiFiPower(String SSID, String PSK);
        void UpdateWiFiPower(String SSID);
        void UpdateWiFiPower(uint8_t PWRIndex = 0);
        static void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
        String buildWiFiAPPage();
        void parseWiFiPageArg(String ARG, String DATA);
        void setupDefaults();
        String APIDtoString(uint8_t Index);
        String PASStoString(uint8_t Index);
        void initZeros();
        String GetSettings();
        void StoreSettings(String FromUser);
        uint16_t USCtoWord(unsigned char High, unsigned char Low);
        void WordtoUSC(uint16_t Word, unsigned char &High, unsigned char &Low);
        uint32_t USCtoDWord(unsigned char HHigh, unsigned char HLow, unsigned char LHigh, unsigned char LLow);
        void DWordtoUSC(uint32_t DWord, unsigned char &HHigh, unsigned char &HLow, unsigned char &LHigh, unsigned char &LLow);
        void RetrieveSettings();
        void RecordSettings();
        void ResetOTA();
        void EndOTA(uint8_t Seconds);
        bool IsEndOTA();
        void ResetEndOTA();
        bool OkNVS(String FaceName);
        void SetNVS(String FaceName, bool Enabled = true);
        void NVSEmpty();
        void SetTurbo();
        bool InTurbo();
        bool BedTime();
        bool UpRight();
        bool DarkWait();
        static bool Showing();
        void RefreshCPU();
        void RefreshCPU(int Value);
        void Reboot();
        bool OTA();
        float rawBatteryVoltage();
        uint8_t getTXOffset(wifi_power_t Current);
        void DisplayInit(bool ForceDark = false);
        void DisplayWake(bool Tapped = false);
        void DisplaySleep();
        void ProcessWeather();
        static void GSRWebGet(void * parameter);
        void WatchFaceStart(uint8_t NewFace, bool NoEndWiFi = false);
        bool ChangeGame(bool Up = true);
        void initGame(uint8_t GameID);
        void WatchFaceEnd();
};

struct MenuPOS {
    byte Gutter; // 3
    byte Top;    // MenuTop 72
    byte Header; // HeaderY 97
    byte Data;   // DataY 138
    const GFXfont *Font; // Menu Font.
    const GFXfont *FontSmall; // Menu Font.
    const GFXfont *FontSmaller; // Menu Font.
};
struct FacePOS {
    const unsigned char *Bitmap;  // Null
    const unsigned char *SleepBitmap;  // Null
    byte Gutter; // 4
    byte Time;   // TimeY 56
    byte TimeHeight; // 45
    uint16_t TimeColor;  // Font Color.
    const GFXfont *TimeFont; // Font.
    WatchyGSR::DesOps TimeStyle; // dCENTER
    byte TimeLeft;  // Only for dSTATIC
    byte Day;    // DayY 101
    byte DayGutter; // 4
    uint16_t DayColor;  // Font Color.
    const GFXfont *DayFont; // Font.
    const GFXfont *DayFontSmall; // Font.
    const GFXfont *DayFontSmaller; // Font.
    WatchyGSR::DesOps DayStyle; // dCENTER
    byte DayLeft;  // Only for dSTATIC
    byte Date;   // DateY 143
    byte DateGutter; // 4
    uint16_t DateColor;  // Font Color.
    const GFXfont *DateFont; // Font.
    const GFXfont *DateFontSmall; // Font.
    const GFXfont *DateFontSmaller; // Font.
    WatchyGSR::DesOps DateStyle; // dCENTER
    byte DateLeft;  // Only for dSTATIC
    byte Year;   // YearY 186
    uint16_t YearColor;  // Font Color.
    const GFXfont *YearFont; // Font.
    WatchyGSR::DesOps YearStyle; // dCENTER
    byte YearLeft;  // Only for dSTATIC
};
struct StatusPOS {
    byte WIFIx;  // NTPX 5
    byte WIFIy;  // NTPY 193
    byte BATTx;  // 155
    byte BATTy;  // 178
    bool Inverted;  // Opposite Colors.
    bool BatteryInverted; // Just for the battery indicator.
};
struct Designing final {
    struct MenuPOS Menu;
    struct FacePOS Face;
    struct StatusPOS Status;
};

struct TimeData final {
    time_t UTC_RAW;           // Copy of the UTC on init.
    tmElements_t UTC;         // Copy of UTC only split up for usage.
    tmElements_t Local;       // Copy of the Local time on init.
    String TimeZone;          // The location timezone, not the actual POSIX.
    bool NewMinute;           // Set to True when New Minute happens.
    bool ESPRTC;              // Tells the system you enabled the ESP32 internal RTC instead of an external one.
    uint8_t NextAlarm;        // Next index that will need to wake the Watchy from sleep to fire.
    bool BedTime;             // If the hour is within the Bed Time settings.
};

struct GSRCPUInfo final {
    esp_chip_model_t Model;
    long Base;
    long BrownOutDetection;
    bool HasWiFi;
    bool HasBT;
    bool HasBLE;
};

extern Designing Design;
extern TimeData WatchTime;
#ifdef STABLEBMA_H_INCLUDED
extern StableBMA SBMA;
#endif
extern LocaleGSR LGSR;
#endif
