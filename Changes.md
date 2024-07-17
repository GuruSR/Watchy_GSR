**Version 1.1:**
- **FIX:**  Repaired Watchy Sync menu to properly work with TimeZone requests.
- **FIX:**  Repaired incorrect Menu Item (Steps) showing up after Watchy Sync was used.
- **ADD:**  Updated icons for WiFi, TimeZone & NTP.  WiFi icon now shows -AP for Access Point mode, -X for last good WiFi (connected) and (AP) -A through -J from Additional WiFi Access Points page.
- **ADD:**  Tone reductions to Alarms and Countdown Timer.
- **ADD:**  Tone Reduction in Alarms Sub-Menu to set all 4 alarms to this reduction.
- **ADD:**  Manual WiFi default settings in Defines_GSR.h (WiFi_DEF_SSID) for people who want to manually add their always default WiFi at compile time.
- **FIX:**  Move NTP test while charging is seen to detect Battery which only happens once a minute (avoids constant NTP checks blocking WiFi usage when no WiFi is available).

**Version 1.2:**
- **FIX:**  Fixed Additional WiFi Access Point functionality.
- **FIX:**  Fixed minor issues with various menus not giving proper results.
- **FIX:**  Fixed WiFi indicator to show properly.
- **FIX:**  Fixed POSIX Timezone to be null when not set with a proper one, will auto-force Timezone detection on reboot only if none is stored in Non-Volatile Storage.
- **ADD:**  Non-Volatile Storage for settings and Timezone.
- **ADD:**  Troubleshooting Menu with Screen Reset, Watchy Reboot and Detect Travel (RTC).
- **ADD:**  Feature Add:  Screen Off with "Disabled", "Always" and "Bed Time".

**Version 1.2.1:**
- **FIX:**  Made buttons work always with screen blanking, once to bring screen on, second one does action (if one).
- **FIX:**  Fixed Restore Settings with empty data causing Watchy to crash.
- **FIX:**  Detect Drift not saving changed value to NVS.
- **FIX:**  Detect Drift now correctly checks 1 minute.  (No more oddities with drift calculations.)

**Version 1.3:**
- **ADD:**  Excessive failover for Detect Travel +/- 30 seconds will cause Watchy to ignore RTC and use internal CPU timing [see Usage]
- **ADD:**  CPU control to improve lifespan of battery operation.  8+ hours of battery while in non-RTC mode, using tailored option changes.
- **FIX:**  Fix WiFi timeouts not properly setup if inside Active Mode for too long.
- **FIX:**  Status printing not showing up on dark backgrounds.
- **FIX:**  Fixed Dark so the face drawing knows it happened and vice-versa.

**Version 1.3.1:**
- **ADD:**  Added Performance menu with "Turbo", "Normal" and "Battery Saving" options.
- **FIX:**  Fixed CPU control to be less intensive with respect to reading the CPU too much, now happens at top of init once, using valid set response to record value later.

**Version 1.3.2:**
- **FIX:**  Fixed Turbo Mode and Sleep Modes to not work on each minute regardless of you pushing a button or not.

**Version 1.3.3:**
- **FIX:**  Fixed the restoration of Screen Blanking from storage and restoration from OTA Website.

**Version 1.3.4:**
- **ADD:**  Unification of 2 Watchy versions (DS3231 & PCF8563), also included (and noted for submission) WatchyBattery.h to read the proper ADC PIN depending on RTC type.

**Version 1.3.5:**
- **FIX:**  Fixed the "Bed Time" orientation to only work during the Bed Time range.
- **ADD:**  Double Tap modes "On" and "Only".
- **ADD:**  3 new repositories added with branched out functions:  SmallRTC (replacement of WatchyRTC), SmallNTP (industry standard NTP) and Olson2POSIX Timezone collection and setting.
- **FIX:**  Fixed battery level control of Watchy, extremely low battery will stop updating the screen to prolong battery life, button presses will only work.

**Version 1.3.6:**
- **FIX:**  Corrected Timezone tests (they were backwards).
- **FIX:**  Changed border from "Show/Hide" to "Light/Dark" to match Display Style.
- **FIX:**  Updated BMA control to properly work with Double Tap, method is there for Wrist tilt, no motion setup for it at present.
- **ADD:**  Functionality to block Non-Volatile Storage from being used, will erase and reboot the Watchy and stop using it for this Watchy face.
- **FIX:**  Corrected NTP server to pool.ntp.org, it should work globally, though it does have a restriction on use, so expect it to fail if used too much.

