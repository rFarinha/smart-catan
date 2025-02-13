#ifndef LEDSTRIPCONTROL_H
#define LEDSTRIPCONTROL_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

void restartLedStrip(Adafruit_NeoPixel *strip, int currentNumLeds, int ledStripPin);

#endif