#pragma once
#ifndef DEFINES_GSR_H
#define DEFINES_GSR_H

// WiFi
#define WiFi_AP_SSID "Watchy Connect"
#define WiFi_AP_PSWD                                                           \
  "" // Leave out so there is no encryption, be careful setting WiFi passwords
     // in public with this.  This is done as some devices will refuse to
     // connect via TKIP.
#define WiFi_AP_HIDE                                                           \
  false // Hide "WiFi_AP_SSID" so it doesn't show up in the normal scan list. IS
        // NOT A SECURITY METHOD!
#define WiFi_AP_MAXC 4   // Maximum user connections (default is 4).
#define WiFi_SSID_MAX 32 // Do not change these two values.
#define WiFi_PASS_MAX 63
// Use instead of Watchy Connect (if necessary)
#define WiFi_DEF_SSID ""
#define WiFi_DEF_PASS ""

// Battery
#define GSR_MaxBattery 4.37
#define GSR_ChargePerMin 1.10

// functions
//#define roller(v, lo, hi) (((v) < (lo)) ? (hi) : ((v) > (hi)) ? (lo) : (v))
//#define gobig(v, lo) ((v) > (lo) ? (v) : (lo))
//#define golow(v, hi) ((v) < (hi) ? (v) : (hi))
//#define isBitOn(var, pos) ((var) & (1 << (pos)))
//#define setBitOn(var, pos) ((var) || (1 << (pos)))
//#define BitValue(pos) (1 << (pos))

// Watch face states.
#define GSR_WATCHON 0
#define GSR_MENUON 1
#define GSR_GAMEON 2

// Maximum styles possible.
#define GSR_MaxStyles 96

// Nenu size defines.
#define GSR_MenuWidth 200
#define GSR_MenuHeight 83

// Color to tell system to use Auto foreground.
#define GSR_AutoFore 0xC0C0

// Menu offsets so I don't have to statically enter them everywhere.
// Root Menu
#define GSR_MENU_STEPS 0
#define GSR_MENU_ALARMS 1
#define GSR_MENU_TIMERS 2
#define GSR_MENU_GAME 3
#define GSR_MENU_OPTIONS 4
// Alarms Menu
#define GSR_MENU_ALARM1 5
#define GSR_MENU_ALARM2 6
#define GSR_MENU_ALARM3 7
#define GSR_MENU_ALARM4 8
#define GSR_MENU_ALARM5 9
#define GSR_MENU_ALARM6 10
#define GSR_MENU_ALARM7 11
#define GSR_MENU_ALARM8 12
#define GSR_MENU_TONES 13
#define GSR_MENU_TIMEDN 14
#define GSR_MENU_TMEDCF 15
#define GSR_MENU_TIMEUP 16
// Options Menu
#define GSR_MENU_STYL 17
#define GSR_MENU_LANG 18
#define GSR_MENU_DISP 19
#define GSR_MENU_BRDR 20
#define GSR_MENU_SIDE 21
#define GSR_MENU_SWAP 22
#define GSR_MENU_ORNT 23
#define GSR_MENU_MODE 24
#define GSR_MENU_FEED 25
#define GSR_MENU_TRBO 26
#define GSR_MENU_DARK 27
#define GSR_MENU_SAVE 28
#define GSR_MENU_INFO 29
#define GSR_MENU_TRBL 30
#define GSR_MENU_TPWR 31
#define GSR_MENU_SYNC 32
#define GSR_MENU_WEAT 33
#define GSR_MENU_WIFI 34
#define GSR_MENU_OTAU 35
#define GSR_MENU_OTAM 36
// Troubleshooting Menu
#define GSR_MENU_SCRN 37
#define GSR_MENU_STYP 99
#define GSR_MENU_RSET 38
#define GSR_MENU_TOFF 39 // Time Diff offset.
#define GSR_MENU_BERR 40
#define GSR_MENU_UNVS 41

// Menu segments.
#define GSR_MENU_INNORMAL 0
#define GSR_MENU_INALARMS 1
#define GSR_MENU_INTIMERS 2
#define GSR_MENU_INOPTIONS 3
#define GSR_MENU_INTROUBLE 4

// Watchface Options
#define GSR_AFW 1
// Used to cancel any open WiFi when in use by that watch face at time of
// switch.
#define GSR_iWW 2
// Used to track whether or not the weather is needed for this face.
#define GSR_NOS 4
// Used to stop the default status symbols from being printed (WiFi -> Alarms &
// Battery Charge).
#define GSR_NOA 8
// Used to stop the default status symbols from being printed (WiFi -> Alarms &
// Battery Charge).
#define GSR_MOV 16
// True if override, but can be disabled *IF* held down for 10 seconds, it would
// not open the menu.
#define GSR_GAM 32
// Set at startup for telling Watchy GSR that the AddOn is a game and not a
// watch face.
#define GSR_APP 64
//  AddOn as an App will offer menu insert.

// Weather Defines.
#define NOLOC                                                                  \
  255 // Means no Geo Location found (255 is out of the range for Long/Lat
      // values).

// SetCPU defines.
#define GSR_CPUMAX 65280
#define GSR_CPUDEF 65281
#define GSR_CPUMID 65282
#define GSR_CPULOW 65283
#endif
