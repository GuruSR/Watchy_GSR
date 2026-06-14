#pragma once
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
#ifndef WATCHY_GSR_H
// 1550064 before export of Alarms.
// 1552276 after export of Alarms.
// 1504164 after removing Watchy base code requirements.
  #define WATCHY_GSR_H
  #include <string>
  #include "Defines_GSR.h"
  #include "Web-HTML.h"
  #include <ArduinoJson.h>
  #include <esp_partition.h>
  #include <ESPmDNS.h>
  #include <mbedtls/base64.h>
  #include "esp_chip_info.h"
  #include <driver/rtc_io.h>
  #include <Wire.h>
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <ArduinoOTA.h>
//  #include "Display_GSR.h"
  #include <WebServer.h>
  #include <esp_wifi.h>
  #include <HTTPClient.h>
  #include <GxEPD2_BW.h>
  //#include <Preferences.h>
  #include "ArduinoNvs.h"
  #include "Globals_GSR.h"
  #include <SmallRTC.h>
  #include <SmallNTP.h>
  #include "Alarms_GSR.h"
  #include <Olson2POSIX.h>
  #include "Locale_GSR.h"
  #include <StableBMA.h>  /* Comment this line out for no BMA support */
  #include "Fonts_GSR.h"
  #include "Icons_GSR.h"
  #ifdef ARDUINO_ESP32S3_DEV
    #define SMALL_RTC_NO_DS3232
    #define SMALL_RTC_NO_PCF8563
  #endif

static const char UserAgent[] = "Watchy";
//WiFi statics.
static const wifi_power_t RawWiFiTX[11]= {WIFI_POWER_19_5dBm,WIFI_POWER_19dBm,WIFI_POWER_18_5dBm,WIFI_POWER_17dBm,WIFI_POWER_15dBm,WIFI_POWER_13dBm,WIFI_POWER_11dBm,WIFI_POWER_8_5dBm,WIFI_POWER_7dBm,WIFI_POWER_5dBm,WIFI_POWER_2dBm};
static const char *IDWiFiTX[11] = {"19.5dBm [Max]","19dBm","18.5dBm","17dBm","15dBm [Laptop]","13dBm","11dBm","8.5dBm [Medium]","7dBm","5dBm [10 meters]","2dBm [Low]"};

static const char WiFiTXT[] = "WiFi-";

const uint16_t Bits[11] = {1,2,4,8,16,32,64,128,256,512,1024};
static uint16_t wIDs[28] = {0,1,2,3,45,48,51,53,55,56,57,61,63,65,66,67,71,73,75,77,80,81,82,85,86,95,96,99}; // Weather IDs accepted.

class WatchyGSR{
    public:
        //static dCh display;
        static GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display;
        static SPIClass hspi;
        static constexpr const char* Build = "1.4.8";
        enum DesOps {dSTATIC, dLEFT, dRIGHT, dCENTER};

    public:
        WatchyGSR();

