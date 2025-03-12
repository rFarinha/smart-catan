/**
 * LedController.h
 *
 * This header defines the LedController class which manages
 * the WS2812B LED strip for visual Catan board display.
 *
 * It handles:
 * - Basic LED control (colors, brightness, etc.)
 * - Board visualization
 * - Animations (waiting, start game, robber)
 * - Mapping between tile positions and LED positions
 */

#ifndef LEDCONTROLLER_H
#define LEDCONTROLLER_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

/**
 * Animation ID constants
 * Defines the types of animations supported by the controller
 */
enum AnimationID
{
    WAITING_ANIMATION = 0,    // Turn on LEDs one by one (white) then off in reverse order in a loop
    START_GAME_ANIMATION = 1, // Blink all LEDs 3 times
    ROBBER_ANIMATION = 2      // Light specified tile(s) red, then sequentially light remaining tiles
};

/**
 * LedController class
 *
 * Manages WS2812B LED strip operations for the Catan board visualization
 */
class LedController
{
public:
    /**
     * Constructor
     *
     * @param pin GPIO pin connected to LED data line
     * @param numLeds Number of LEDs in the strip
     * @param brightness LED brightness (0-255, default 50)
     */
    LedController(uint8_t pin, uint16_t numLeds, uint8_t brightness = 50);

    /**
     * Destructor
     * Ensures animations are stopped and resources freed
     */
    ~LedController();

    /**
     * Initialize the LED strip
     * @param numLeds Number of LEDs to initialize
     */
    void begin(uint16_t numLeds);

    /**
     * Reinitialize the LED strip with a new LED count
     * Used when switching between classic and extension boards
     *
     * @param numLeds New number of LEDs
     */
    void restart(uint16_t numLeds);

    /**
     * Turn off all LEDs on the strip
     */
    void turnOffAllLeds();

    /**
     * Update the LED strip display
     * Pushes current colors to the physical LEDs
     */
    void update();

    /**
     * Set the color of a specific LED
     *
     * @param pixel LED index
     * @param color 32-bit color value (WRGB format)
     */
    void setPixelColor(uint16_t pixel, uint32_t color);

    /**
     * Turn on a specific tile using its tile index
     * Maps tile index to the corresponding LED index
     *
     * @param tile Tile index (0-18 for classic, 0-29 for extension)
     * @param color 32-bit color value (WRGB format)
     */
    void turnTileOn(uint16_t tile, uint32_t color);

    /**
     * Create a color value from RGB components
     *
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     * @return 32-bit color value
     */
    uint32_t Color(uint8_t r, uint8_t g, uint8_t b);

    /**
     * Get access to the underlying NeoPixel strip
     * @return Pointer to the Adafruit_NeoPixel object
     */
    Adafruit_NeoPixel *getStrip();

    /**
     * Run dice roll animation
     * Shows colorful random patterns when dice are rolled
     */
    void rollDiceAnimation();

    // ----- Animation Functions -----

    /**
     * Start an animation sequence
     *
     * @param animationId Animation type (WAITING_ANIMATION, START_GAME_ANIMATION, ROBBER_ANIMATION)
     * @param tiles Array of tile indices (used for ROBBER_ANIMATION)
     * @param numTiles Number of tiles in the array
     * @param delayMs Delay between animation steps in milliseconds
     */
    void startAnimation(uint8_t animationId, uint16_t *tiles = nullptr, uint8_t numTiles = 0, uint32_t delayMs = 500);

    /**
     * Stop any currently running animation
     */
    void stopAnimation();

private:
    uint8_t ledPin;           // GPIO pin connected to LED strip
    uint16_t ledCount;        // Number of LEDs in the strip
    uint8_t ledBrightness;    // Brightness level (0-255)
    Adafruit_NeoPixel *strip; // Pointer to the NeoPixel strip object

    // Animation control variables
    volatile bool animationRunning;   // Flag indicating if an animation is active
    TaskHandle_t animationTaskHandle; // Handle to the FreeRTOS animation task

    /**
     * Animation parameters structure
     * Used to pass data to the animation task
     */
    struct AnimationParams
    {
        uint8_t animationId;     // Animation type ID
        uint16_t *tiles;         // Array of tile indices (for robber animation)
        uint16_t numTiles;       // Number of tiles provided
        uint32_t delayMs;        // Animation timing parameter
        LedController *instance; // Pointer to LedController instance
    };

    /**
     * Static animation task function
     * Runs the animation in a separate FreeRTOS task
     *
     * @param pvParameters Pointer to AnimationParams structure
     */
    static void animationTask(void *pvParameters);
};

#endif