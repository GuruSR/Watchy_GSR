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

**What to do when the above fails:**

1. KEEP the Merged Bin file from above!
2. Visit [ESPTOOL](<https://github.com/espressif/esptool/releases> "ESPTool") and download the version for your system.
3. Extract the downloaded archive to a location you can remember.
4. Open a terminal/shell/command line and change the current path to the location where you extracted the archive.
5. If you're archive has esptool.py in it, you need to have the .py on the end of the command.
6. If you're flashing a V3, you need to add s3 to the chip name.
7. Use the command in 10 to flash the firmware, be sure to note the \[\] & \(\) segments to fill them with the necessary information.
8. Remove the `[s3]` section if you don't have an V3 Watchy and you're not using esptool.py `[.py]`.
9. Remove the \[ and \] surrounding anything you do need.  Do not include the \( or \) in your final command.
10. esptool\[.py\] -p \(PORT\)  --before default_reset --after hard_reset --chip esp32\[s3\] write_flash --flash_mode dio --flash_size detect --flash_freq 80m 0x0 \(location and filename of your downloaded Merged Bin\)
11. Once it flashes, V1 to V2 will reboot on their own, V3 requires releasing the top right button after holding both top buttons for ~5 seconds.
