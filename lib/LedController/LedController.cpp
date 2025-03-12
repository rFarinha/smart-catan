#include "LedController.h"
#include <Arduino.h>
#include "LedIndex.h"
#include "adjancency.h"

/**
 * Constructor - initialize controller with pin and LED count
 *
 * @param pin GPIO pin connected to the WS2812B data line
 * @param numLeds Number of LEDs in the strip
 * @param brightness Initial brightness level (0-255)
 */
LedController::LedController(uint8_t pin, uint16_t numLeds, uint8_t brightness)
    : ledPin(pin), ledCount(numLeds), ledBrightness(brightness), strip(nullptr),
      animationRunning(false), animationTaskHandle(NULL)
{
}

/**
 * Destructor - clean up resources
 * Stops any running animation and frees the LED strip object
 */
LedController::~LedController()
{
    stopAnimation();
    if (strip != nullptr)
    {
        delete strip;
    }
}

/**
 * Initialize the LED strip
 * Creates the NeoPixel object and sets initial brightness
 *
 * @param numLeds Number of LEDs to initialize (updates ledCount)
 */
void LedController::begin(uint16_t numLeds)
{
    // Create a new LED strip instance
    ledCount = numLeds;
    if (strip != nullptr)
    {
        delete strip;
    }
    strip = new Adafruit_NeoPixel(ledCount, ledPin, NEO_GRB + NEO_KHZ800);
    strip->begin();
    strip->setBrightness(ledBrightness);
}

/**
 * Reinitialize the LED strip with new LED count
 * Used when switching between classic and extension board modes
 *
 * @param numLeds New number of LEDs
 */
void LedController::restart(uint16_t numLeds)
{
    // Update the LED count and reinitialize the strip
    ledCount = numLeds;
    if (strip != nullptr)
    {
        delete strip;
    }
    strip = new Adafruit_NeoPixel(ledCount, ledPin, NEO_GRB + NEO_KHZ800);
    strip->begin();
    strip->setBrightness(ledBrightness);
}

/**
 * Turn off all LEDs in the strip
 * Sets all pixels to color 0 (off)
 */
void LedController::turnOffAllLeds()
{
    for (uint16_t i = 0; i < ledCount; i++)
    {
        strip->setPixelColor(i, 0);
    }
}

/**
 * Update the LED display
 * Sends color data to the physical LED strip
 */
void LedController::update()
{
    if (strip != nullptr)
    {
        strip->show();
    }
}

/**
 * Set a specific LED to a color
 *
 * @param pixel LED index
 * @param color 32-bit color value
 */
void LedController::setPixelColor(uint16_t pixel, uint32_t color)
{
    if (strip != nullptr)
    {
        strip->setPixelColor(pixel, color);
    }
}

/**
 * Turn on a specific tile using its Catan board position
 * Maps the tile index to the corresponding LED index
 *
 * @param tile Tile index on the Catan board
 * @param color 32-bit color value
 */
void LedController::turnTileOn(uint16_t tile, uint32_t color)
{
    // Map tile index to LED index using the appropriate lookup table
    int ledIndex = ledCount == 30 ? tileToLedIndexExtension[tile] : tileToLedIndexClassic[tile];

    if (strip != nullptr)
    {
        strip->setPixelColor(ledIndex, color);
    }
}

/**
 * Animation for rolling dice
 * Shows a sequence of random colors in a spiral pattern
 */
void LedController::rollDiceAnimation()
{
    // Retrieve the LED count
    uint16_t count = ledCount;

    // LED animation: Turn on LEDs sequentially with random colors
    // For each LED, pick an index based on board mode
    for (int i = count - 1; i >= 0; i--)
    {
        // Use the spiral index mapping for the animation
        int ledIndex = (count == 30 ? spiralLedIndexExtension[i] : spiralLedIndexClassic[i]);

        // Generate a random color
        uint8_t r = random(0, 256);
        uint8_t g = random(0, 256);
        uint8_t b = random(0, 256);
        uint32_t color = Color(r, g, b);

        setPixelColor(ledIndex, color);
        update();  // Update the strip to show the new color
        delay(50); // Short delay between steps
    }
}

/**
 * Create a 32-bit color value from RGB components
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return 32-bit color value
 */
