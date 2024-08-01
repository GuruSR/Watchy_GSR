The above files are in two types:

Regular OTA:

&nbsp;&nbsp;&nbsp;&nbsp;Watchy Versions 1.0 to 2.0: GSR.ino.esp32.bin<br>
&nbsp;&nbsp;&nbsp;&nbsp;Watchy Version 3.0 ONLY:&nbsp;&nbsp; GSR.ino.esp32s3.bin


Merged Bin File for full upload:

&nbsp;&nbsp;&nbsp;&nbsp;Watchy Versions 1.0 to 2.0: GSR.ino.esp32.merged.bin<br>
&nbsp;&nbsp;&nbsp;&nbsp;Watchy Version 3.0 ONLY:&nbsp;&nbsp;GSR.ino.esp32s3.merged.bin


To use OTA bin files above:
1. With Watchy GSR, go to Options -> OTA Website.
2. Browse to the OTA Website, you can select to Upload Firmware.
3. Select the downloaded bin for your Watchy version.
4. Click Upload to upload the firmware.
5. Once the upload has finished all Watchy versions will automatically reboot when the upload is complete.

**For those who DO NOT WANT TO COMPILE:**

1. Download the Merged Bin file for your version.
2. Visit using a Chromium based browser [Adafruit ESPTool](<https://adafruit.github.io/Adafruit_WebSerial_ESPTool/> "Adafruit ESPTool")
3. Plug in your Watchy (if not already done).
4. **V3 Users, hold the top two buttons down for at least 4 seconds, then release the top left one.**
5. Click Connect and pick your Watchy and click Connect again.
6. You should see 4 "Choose a file..." buttons, press the first one, pick the Merged Bin you downloaded.
7. Once you've selected the file, the Program button will be present.
8. Click the Program button to upload the firmware.
9. Once the upload has finished all Watchy versions will automatically reboot when the upload is complete.
