***For those with the new 2024 Watchy (V3), please read this TO THE END!***

![Watchy GSR Face](/Images/Watchy_GSR.gif)

This repo's format:

Bin folder contains the current OTA Bin (V1 to V2 ONLY) that you can use a tool to upload with or an OTA Upgrade capable firmware to install the current OTA Bin (V1 to V2 ONLY) to the Watchy.

src folder contains a universal (Arduino & PlatformIO) setup for compilation, instructions on how to get it to compile are in the src folder.

WatchFace Addons contains an EmptyAddOn.h (so you can use it as a template for making your own), an EmptyGameAddOn.h as well as 2 stock Watchy_GSR WatchFaces (Ballsy & LCD).

Needed libraries:  Arduino Libraries, ArduinoOTA (included in ESP32 2.0.13), SmallRTC (2.3.7 or greater, GuruSR), SmallNTP (GuruSR), StableBMA (GuruSR), Olson2POSIX (GuruSR) AND Watchy (1.4.7 or greater) base.

**NOTES:**
- V1 to V2 Watchy: Watchy should use version 2.0.13 of the ESP32 libraries, 3.0.2 can be used but causes a much larger binary and requires ESP-IDF 5.1 or greater (which is a manual install).
- V3 Watchy:  You need 3.0.2 and ESP-IDF 5.1 or greater (manual install).

**WARNING:**

**2024 V3 Watchy Owners**:  This firmware is *NOT* tested on your device, it *MAY* work, but I cannot test it as I do not have a V3 to test it with.  I have in as much capacity that I am capable of, set this up to work in the environment OF the new V3, but I cannot test it.  If you feel brave enough to do so, until I can actually physically test it on a V3, you can by all means do so.  Please see the Compilation Instructions for the specific notice for the V3.

As of Version 1.4.3H, the Compilation Instructions have changed, the GxEPD2 requires a define to be added to the .h file in the Compilation Instructions or the Dark Border will be disabled.

For instructions on usage of the firmware, see "Usage".

Below is a layout of the Watchy Connect & the OTA Website (used while online your network) that gives you options to various things.


![OTA Website](/Images/Server-Help.png)

OTA update is **ONLY** possible via an OTA compatible firmware which are:

`Watchy GSR` and those overriding it.

**If you have an OTA upload function in your firmware, ask for it to be added here.**

For those wanting to override the looks of this Watchy face, look at the Override Information.

**First Timers**

V3:  For those with the 2024 V3 Watchy, see the file in the main folder about it.

To get this setup for compiling, you need to make a folder in your (Arduino) Stream folder or (PlatformIO) project folder, call it GSR, in there, download the "Starting Point.zip" and latest Release (Source Code) .zip from the Release folder above.
Extract both "Starting Point.zip" and the latest release (Source Code .zip) into the GSR folder, this will ensure all the necessary files are there.  If you plan to download any AddOns, be sure to deposit those files into the same GSR folder.  Follow the instructions for including AddOns in the src folder's Compilation Instructions.
