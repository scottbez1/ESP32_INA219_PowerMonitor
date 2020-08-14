#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Adafruit_INA219.h>

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

Adafruit_INA219 ina219;

/**
 * Pinout:
 *  LCD
 *    GND=GND
 *    VCC=3.3V
 *    CLK=18
 *    MOSI=19
 *    RES=23
 *    DC=16
 *    BLK=4
 *  INA219
 *    VCC=3.3V
 *    GND=GND
 *    SCL=22 (+external I2C pull-up)
 *    SDA=21 (+external I2C pull-up)
 */

void setup() {
  tft.begin();
  tft.invertDisplay(1);
  tft.setRotation(0);
  tft.setFreeFont(&FreeSans12pt7b);
  tft.setTextDatum(TL_DATUM);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(0xFFFF, TFT_BLACK);

  Serial.begin(921600);

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 20);
  tft.print("Hi!");

  spr.setColorDepth(16);
  spr.createSprite(TFT_WIDTH, TFT_HEIGHT);
  spr.setFreeFont(&FreeSans12pt7b);
  spr.setTextDatum(TL_DATUM);
  spr.fillScreen(TFT_BLACK);
  spr.setTextColor(0xFFFF, TFT_BLACK);

  if (! ina219.begin()) {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 20);
    tft.print("Failed to find INA219 chip");
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  Wire.setClock(400000);
  Serial.printf("\n\n\n\n\n\n\n\nrunningTimeMicros,energymWh,voltage,currentmA\n");
}


unsigned long lastMeaurementTimeMicros = 0;
float energy_mW_us = 0;

uint64_t runningTimeMicros = 0;

unsigned long lastDisplayTimeMicros = 0;



void loop() {
  unsigned long timeMicros = micros();

  float power_mW = ina219.getPower_mW();

  if (lastMeaurementTimeMicros > 0) {
    unsigned long deltaMicros = (timeMicros - lastMeaurementTimeMicros);
    energy_mW_us += power_mW * deltaMicros;
    runningTimeMicros += deltaMicros;
  }
  lastMeaurementTimeMicros = timeMicros;

  if (timeMicros - lastDisplayTimeMicros > 500000) {
    float voltage = ina219.getBusVoltage_V();
    float current_mA = ina219.getCurrent_mA();
    Serial.printf("%llu,%f,%f,%f\n", runningTimeMicros, energy_mW_us / 1000000 / 60 / 60, voltage, current_mA);

    static char buf[100];
    snprintf(
      buf,
      sizeof(buf),
      "Time: %0.3f h\nEnergy: %0.2f mWh\n\nVoltage: %0.3fV\nCurrent: %0.1fmA\n", 
      (float)runningTimeMicros / 1000000 / 60 / 60,
      energy_mW_us / 1000000 / 60 /60,
      voltage,
      current_mA
    );
    spr.fillSprite(TFT_BLACK);
    spr.setCursor(0, 20);
    spr.print(buf);
    spr.pushSprite(0, 0);
  }
}