**Version 1.3.7:**
- **ADD:**  Unified format of main Watchy face, so override can be done to include new fonts and relocation.
- **ADD:**  Functions to allow insertion of override code into certain parts of Watchy_GSR without having to replicate whole sections.
- **FIX:**  Changed icons.h to Icons_GSR.h to remove Watchy.h conflict and also removed steps image.

**Version 1.3.8:**
- **ADD:**  Extra Design functionality for Unified format of main Watchy face which includes orientation of elements.
- **FIX:**  InsertWiFi() now only runs if no other process is using the WiFi to avoid collsions.

**Version 1.3.9:**
- **FIX:**  Repaired Screen Blanking issues with alarms during Bed Time (utilizing atMinuteWake from SmallRTC 1.5).
- **FIX:**  Recognize Countdown Timer is in use during Screen Blanking and not use atMinuteWake, but nextMinuteWake instead.
- **ADD:**  All Screen Blanking now use atMinuteWake when Countdown Timer is not in use, this works by waking at minutes 0 and 30 or the next Alarm minute.
- **FIX:**  makeTime and breakTime introduce a month and week day starting with 1, which is incorrect, versions from SmallRTC & SmallNTP now fix this issue, so January 29 will not be seen as February 29 by the RTCs.

**Version 1.4:**
- **FIX:**  Corrected the orientation of the display with the accelerometer.
- **ADD:**  Added Wrist Tilt support for all Screen Blanking with "Screen on Hold" (actively watches your wrist to turn screen off again, heavy battery usage).

**Version 1.4.1:**
- **FIX:**  Corrected missing internal wake functions.
- **ADD:**  Watch Style menu with 1 extra style called "Ballsy".
- **FIX:**  Re-arranged Design Menu setup and included Font.

**Version 1.4.2:**
- **FIX:**  Repaired the Tilt detection so non-Screen Blanking won't keep the Watchy in Active Mode draining the battery while upright.
- **ADD:**  Extra functions (see Override Information) to offer adding or replacing existing Watch Styles without altering the base code.

**Version 1.4.3:**
- **FIX:**  Fixed tilt during non-RTC mode, to work the same as normal.
- **ADD:**  Added InsertNTPServer() function to insert your favorite NTP server instead of pool.ntp.org.

**Version 1.4.3C:**
- **FIX:**  Fixed InsertOnMinute to work outside of WakeUp.
- **FIX:**  Fixed Menu usage during "Always" Screen Blanking, to work on a nextMinute as apposed to staying in Active Mode (battery saving).
- **ADD:**  Added extra font sizes for various segments and added a Gutter value for the watch face (for Overriding) in anticipation of Language switching.
- **FIX:**  Moved Button PINs to Defines_GSR to a different value from Watchy base's defines to avoid conflicts and to correct them based on RTC and not static.

**Version 1.4.3D:**
- **FIX:**  Fixed the PCF8563 variants so that the UP button will work with both versions, this now requires SmallRTC Version 1.8.

**Version 1.4.3E:**
- **ADD:**  Added support in the Override and Design setup to allow for background bitmaps for Watchy on and sleep.
- **DEL:**  Removed InsertBitmap(), replaced with bool OverrideBitmap(), see Override Information for more details.
- **ADD:**  bool OverrideBitmap() and bool OverrideSleepBitmap(), see Override Information for more details.

**Version 1.4.3F:**
- **DEL:**  Removed outdated web portal.
- **ADD:**  Added OTA Website to Watchy Connect.
- **FIX:**  Reworked wake-up code to better behave with and without Sleep modes.

**Version 1.4.3G:**
- **FIX:**  Reworked Deep Sleep to clean up some code that may or may not have worked accurately at times.

**Version 1.4.3H:**
- **FIX:**  Added a #define needed for Dark Border, so compiling without the #define included from Compilation Instructions will disable Dark Border completely.

**Version 1.4.3I:**
- **FIX:**  Fixed structures to be global for the ones necessary for overriding directly from the INO file.
- **ADD:**  Updated GSR.ino to reflect overriding with examples inside that *do* work.
- **FIX:**  Turned VibeTo to false on Deep Sleep to avoid confusion as the motor will turn off then anyways.
- **ADD:**  getAngle changed from "gutter" to "width" and "height" mode, so now analog watch faces can be of any size and not necesarily the whole screen.