      // Boot Override
        virtual void InsertDefaults();
        virtual void AllowDefaultWatchStyles(bool Allow = true) final;
        virtual void InsertPost();
        virtual bool OverrideBitmap();
        virtual bool OverrideSleepBitmap();
      // Boot Face, Game, App
        virtual uint8_t AddWatchStyle(String StyleName, WatchyGSR *AddOn, bool IsGame = false, bool isAPP = false) final;
        virtual void InsertAddWatchStyles();
        virtual void OverrideDefaultMenu(bool Override);
        virtual void WantWeather(bool Wanting);
        virtual void NoStatus(bool NoStats);
        virtual void NoAlarmStatus(bool NoAlarmStats);
        virtual void UseMyWiFiIcon(const unsigned char *cWiFiOFF, const unsigned char *cWiFiON, uint8_t width, uint8_t height);
      // Face, Game, App
        virtual uint8_t CurrentStyleID() final;
        virtual uint8_t CurrentGameID() final;
        virtual void drawTime(uint8_t Flags = 0); // 32 means no AM/PM, 64 means add padding to <10 hour.
        virtual void drawDay();
        virtual void drawDate(bool Short = false);  // Short month can be used.
        virtual void drawYear();
        virtual void drawWeather(bool Status = false);
        void drawChargeMe(bool Dark = false);
        void drawStatus(bool Dark = false);
        virtual bool IsBatteryHidden() final;
        virtual bool IsDark() final;
        virtual bool IsAM() final;
        virtual bool IsPM() final;
        virtual bool Is24HourMode() final;
        virtual bool IsLightMode() final;
        virtual bool isLeapYear(bool useLocal = true);
        virtual String MakeTime(int Hour, int Minutes, bool& Alarm) final; // For Hour | 32 means no AM/PM, | 64 means add padding to <10 hour.
        virtual String MakeHour(uint8_t Hour) final;
        virtual String MakeMinutes(uint8_t Minutes) final;
        virtual uint16_t ForeColor() final;
        virtual uint16_t BackColor() final;
        virtual void InsertOnMinute();
        virtual void InsertInitWatchStyle(uint8_t StyleID);
        virtual void InsertDrawWatchStyle(uint8_t StyleID);
        virtual void InsertDrawOverlay(uint8_t StyleID);
        virtual void InsertDrawWeather(uint8_t StyleID, bool Status = false);
        virtual bool InsertNeedAwake(bool GoingAsleep);
        virtual void GameStatus(bool NeedAttention) final;
        virtual bool GameStatusOn() final;
        virtual void ShowDefaultMenu() final;
        virtual String CurrentSteps(bool Yesterday = false) final;
        virtual uint32_t CurrentStepCount() final;
        virtual String YesterdaySteps() final;
        virtual uint32_t YesterdayStepCount() final;
        virtual bool BMAAvailable() final;
        virtual float BMATemperature(bool Metric = true) final;
        virtual String CurrentWatchStyle() final;
        virtual String CurrentGameStyle() final;
        virtual String CountdownTimer() final;
        virtual String CountdownTimerState() final;
        static bool CountdownTimerActive();
        virtual String ElapsedTimer() final;
        virtual String ElapsedTimerState() final;
        virtual bool ElapsedTimerActive() final;
        virtual bool GetWantWeather() final;
        virtual bool GetNoStatus() final;
        virtual bool GetNoAlarmStatus() final;
        virtual bool GetMenuOverride() final;
        virtual void ShowGame() final;
        virtual void HideGame() final;
        virtual bool SafeToDraw() final;
        virtual bool NoMenu() final;
        virtual bool InGame() final;
        virtual void setFontColor(uint16_t Color) final;
      // Interaction
        virtual bool InsertHandlePressed(uint8_t SwitchNumber, bool &Haptic, bool &Refresh);
        static uint8_t getButtonPins();
      // Power
        virtual bool getBatteryCharging() final;
        static float getBatteryVoltage();
        virtual float getBatteryVoltagePercent() final;
        virtual float getLowBattery(bool useCritical = false) final;
        virtual float getLowBatteryRadio() final;
      // Radio
        virtual void AskForWiFi() final;
        virtual bool GetAskWiFi() final;
        virtual void InsertWiFi();
        virtual void InsertWiFiEnding();
        virtual void endWiFi() final;
      // Clock
        virtual time_t getISO8601(String inTime) final;
        virtual tmElements_t UTCtoLocal(time_t Incoming) final;
        virtual void requestClockUpdate();
        static void requestUTCUpdate();
      // Utility
        static void requestRefreshScreen();
        virtual void UpdateScreen() final;
        static void requestDisplayWake();
        static bool requestShowing();
        static bool requestOTAStatus();
        virtual void NeedsSaving() final;
        virtual String InsertNTPServer();
        virtual void getAngle(uint16_t Angle, uint8_t Width, uint8_t Height, uint8_t &X, uint8_t &Y) final;
        virtual bool filterGeo(double geoValue, bool isLatitude = false) final;
        virtual bool ChangeWatchFace(bool Up = true) final;
        virtual String getCurrentNTPServer() final;
        virtual String CleanString(String Clean) final;
      // Weather
        virtual bool IsWeatherAvailable() final;
        virtual int GetWeatherTemperature() final;
        virtual int GetWeatherTemperatureFeelsLike() final;
        virtual void SetWeatherScale(bool Metric) final;
        virtual bool IsMetric() final;
        virtual int GetWeatherID() final;
        virtual uint8_t GetWeatherHumidity() final;
        virtual uint8_t GetWeatherClouds() final;
        virtual time_t GetWeatherSunRise() final;
        virtual time_t GetWeatherSunSet() final;
        virtual uint16_t GetWeatherPressure() final;
        virtual uint32_t GetWeatherVisibility() final;
        virtual float GetWeatherWindSpeed() final;
        virtual float GetWeatherWindDirection() final;
        virtual float GetWeatherWindGust() final;
        virtual double getWeatherLatitude(bool useStatic = false) final;
        virtual double getWeatherLongitude(bool useStatic = false) final;
      // Web
        virtual bool GetWebAvailable() final;
        virtual bool GetWebReady() final;
        virtual int GetWebResponse() final;
        virtual String GetWebData() final;
        virtual bool AskForWeb(String URL, uint8_t Timeout = 5) final;
        

