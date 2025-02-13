#ifndef LEDSTRIPCONTROLLER_H
#define LEDSTRIPCONTROLLER_H

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

class LEDStripControl {
public:
    // Constructor: initializes the LED strip with a given number of LEDs and pin.
    LEDStripControl(int numLeds, int pin);
    
    // Destructor: cleans up dynamic allocation.
    ~LEDStripControl();

    // Getter for the number of LEDs.
    int getNumLeds() const;

    // Setter for the number of LEDs.
    // If the new number is different, the LED strip is restarted.
    void setNumLeds(int newNumLeds);

    // Restart the LED strip (reinitializes the object).
    void restart();

    // Example methods to control the LED strip.
    void setColor(uint32_t color, int index);
    void show();

    // Convenience: a helper to generate a color value.
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b);

private:
    Adafruit_NeoPixel *strip;
    int numLeds;
    int pin;
};

#endif // LEDSTRIPCONTROLLER_H