**Version 1.4.3J:**
- **FIX:**  Moved the pinMode code until after the setting of the nextMinuteWake to repair the issue with the PCF8563 not waking, thanks to ZeroKelvinKeyboard for the find.

**Version 1.4.4:**
- **FIX:**  Moved all the fonts into 1 font file to reduce clutter.
- **ADD:**  Locale_GSR now contains the English entries for all of the Watchy face text and Webpage text.  (Documentation on adding other languages to follow.)
- **ADD:**  Overriding the Default Menu, so you can USE the Menu button for other things.  10 second hold down of MENU button will force the entry of the default menus.
- **ADD:**  Added a third watch face LCD.
- **FIX:**  Re-added missing Requests test for Active Mode so AskForWiFi() would work properly, missed replacing that after a rollback.

**Version 1.4.5:**
- **FIX:**  Fixed Countdown & Elapsed Timers to be second accurate.
- **ADD:**  Countdown & Elapsed Timers will keep Watchy in "Active Mode" (always on) while viewing their current values when they are On.
- **ADD:**  **BETA** Added 4 extra Button values when combining MENU and BACK with UP and DOWN (see Override Information for more).

**Version 1.4.6:**
- **FIX:**  Fixed font scaling in design of Watchface.
- **FIX:**  Moved Haptic Feedback and Alarm Tones to new task to avoid conflicts and oddities.
- **FIX:**  Removed BMA disable/enable around Vibration Motor usage, it caused no significant changes in operation.
- **ADD:**  Added "Countdown Settings" where there is a Once/Repeat and the Tone length moved from "Countdown Timer", see **Usage** for more information.
- **ADD:**  Added bool InsertNeedAwake(bool GoingAsleep) in 3 places on the single loop for usage to run code along with stopping Watchy from Deep Sleep.

**Version 1.4.6A:**
- **FIX:**  Fixed UTC update not confusing minute changes when Countdown or Elapsed Timer is/are in use.
- **ADD:**  Added a function for override use called ClockSeconds(), updates the WatchTime.UTC and WatchTime.Local with current time.

**Version 1.4.7:**
- **FIX:**  Redesign main loop to be single passthrough instead of multiple, reduced code and made stable.
- **ADD:**  Added Inverts for Design to allow Status and Battery to be inverted from current ForeColor.
- **ADD:**  Added (and fixed the default Design) GSR_AutoFore which replaces the GxEPD_BLACK on Design elements, allowing Design elements to follow current ForeColor automatically.
- **ADD:**  OpenWeatherMap functionality, OTA Website will offer a spot to enter your API Key.  The API Key is required for StartWeather() to work.
- **ADD:**  InsertDrawWeather(bool Small) is called when drawWeather() is called during WatchFace display.
- **FIX:**  Menu Overrides only works for the WatchFaces that ask for it, others do not.
- **FIX:**  WiFi Indicator will stay active until all requests are finished.
- **FIX:**  Fix Display Update during heavy timer and Edit RTC usage (1 second enforcement per update).  Moving through elements will be 1 second apart.
- **FIX:**  Display Sleeping incorrectly ignoring MENU state and not updating time onscreen.
- **FIX:**  SmallRTC2.2 replaces "next minute" from DS3231M with atMinuteWake with specific minute to avoid "it doesn't feel like minute counting".
- **FIX:**  Internet connectivity speed increased greatly, IP address should happen within a second of connection.
- **FIX:**  TimeZone, NTP and Weather should all collect data within a few seconds instead of close to 15 seconds.
- **FIX:**  Screen Blanking not honoring current state with new smaller loop, now properly behaves.  Button presses while screen is blanked, just turns it on.
- **FIX:**  Reduced code segments and unnecessary tests in a variety of places (more will come later).
- **ADD:**  Built in Geo-Locate code, will now be used in conjunction with OWM to get Geo-Location weather data (OWM 2.5+).
- **ADD:**  Threaded AskForWeb function with companion functions to ask a website for returned data.  Used with StartWeather() for both Geo-Locate & OWM.
- **ADD:**  Functions to retrieve Weather data and to tell if it has been ever received.
- **ADD:**  Functions to allow WatchFace AddOns to be included easily with a simple \#include in GSR.ino allowing collections up to 96 WatchFaces.
- **FIX:**  Fixed system debounce for buttons to avoid repeated presses.
- **FIX:**  BUTTON MIXING is no longer BETA.
- **FIX:**  Bitmap and SleepBitmap to no longer follow the ForeGround color, will always use GxEPD_WHITE (Fore) and GxEPD_BLACK (Back).
- **ADD:**  Menuing system now includes a cursor to show position and offers toggles for certain options (White is on, Black is off in some cases).
- **ADD:**  NTP Auto Sync, allows the Watchy_GSR to wake the Watchy up for an NTP Sync.  NTP Syncs do *NOT* work when Drift Calculation is in progress.
- **FIX:**  Threaded Alarm & Haptic Feedback system fixed to reduce interference with Watchy operation.  Reboots cause Haptic feedback.
- **ADD:**  Step Counter now is stored each minute to NVS, so upon a reboot, progress shouldn't be lost.
- **ADD:**  NTP and Weather can now wake the Watchy on interval while Screen Blanking is active, to run their respective requests.
- **FIX:**  Included current PinMode setup before DeepSleep.
- **FIX:**  Cleaned up DeepSleep function, so that less work is done.
- **FIX:**  InsertNeedAwake is only in 1 place in the main loop to reduce code execution per loop.
- **FIX:**  WiFi requests are terminated if WatchFace is changed during use to avoid cross-contamination between WatchFaces.
- **FIX:**  Default Design style for Watchy_GSR is now used in initWatchFaceStyle to avoid accidental WatchFaces having no Design elements and winding up with a blank screen with no fonts.
- **FIX:**  Removed attachInterrupts, they were responsible for causing the brownouts by causing large amounts of interrupts, even when using RISING as a means to detect the button presses.

