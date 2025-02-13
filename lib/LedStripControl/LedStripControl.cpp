#include "LEDStripControl.h"

LEDStripControl::LEDStripControl(int numLeds, int pin)
    : numLeds(numLeds), pin(pin)
{
    strip = new Adafruit_NeoPixel(numLeds, pin, NEO_GRB + NEO_KHZ800);
    strip->begin(); 
    strip->show(); // Initialize all pixels to 'off'
}

LEDStripControl::~LEDStripControl() {
    if (strip != nullptr) {
        delete strip;
        strip = nullptr;
    }
}

int LEDStripControl::getNumLeds() const {
    return numLeds;
}

void LEDStripControl::setNumLeds(int newNumLeds) {
    if (newNumLeds != numLeds) {
        numLeds = newNumLeds;
        restart();
    }
}

void LEDStripControl::restart() {
    if (strip != nullptr) {
        delete strip;
        strip = nullptr;
    }
    strip = new Adafruit_NeoPixel(numLeds, pin, NEO_GRB + NEO_KHZ800);
    strip->begin();
    strip->show();
}

void LEDStripControl::setColor(uint32_t color, int index) {
    if (strip != nullptr && index >= 0 && index < numLeds) {
        strip->setPixelColor(index, color);
    }
}

void LEDStripControl::show() {
    if (strip != nullptr) {
        strip->show();
    }
}

uint32_t LEDStripControl::Color(uint8_t r, uint8_t g, uint8_t b) {
    return strip->Color(r, g, b);
}