      // Override Only
        virtual void init(String datetime = "") final;
        virtual void StartWeb() final;
        virtual void handleButtonPress(uint8_t Pressed) final;
        virtual void deepSleep() final;
        virtual String GetLangWebID() final;
        virtual uint8_t getLangID() final;
        virtual void CheckButtons() final;   // UNUSED
        static void alarmResponse();
        static void setDarknessLast(uint32_t newDark);
        static uint8_t requestMenuItem();
        virtual void showWatchFace();
        virtual void drawWatchFace(); //override this method for different watch faces
        virtual void drawGame() final;
        virtual void drawOverlay(uint8_t StyleID) final;
        static uint16_t Debounce();
        static void setDebounce(bool pulse = false);
        virtual void SaveProgress();
        virtual void RegisterWatchFaces();
        virtual uint8_t AddWatchStyle(String StyleName) final;
        virtual wl_status_t currentWiFi() final;
        virtual wl_status_t WiFiStatus() final;
        virtual void StartNTP(bool TimeSync = true, bool TimeZone = false) final;
        virtual void StartWeather() final;
        virtual void initWatchFaceStyle() final;
        virtual void drawWatchFaceStyle() final;
        virtual void initAddOn(WatchyGSR *NewAddon) final;
   private:
        void setStatus(String Status);
        void drawMenu();
        void setFontFor(String O, const GFXfont *Normal, const GFXfont *Small, const GFXfont *Smaller, byte Gutter = 5);
        void drawData(String dData, byte Left, byte Bottom, WatchyGSR::DesOps Style, byte Gutter, bool isTime = false, bool PM = false);
        void GoDark(bool DeepSleeping = false);
        void espPinSetup(gpio_num_t pin, bool pullUp = true, bool bOutput = true);
        void adcPins();
        void ForceInputs();
        void detectBattery(bool booting = false);
        static bool inBrownOut();
        static void BrownOutDetect(bool On = false);
        void SetupESPValues();
        void espPinModes();
        static void startSetup();
        static bool isESP32S3();
        void getPins(float Version);
        static uint16_t getDispCS();
        static uint16_t getDispDC();
        static uint16_t getDispRES();
        static uint16_t getDispBSY();
        void ProcessNTP();
        void UpdateUTC();
        void UpdateClock();
        bool TimerAbuse();
        void KeysStart();
        void KeysStop();
        void drawLogOutput();
        static void KeysCheck(void * parameter);
        time_t tmTOtime_t(tm intm, bool flatten = true);
        void ManageTime();
        void _bmaConfig();
        void UpdateBMA();
        static uint16_t _readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
        static uint16_t _writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
        bool syncToday();
        String MakeTOD(uint8_t Hour, bool AddZeros);
        String MakeSeconds(uint8_t Seconds);
        String MakeSteps(uint32_t uSteps);
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
        String getNTPfromWeb();
        void setNTPfromWeb(String newNTP);
        String overrideNTPfromWeb(String currentNTP);
        void initZeros();
        String GetSettings();
        void StoreSettings(String FromUser);
        uint16_t USCtoWord(unsigned char High, unsigned char Low);
        void WordtoUSC(uint16_t Word, unsigned char &High, unsigned char &Low);
        uint32_t USCtoDWord(unsigned char HHigh, unsigned char HLow, unsigned char LHigh, unsigned char LLow);
        void DWordtoUSC(uint32_t DWord, unsigned char &HHigh, unsigned char &HLow, unsigned char &LHigh, unsigned char &LLow);
        void RetrieveSettings();
        void RecordSettings();
        String getBasicIndex();
        void ResetOTA();
        void EndOTA(uint8_t Seconds);
        bool IsEndOTA();
        void ResetEndOTA();
        bool OkNVS(String FaceName);
        void SetNVS(String FaceName, bool Enabled = true);
        void NVSEmpty();
        void SetTurbo();
        bool InTurbo();
        bool UpRight();
        bool DarkWait();
        static bool Showing();
        void RefreshCPU();
        void RefreshCPU(int Value);
        void Reboot();
        bool OTA();
        static float rawBatteryVoltage(bool useSecond = false);
        static bool readRawBatteryVoltage(float &rawBattery, bool useSecond = false);
        uint8_t getTXOffset(wifi_power_t Current);
        void DisplayInit(bool ForceDark = false);
        void DisplayWake(bool Tapped = false);
        void DisplaySleep();
        float getRealLowBattery(bool useCritical, bool radioLevel = false);
        void ProcessWeather();
        String makeGeo(String inGeo, bool isLat);
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

struct GSRCPUInfo final {
    esp_chip_model_t Model;
    long Base;
    long BrownOutDetection;
    bool HasWiFi;
    bool HasBT;
    bool HasBLE;
};

struct dummyalarms final {
    AlarmsGSR Alarms;
};

extern struct String GSRLogOutput;
extern struct Designing Design;
extern  SmallRTC SRTC;
extern  SmallNTP SNTP;
extern  AlarmsGSR Alarms;
#ifdef STABLEBMA_H_INCLUDED
extern StableBMA SBMA;
#endif
extern LocaleGSR LGSR;
#endif