**Version 1.4.7A:**
- **FIX:**  Fixed the battery indicator on screen blanking.
- **FIX:**  Fixed the AskForWiFi not allowing multiple requests from one watch face.
- **FIX:**  Removed CleanJSON that was interfering with JSON data requests.
- **ADD:**  Added CleanString for JSON data.
- **ADD:**  IsMetric function to find out of the weather scale is Metric or not.

**Version 1.4.7B:**
- **FIX:**  Fixed the battery indicator on screen blanking FINALLY!
- **FIX:**  Fixed the OTA Website and OTA Update so that they don't try to connect too fast via WiFi (which can cause a crash or brownout if done too fast).
- **ADD:**  drawStatus() now has a Screen Blanking method that also now includes (for both), a Joystick icon showing the current active game "wants your attention".
- **FIX:**  Added a ForceInputs() function that does the pinModes for the pins that need it (and not a loop for pins that don't).
- **ADD:**  The missing weather information (Clouds, Sunrise, Sunset, Pressure and Visibility) converted (if possible) to the unit selection currently in use.
- **ADD:**  A function for changing the current Watchface forward or backward through the list, will return true if it actually switched.
- **ADD:**  Migrated all the settings for Watchface Styles into 1 byte instead of 4 bools to save RTC Memory space, which includes new functions to set and retrieve those states.
- **FIX:**  AskForWiFi() won't attempt to start the WiFi interface while the notification motor is active to avoid brownouts.

**Version 1.4.7C:**
- **FIX:**  Battery Detection for charging has changed, now using a float level system, should stop fantom charging indications.
- **ADD:**  inBrownOut code to avoid overtaxing the battery during WiFi and other uses to reduce draw.  FreeRTOS BrownOut Detection is disabled.
- **FIX:**  Leftover variable IDidIt from old loop replaced in 1.4.7 was blocking `InsertOnMinute` has been removed as it wasn't supposed to be there.
- **FIX:**  Alarms not firing when Drift correction would reduce the time by 1 second at RTC read when an alarm woke the Watchy up, now has a WfNM (Wait for New Minute) flag for the main loop that only goes `false` when a new minute happens.
- **ADD:**  Added configuration changes for keeping track of 2 Drift Values.
- **ADD:**  Drift Value editing, so it can be altered after Drift Calculation has finished.  This helps with accuracy, see SmallRTC on Drift Calculation for more information on how to edit this value.
- **FIX:**  Fixed Edit RTC's BACK button usage, was missing screen update requests which caused random jumps of 2 to 3 digits from previous spot.
- **ADD:**  WiFi indicator will now have a ! at the end to denote it is "attempting to connect" to that AP, when the ! disappears, it is connected.
- **ADD:**  BrownOut limiter to slow WiFi and other functions down in the event the battery level drops too much during operation, some functions may slow down considerably as battery level drops.
- **ADD:**  Added threaded button detection, which helps improve button response.
- **FIX:**  Fixed Alarms to be more global in response.
- **FIX:**  Fixed Battery state causng non-Screen Blanking modes to keep Watchy awake for minutes.
- **FIX:**  Sorted the lower menu items so that the WiFi functions are all together, made no sense offering WiFi settings when WiFi isn't available.
- **ADD:**  With the inclusion of CPU ID detection, features unavailable will be absent if they do not exist on the current CPU.

**Version 1.4.7C:**
- **FIX:**  Fixed Watchy Connect to have no password to avoid TKIP connection issues (use with caution, recommend lowering WiFi transmission power to 7db and move it close to your device).
- **FIX:**  Fixed Watchy Connect Menu option, pressing BACK now will tell you to WAIT.  The OTA Website will also do the same thing.  3 seconds later, you will be properly out.
- **FIX:**  Battery Detection was incorrectly saving state during WiFi usage and would either say it was charging incorrectly, but will still show LOW battery.
- **ADD:**  AddOn functions BMAAvailable() and BMATemperature(bool Metric) to allow AddOns access to tell if the BMA was compiled in or not and access to the temperature sensor.
- **FIX:**  Override Information was lacking changes and now reflects the correct function list.

**Version 1.4.7D:**
- **FIX:**  Re-design of battery charge detection (hopefully this one works better).
- **FIX:**  Compatibility with SmallRTC 2.3.4+.
- **ADD:**  Day of week choice for NTP sync.  See Usage.md for how to use it.

**Version 1.4.7E:**
- **FIX:**  Fixed V2 UP button declaration for wakeup.
- **FIX:**  Fixed OTA Website Weather section erroring on invalid length.
- **ADD:**  New system for versioning of hardware.  No longer using pre-created macros as they're no longer up to date.

**Version 1.4.7F:**
- **FIX:**  Fixed WiFi code that was causing a re-attempt every split second to connect when connection was already done.
- **FIX:**  Fixed shortcomings of Watchy V3 and changed ESP32C3 to ESP32S3 (noticed that was the wrong test).
- **REM:**  Removed openweathermap.org due to 2.5 being discontinued and One Call 3.0 is the only available weather option (which costs money).
- **ADD:**  Added in open-meteo.com weather in place of openweathermap.org, all calls minus 1 work as expected.  (Sunrise & Sunset may not be as expected.)
- **REM:**  Removed GetWeatherIcon function as it was specific to openweathermap.org.
- **FIX:**  Reduced HTTP timeout to 2 seconds, lest the task scheduler has a fit and reboots the Watchy.
- **FIX:**  Fixed the timing on the vib motor and WebGet tasks to promote better user responsiveness.
- **FIX:**  Battery Charge init was incorrectly setup, thus caused the charge indicator to instantly light when it shouldn't.
- **ADD:**  Added Latitude and Longitude static values which can be set in the Weather Settings section of OTA Website.
- **FIX:**  Hopefully fixed the auto-detect on the V3 display pin change.  (Faked V3 on V2 and the V2 screen did nothing, which is promising.)

**Version 1.4.7G:**
- **ADD:**  Added the GxEPD driver with direct changes to the code base (sadly, as the driver should allow late binding).
- **FIX:**  Moved the chip detection to the GxEPD driver code to hopefully properly setup for V3.

**Version 1.4.7H:**
- **REM:**  Removed the GxEPD driver code duplication, users wanting Dark border need to do the patch again.
- **FIX:**  Fixed the display code for V1 to V2, now all versions work properly.
- **REM:**  Removed the detection of the USB as it was causing constant boots flip-flopping the detection bit.
- **REM:**  Temporarily removed the BMA Tilt and Double Tap, since it doesn't appear to work the same with this BMA (may need more info).
- **FIX:**  Fixed NTP being run multiple times in a row.
- **ADD:**  V3 supported.

**Version 1.4.7I:**
- **FIX:**  Fixed GSRWebGet, it now properly obtains the results and can now have longer timeouts (20 second max programmed in).
- **FIX:**  Altered the divider for V3, hopefully it'll represent the battery level better.
- **PAR:**  Partial fix with the charging indicator on V3, to see if it will behave better, may end up going to the V1-V2 solution.
