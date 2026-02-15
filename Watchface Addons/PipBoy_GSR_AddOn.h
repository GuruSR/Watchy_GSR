#include "PipBoy_GSR_AddOn_Media.h"


RTC_DATA_ATTR uint8_t PipBoyWatchfaceStyle;
int PipBoyWatchfaceState = 0;


class PipBoyAddOnClass : public WatchyGSR {
public:
  PipBoyAddOnClass()
    : WatchyGSR() {
    initAddOn(this);
  }

  void RegisterWatchFaces() {
    PipBoyWatchfaceStyle = AddWatchStyle("PipBoy", this);
  };

  void InsertInitWatchStyle(uint8_t StyleID) {
    if (StyleID == PipBoyWatchfaceStyle) {
      WantWeather(true);
      Design.Face.TimeFont = &monofonto28pt7b;
      Design.Face.DateFontSmall = &monofonto8pt7b;
      Design.Face.DateFont = &monofonto10pt7b;
      Design.Status.Inverted = true;
    }
  };



  void InsertDrawWatchStyle(uint8_t StyleID) {
    if (StyleID == PipBoyWatchfaceStyle) {
      if (SafeToDraw()) {
        display.setFont(Design.Face.DateFontSmall);
        display.setTextColor(ForeColor());
        display.drawBitmap(0, 10, vaultboymenubar, 200, 9, ForeColor(), BackColor());
        display.setCursor(22, 14);
        display.print("STAT  INV  DATA  MAP");

        display.setFont(Design.Face.DateFontSmall);
        display.setCursor(10, 195);
        display.println("PIP-BOY 3000 ROBCO IND.");

        drawTime();
        drawDate();
        drawSteps();
        drawWeather();
        drawBattery();
      }
    }
  };


  uint8_t vaultBoyNum = 0;
  const uint16_t STEPSGOAL = 8000;

  void drawTime() {

    if (WatchTime.Local.Minute % 15 == 0) {
      vaultBoyNum = random(0, 3);
    }

    switch (vaultBoyNum) {
      case 0:
        display.drawBitmap(70, 50, vaultboy, 57, 100, ForeColor(), BackColor());
        break;
      case 1:
        display.drawBitmap(70, 50, vaultboypoint, 57, 100, ForeColor(), BackColor());
        break;
      case 2:
        display.drawBitmap(60, 50, vaultboysmile, 67, 100, ForeColor(), BackColor());
        break;
    }

    display.drawLine(137, 28, 200, 28, ForeColor());
    display.drawLine(137, 28, 137, 132, ForeColor());


    display.setFont(Design.Face.TimeFont);
    display.setCursor(141, 125);
    display.print(MakeMinutes(WatchTime.Local.Minute));
    display.setCursor(141, 75);

    if(IsAM() || IsPM()) {
      display.print(MakeMinutes((WatchTime.Local.Hour + 11) % 12 + 1));

      display.drawLine(137, 132, 157, 132, ForeColor());
      display.drawLine(180, 132, 200, 132, ForeColor());
      display.setCursor(160, 140);
      display.setFont(Design.Face.DateFontSmall);
      display.print(WatchTime.Local.Hour < 11 ? "AM" : "PM");
    }
    else {
      display.print(MakeMinutes(WatchTime.Local.Hour));
      display.drawLine(137, 132, 200, 132, ForeColor());
    }
  }

  void drawDate() {

    display.setFont(Design.Face.DateFont);
    int16_t x1, y1;
    uint16_t w, h;
    String dayOfWeek = dayStr(WatchTime.Local.Wday + 1);
    dayOfWeek.toUpperCase();
    display.setTextColor(BackColor());
    display.getTextBounds(dayOfWeek, 7, 42, &x1, &y1, &w, &h);
    display.setCursor(7, 42);
    display.fillRect(x1 - 2, y1 - 2, w + 4, h + 4, ForeColor());
    display.print(dayOfWeek);

    display.setFont(Design.Face.DateFont);
    display.setTextColor(ForeColor());
    display.setCursor(7, 62);
    display.print(monthShortStr(WatchTime.Local.Month + 1));
    display.print(" ");
    display.print(WatchTime.Local.Day);
    display.setCursor(7, 78);
    display.print(WatchTime.Local.Year + SRTC.getLocalYearOffset());
  }

  void drawSteps() {
    if (BMAAvailable()) {
      display.setFont(Design.Face.DateFontSmall);
      display.setTextColor(ForeColor());
      display.setCursor(150, 160);
      display.print("STEPS");
      display.setCursor(150, 175);
      display.print(CurrentStepCount());

      uint8_t progress = (uint8_t)(CurrentStepCount() * 100.0 / STEPSGOAL);
      progress = progress > 100 ? 100 : progress;
      display.drawBitmap(60, 155, vaultboygauge, 73, 10, ForeColor(), BackColor());
      display.fillRect(60 + 13, 155 + 5, (progress / 2) + 5, 4, ForeColor());
    }
  }

  void drawBattery() {
    display.drawBitmap(10, 150, vaultboybattery, 37, 21, ForeColor(), BackColor());
    display.fillRect(15, 155, 27, 11, BackColor());  //clear battery segments
    int8_t batteryLevel = 0;
    float VBAT = getBatteryVoltage();
    if (VBAT > 4.1) {
      batteryLevel = 3;
    } else if (VBAT > 3.95 && VBAT <= 4.1) {
      batteryLevel = 2;
    } else if (VBAT > 3.80 && VBAT <= 3.95) {
      batteryLevel = 1;
    } else if (VBAT <= 3.80) {
      batteryLevel = 0;
    }

    for (int8_t batterySegments = 0; batterySegments < batteryLevel; batterySegments++) {
      display.fillRect(15 + (batterySegments * 9), 155, 7, 11, ForeColor());
    }
  }

  void drawWeather() {
    if (IsWeatherAvailable()) {
      const unsigned char* weatherIcon = getWeatherIcon(GetWeatherID());
      display.drawBitmap(5, 85, weatherIcon, 48, 32, ForeColor(), BackColor());

      String T = String(GetWeatherTemperatureFeelsLike());
      display.setFont(Design.Face.DateFont);
      display.setTextColor(ForeColor());
      display.setCursor(12, 133);
      display.print(T);
      display.print(IsMetric() ? "C" : "F");
    }
  }


};

PipBoyAddOnClass PipBoyAddOnClassLoader;
