#ifndef DEFINES_GSR_H
#define DEFINES_GSR_H

//debug
#define USEDEBUG 1  // !0 is on, will not setup Serial OR print output if zero.

//display
#define BLE_DEVICE_NAME "Watchy BLE OTA"
// #define WATCHFACE_NAME "Watchy GSR"
#define SOFTWARE_VERSION_MAJOR 1
#define SOFTWARE_VERSION_MINOR 0
#define SOFTWARE_VERSION_PATCH 0
#define HARDWARE_VERSION_MAJOR 1
#define HARDWARE_VERSION_MINOR 0

// WiFi
#define WiFi_AP_SSID "Watchy Connect"
#define WiFi_AP_PSWD "Watchy123"
#define WiFi_AP_HIDE false

// Battery
#define MinBattery 3.58

// functions
#define clamp(v,lo,hi) (((v)<(lo))?(lo):((v)>(hi))?(hi):(v))
#define roller(v,lo,hi) (((v)<(lo))?(hi):((v)>(hi))?(lo):(v))
#define gobig(v,lo) ((v)>(lo)?(v):(lo))
#define golow(v,hi) ((v)<(hi)?(v):(hi))

// Watch face states.
#define WATCHON 0
#define MENUON 1

// Nenu offset defines.
#define MenuTop 72
#define MenuHeight 83
#define HeaderY 97
#define DataY 138

// Watchface offset defines.
#define TimeY 56
#define DayY 101
#define DateY 143
#define YearY 186

// NTP & TZ Offset defines.
#define NTPY 193
#define NTPX 5

// Menu offsets so I don't have to statically entere them everywhere.
#define MENU_STEPS 0
#define MENU_ALARMS 1
#define MENU_TIMERS 2
#define MENU_OPTIONS 3
#define MENU_ALARM1 4
#define MENU_ALARM4 7
#define MENU_TIMEDN 8
#define MENU_TIMEUP 9
#define MENU_DISP 10
#define MENU_BRDR 11
#define MENU_SIDE 12
#define MENU_SWAP 13
#define MENU_ORNT 14
#define MENU_MODE 15
#define MENU_FEED 16
#define MENU_TRBO 17
#define MENU_SCRN 18
#define MENU_SYNC 19
#define MENU_WIFI 20
#define MENU_OTAU 21
#define MENU_OTAM 22
#define MENU_RSET 23

// Menu segments.
#define MENU_INNORMAL 0
#define MENU_INALARMS 1
#define MENU_INTIMERS 2
#define MENU_INOPTIONS 3

// Button debounce.
#define KEYPAUSE 333

//ALARM flags.
#define ALARM_REPEAT 128
#define ALARM_ACTIVE 256
#define ALARM_NOTRIGGER 511
#define ALARM_TRIGGERED 512
#define ALARM_DAYS 127
#define ALARM_ALL 1023

// Setup lengths.
#define AlarmSetup 1234567890

//pins
#define ADC_PIN 33
#define RTC_PIN GPIO_NUM_27
#define CS 5
#define DC 10
#define RESET 9
//#define BUSY 19
#define VIB_MOTOR_PIN 13
#define MENU_BTN_PIN 26
#define BACK_BTN_PIN 25
#define UP_BTN_PIN 32
#define DOWN_BTN_PIN 4
#define MENU_BTN_MASK GPIO_SEL_26
#define BACK_BTN_MASK GPIO_SEL_25
#define UP_BTN_MASK GPIO_SEL_32
#define DOWN_BTN_MASK GPIO_SEL_4
#define ACC_INT_MASK GPIO_SEL_14
#define BTN_PIN_MASK MENU_BTN_MASK|BACK_BTN_MASK|UP_BTN_MASK|DOWN_BTN_MASK

#endif
