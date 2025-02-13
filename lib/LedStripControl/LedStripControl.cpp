#include "LedStripControl.h"

void restartLedStrip(Adafruit_NeoPixel *strip, int currentNumLeds, int ledStripPin)
{
    // Clean up the old LED strip object
    if (strip != nullptr)
    {
        delete strip;
        strip = nullptr;
    }

    // Create a new LED strip object
    strip = new Adafruit_NeoPixel(currentNumLeds, ledStripPin, NEO_GRB + NEO_KHZ800);
}
