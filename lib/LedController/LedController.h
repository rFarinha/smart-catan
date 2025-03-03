#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// LED Animation IDs
enum AnimationID
{
    WAITING_ANIMATION = 0,    // Turn on LEDs one by one (white) then off in reverse order in a loop.
    START_GAME_ANIMATION = 1, // Blink all LEDs 3 times.
    ROBBER_ANIMATION = 2      // Light specified tile(s) red, then sequentially light remaining tiles.
};

class LedController
{
public:
    // Constructor: provide the pin number, LED count, and optional brightness (default 50)
    LedController(uint8_t pin, uint16_t numLeds, uint8_t brightness = 50);
    ~LedController();

    // Initialize and restart the LED strip
    void begin();
    void restart(uint16_t numLeds);

    // Basic LED functions
    void turnOffAllLeds();
    void update();
    void setPixelColor(uint16_t pixel, uint32_t color);

    // Turn specific number LED on based on led index
    void turnTileOn(uint16_t tile, uint32_t color);

    // Utility to create a color (same as Adafruit_NeoPixel::Color)
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b);

    // (Optional) Get access to the underlying strip pointer
    Adafruit_NeoPixel *getStrip();

    // Run the dice roll LED animation.
    void rollDiceAnimation();

    // ----- Animation Functions -----
    // Starts an animation based on the provided ID.
    // For ROBBER_ANIMATION (id==2), provide an array of tile indices and its count.
    // delayMs is used for pacing the animation.
    void startAnimation(uint8_t animationId, uint16_t *tiles = nullptr, uint8_t numTiles = 0, uint32_t delayMs = 500);

    // Stops any running animation.
    void stopAnimation();

private:
    uint8_t ledPin;
    uint16_t ledCount;
    uint8_t ledBrightness;
    Adafruit_NeoPixel *strip;

    // Animation control
    volatile bool animationRunning;
    TaskHandle_t animationTaskHandle;

    // Structure to pass parameters to the animation task.
    struct AnimationParams
    {
        uint8_t animationId; // 0=waiting, 1=start game, 2=robber
        uint16_t *tiles;     // For robber animation: pointer to tile indices (1 or 2 tiles)
        uint16_t numTiles;   // Number of tiles provided for robber animation
        uint32_t delayMs;    // Delay between animation steps (in milliseconds)
        LedController *instance;
    };

    // The static animation task function.
    static void animationTask(void *pvParameters);
};

#endif