uint32_t LedController::Color(uint8_t r, uint8_t g, uint8_t b)
{
    if (strip != nullptr)
    {
        return strip->Color(r, g, b);
    }
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

/**
 * Get a pointer to the NeoPixel strip
 *
 * @return Pointer to the Adafruit_NeoPixel object
 */
Adafruit_NeoPixel *LedController::getStrip()
{
    return strip;
}

// ----- Animation Functions -----

/**
 * Start an LED animation
 *
 * Creates a FreeRTOS task to run the animation in the background
 *
 * @param animationId Type of animation to run (WAITING_ANIMATION, START_GAME_ANIMATION, ROBBER_ANIMATION)
 * @param tiles Array of tile indices (for ROBBER_ANIMATION)
 * @param numTiles Number of tiles in the array
 * @param delayMs Delay between animation steps in milliseconds
 */
void LedController::startAnimation(uint8_t animationId, uint16_t *tiles, uint8_t numTiles, uint32_t delayMs)
{
    // Do not start if an animation is already running
    if (animationRunning)
    {
        Serial.println("Animation still running!");
        return;
    }
    animationRunning = true;

    // Prepare parameters for the animation task
    AnimationParams *params = new AnimationParams;
    params->animationId = animationId;
    params->tiles = tiles;
    params->numTiles = numTiles;
    params->delayMs = delayMs;
    params->instance = this;

    // Create the animation task on core 1
    xTaskCreatePinnedToCore(
        animationTask,        // Task function
        "LedAnimationTask",   // Name of task
        4096,                 // Stack size (words)
        (void *)params,       // Parameters
        1,                    // Priority
        &animationTaskHandle, // Task handle
        1                     // Core where the task should run
    );
}

/**
 * Stop any currently running animation
 * Sets the stop flag and waits for the task to complete
 */
void LedController::stopAnimation()
{
    if (animationRunning)
    {
        animationRunning = false;
        // Wait until the animation task has ended
        while (animationTaskHandle != NULL)
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}

/**
 * Animation task function - runs animations in a separate FreeRTOS task
 *
 * @param pvParameters Pointer to AnimationParams structure
 */
void LedController::animationTask(void *pvParameters)
{
    Serial.println("START TASK!");
    AnimationParams *params = static_cast<AnimationParams *>(pvParameters);
    LedController *instance = params->instance;
    uint32_t delayMs = params->delayMs;

    // Choose which animation to run based on the animation ID
    switch (params->animationId)
    {
    case WAITING_ANIMATION:
        // Waiting animation: Light LEDs white one at a time; then turn them off in reverse order
        while (instance->animationRunning)
        {
            // Turn on LEDs sequentially
            for (uint16_t i = 0; i < instance->ledCount && instance->animationRunning; i++)
            {
                // Pick the correct LED index using the spiral pattern
                int ledIndex = instance->ledCount == 30 ? spiralLedIndexExtension[i] : spiralLedIndexClassic[i];
                instance->strip->setPixelColor(ledIndex, instance->Color(255, 255, 255));
                instance->strip->show();
                vTaskDelay(delayMs / portTICK_PERIOD_MS);
            }

            // Turn off LEDs sequentially (reverse order)
            for (int i = instance->ledCount - 1; i >= 0 && instance->animationRunning; i--)
            {
                int ledIndex = instance->ledCount == 30 ? spiralLedIndexExtension[i] : spiralLedIndexClassic[i];
                instance->strip->setPixelColor(ledIndex, 0);
                instance->strip->show();
                vTaskDelay(delayMs / portTICK_PERIOD_MS);
            }
        }
        break;

    case START_GAME_ANIMATION:
        // Start Game Animation: Blink all LEDs white 3 times then turn off
        for (int j = 0; j < 3 && instance->animationRunning; j++)
        {
            // All on
            for (uint16_t i = 0; i < instance->ledCount; i++)
            {
                instance->strip->setPixelColor(i, instance->Color(255, 255, 255));
            }
            instance->strip->show();
            vTaskDelay(delayMs / portTICK_PERIOD_MS);

            // All off
            for (uint16_t i = 0; i < instance->ledCount; i++)
            {
                instance->strip->setPixelColor(i, 0);
            }
            instance->strip->show();
            vTaskDelay(delayMs / portTICK_PERIOD_MS);
        }
        // Ensure LEDs are off at the end
        for (uint16_t i = 0; i < instance->ledCount; i++)
        {
            instance->strip->setPixelColor(i, 0);
        }
        instance->strip->show();
        break;

    case ROBBER_ANIMATION:
    {
        Serial.println("ROBBER!");
        Serial.println(params->numTiles);

        uint32_t delayMs = params->delayMs;

        // Robber Animation: requires 1 or 2 tile indices in params->tiles
        if (params->numTiles > 0)
        {
            // Determine board mode based on ledCount
            // Classic: 19 LEDs, Extension: 30 LEDs
            bool isExtension = (instance->ledCount == 30);
            int tileCount = isExtension ? 30 : 19;
            const int *tileToLedIndex = isExtension ? tileToLedIndexExtension : tileToLedIndexClassic;
            const int(*adjacencyList)[6] = isExtension ? adjacencyListExtension : adjacencyListClassic;

            // Processed array for BFS (size 30 covers both modes)
            bool processed[30] = {false};

            // A queue for BFS
            uint16_t queue[30];
            int queueEnd = 0; // Points to the end of the queue

            // Initialize the queue with the provided robber tile(s) - usually desert tiles
            for (uint16_t j = 0; j < params->numTiles; j++)
            {
                uint16_t tile = params->tiles[j];
                Serial.print("Starting tile: ");
                Serial.println(tile);
                if (tile < tileCount)
                {
                    processed[tile] = true;
                    queue[queueEnd++] = tile;
                    // Set the LED for this tile to red
                    uint16_t ledIndex = tileToLedIndex[tile];
                    Serial.print("Turning on LED for tile ");
                    Serial.print(tile);
                    Serial.print(" at LED index ");
                    Serial.println(ledIndex);
                    instance->strip->setPixelColor(ledIndex, instance->Color(255, 0, 0));
                }
            }
            instance->strip->show();
            vTaskDelay(delayMs / portTICK_PERIOD_MS);

            // Process the BFS level-by-level (each level becomes a "wave")
            int currentLevelStart = 0; // Start index of the current level in the queue
            while (currentLevelStart < queueEnd && instance->animationRunning)
            {
                int nextLevelStart = queueEnd; // New nodes will be appended starting here

                // Process all nodes in the current level
                for (int i = currentLevelStart; i < nextLevelStart && instance->animationRunning; i++)
                {
                    uint16_t currentTile = queue[i];
                    Serial.print("Processing tile: ");
                    Serial.println(currentTile);

                    // Check each neighbor
                    for (int k = 0; k < 6 && instance->animationRunning; k++)
                    {
                        int neighbor = adjacencyList[currentTile][k];
                        if (neighbor != -1 && neighbor < tileCount && !processed[neighbor])
                        {
                            processed[neighbor] = true;
                            queue[queueEnd++] = neighbor;
                            // Set the neighbor LED to red (but do not show yet)
                            uint16_t ledIndex = tileToLedIndex[neighbor];
                            Serial.print("Queuing neighbor tile: ");
                            Serial.print(neighbor);
                            Serial.print(" at LED index ");
                            Serial.println(ledIndex);
                            instance->strip->setPixelColor(ledIndex, instance->Color(255, 0, 0));
                        }
                    }
                }

                // Now, all nodes added in this round form the next wave
                if (nextLevelStart < queueEnd)
                {
                    Serial.println("New wave:");
                    // Optionally, print details of the new wave
                    for (int i = nextLevelStart; i < queueEnd && instance->animationRunning; i++)
                    {
                        uint16_t tile = queue[i];
                        uint16_t ledIndex = tileToLedIndex[tile];
                        Serial.print("Wave tile: ");
                        Serial.print(tile);
                        Serial.print(" at LED index ");
                        Serial.println(ledIndex);
                    }
                    // Update the strip for the entire wave and delay
                    instance->strip->show();
                    vTaskDelay(delayMs / portTICK_PERIOD_MS);
                }

                // Move to the next level
                currentLevelStart = nextLevelStart;
            }
        }
    }
    break;

    default:
        break;
    }

    // Clean up before ending the task
    delete[] params->tiles;
    instance->animationRunning = false;
    instance->animationTaskHandle = NULL;
    delete params;
    vTaskDelete(NULL);
}