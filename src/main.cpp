#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include "config.hpp"
#include "MyCom.hpp"
#include "MyGPS.hpp"
#include "MyDisplay.hpp"

using namespace lacar::droid_basestation::firmware;

void setup() {
  Serial.begin(115200);

  SPI.begin();
  Wire.begin();

  MyDisplay::getInstance().setup();
  MyCom::getInstance().setup();
  MyGPS::getInstance().setup();
}

void loop() {
  MyDisplay::getInstance().loop();
  MyCom::getInstance().loop();
  MyGPS::getInstance().loop();
}
