// TODO HOME ASSISTANT
// TODO CLEANING

// External
#include <Arduino.h>
#include <WebServer.h>
#include <FS.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// Internal
#include "BoardGenerator.h"
#include "password.h"
#include "WebPage.h"
#include "LedController.h"

// Increase this value if you continue to get stack overflows.
#define BOARD_GEN_STACK_SIZE 8192
#define BOARD_GEN_TASK_PRIORITY 1
#define BOARD_GEN_TASK_CORE 1 // or 0, depending on your design

volatile bool boardReady = true; // global flag
bool gameLoaded = false;

// ------------- WIFI Credentials -------------
#ifndef PASSWORD_H
#define PASSWORD_H
const char *WIFI_SSID = "PlaceholderSSID";
const char *WIFI_PASS = "PlaceholderPassword";
#endif

// ------------- PINS & LED STRIP -------------
#define LED_STRIP_PIN 4
#define LED_COUNT_CLASSIC 19
#define LED_COUNT_EXTENSION 30

// Create a global instance of the LedController
LedController ledController(LED_STRIP_PIN, LED_COUNT_CLASSIC);

int selectedNumber;

//---------------SETTINGS----------------------
#define DEFAULT_EIGHT_SIX_CANTOUCH true
#define DEFAULT_TWO_TWELVE_CANTOUCH true
#define DEFAULT_SAMENUMBERS_CANTOUCH true
#define DEFAULT_SAMERESOURCE_CANTOUCH true
#define DEFAULT_MANUAL_DICE false
#define DEFAULT_IS_EXTENSION false

bool manualDice;

bool gameStarted;

// ------------- WEB SERVER -------------------
WebServer server(80);
// store HTML content
String htmlPage;

// ------------- CATAN DATA -------------------
Board board;
BoardConfig boardConfig;

//---------------------------------------------------------------
//                 UTILS
//---------------------------------------------------------------

String generateJSON()
{
  // Create a JSON document.
  // Allocate the JSON document
  JsonDocument doc;
  int ledNumber;

  if (!boardConfig.isExtension)
  {
    ledNumber = LED_COUNT_CLASSIC;
  }
  else
  {
    ledNumber = LED_COUNT_EXTENSION;
  }

  // Add the resources array.
  JsonArray resources = doc["resources"].to<JsonArray>();
  for (int i = 0; i < ledNumber; i++)
  {
    resources.add(board.resources[i]);
  }

  // Add the numbers array.
  JsonArray numbers = doc["numbers"].to<JsonArray>();
  for (int i = 0; i < ledNumber; i++)
  {
    numbers.add(board.numbers[i]);
  }

  // Include the game mode flag (classic mode => Extension is false)
  doc["extension"] = boardConfig.isExtension;
  doc["gameStarted"] = gameStarted;

  // Include the game settings
  doc["eightSixCanTouch"] = boardConfig.eightSixCanTouch;
  doc["twoTwelveCanTouch"] = boardConfig.twoTwelveCanTouch;
  doc["sameNumbersCanTouch"] = boardConfig.sameNumbersCanTouch;
  doc["sameResourceCanTouch"] = boardConfig.sameResourceCanTouch;
  doc["manualDice"] = manualDice;

  // Include Turn on led
  doc["selectedNumber"] = selectedNumber;

  // Serialize the JSON document to a string.
  String jsonResponse;
  serializeJson(doc, jsonResponse);

  return jsonResponse;
}

// Save the current game state to flash
void saveGameState()
{
  // Generate the json data to send to the webpage
  String jsonString = generateJSON();

  // Write to SPIFFS
  File file = SPIFFS.open("/gamestate.json", FILE_WRITE);
  if (!file)
  {
    Serial.println("Failed to open file for writing");
    return;
  }
  file.print(jsonString);
  file.close();
  Serial.println("Game state saved to flash.");
}

// Delete the saved game state from flash
void deleteGameState()
{
  if (SPIFFS.exists("/gamestate.json"))
  {
    SPIFFS.remove("/gamestate.json");
    Serial.println("Game state deleted from flash.");
  }
}

void loadGameState()
{
  Serial.print("Load Start!");
  if (SPIFFS.exists("/gamestate.json"))
  {
    Serial.print("Gamestate.json exists");
    File file = SPIFFS.open("/gamestate.json", FILE_READ);
    if (!file)
    {
      Serial.println("Failed to open game state file for reading");
      return;
    }
    String jsonString = file.readString();
    Serial.print("JSON STRING FROM FILE: ");
    Serial.println(jsonString);
    file.close();

    // Create a dynamic JSON document.
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error)
    {
      Serial.print("Failed to parse game state: ");
      Serial.println(error.f_str());
      return;
    }

    // Load board configuration.
    boardConfig.isExtension = doc["extension"];
    boardConfig.eightSixCanTouch = doc["eightSixCanTouch"];
    boardConfig.twoTwelveCanTouch = doc["twoTwelveCanTouch"];
    boardConfig.sameNumbersCanTouch = doc["sameNumbersCanTouch"];
    boardConfig.sameResourceCanTouch = doc["sameResourceCanTouch"];
    manualDice = doc["manualDice"];

    gameStarted = doc["gameStarted"];
    selectedNumber = doc["selectedNumber"];

    // Load board resources and numbers.
    board.resources.clear();
    board.numbers.clear();

    JsonArray resources = doc["resources"].as<JsonArray>();
    for (JsonVariant v : resources)
    {
      board.resources.push_back(v.as<int>());
    }

    JsonArray numbers = doc["numbers"].as<JsonArray>();
    for (JsonVariant v : numbers)
    {
      board.numbers.push_back(v.as<int>());
    }
    Serial.println("Game state loaded from flash.");
  }
  else
  {
    Serial.println("Game state loaded from flash failed!");
  }
}

// --------------------------------------------------------------
//                  SERVER HANDLERS
// --------------------------------------------------------------
// This task runs board generation in its own FreeRTOS task.
void boardGenerationTask(void *pvParameters)
{
  Serial.println("Board generation task started.");

  // Use the current boardConfig to generate a board.
  board = generateBoard(boardConfig);

  // You could now signal that the board is ready, update LEDs, etc.
  Serial.println("Board generation complete.");
  boardReady = true;

  // Delete the task when finished if it's a one-off.
  vTaskDelete(NULL);
}

void createBoardTask()
{
  // Reset flag and create the board generation task
  if (boardReady)
  {
    boardReady = false;
    xTaskCreatePinnedToCore(
        boardGenerationTask,
        "BoardGenTask",
        8192, // Increased stack size
        NULL,
        1, // Priority
        NULL,
        1 // Run on core 1 (adjust if needed)
    );

    // Wait until the board generation task is done.
    // (Be cautious with long delays; adjust timeout as needed.)
    while (!boardReady)
    {
      delay(10); // Yield to other tasks
    }
  }
}

// Serve the main HTML page
void handleRoot()
{
  server.send(200, "text/html", htmlPage);
}

// Update Config: 6 & 8 Can Touch
void handleUpdateEightSixCanTouch()
{
  String value = server.arg("value"); // Expecting "1" for true or "0" for false
  boardConfig.eightSixCanTouch = (value == "1");
  Serial.print("8 & 6 Can Touch set to: ");
  Serial.println(boardConfig.eightSixCanTouch ? "true" : "false");
  server.send(200, "text/plain", "eightSixCanTouch updated");
}

// Update Config: 2 & 12 Can Touch
void handleUpdateTwoTwelveCanTouch()
{
  String value = server.arg("value");
  boardConfig.twoTwelveCanTouch = (value == "1");
  Serial.print("2 & 12 Can Touch set to: ");
  Serial.println(boardConfig.twoTwelveCanTouch ? "true" : "false");
  server.send(200, "text/plain", "twoTwelveCanTouch updated");
}

// Update Config: Same Numbers Can Touch
void handleUpdateSameNumbersCanTouch()
{
  String value = server.arg("value");
  boardConfig.sameNumbersCanTouch = (value == "1");
  Serial.print("Same Numbers Can Touch set to: ");
  Serial.println(boardConfig.sameNumbersCanTouch ? "true" : "false");
  server.send(200, "text/plain", "sameNumbersCanTouch updated");
}

// Update Config: Same Resource Can Touch
void handleUpdateSameResourceCanTouch()
{
  String value = server.arg("value");
  boardConfig.sameResourceCanTouch = (value == "1");
  Serial.print("Same Resource Can Touch set to: ");
  Serial.println(boardConfig.sameResourceCanTouch ? "true" : "false");
  server.send(200, "text/plain", "sameResourceCanTouch updated");
}

void handleUpdateManualDice()
{
  String value = server.arg("value");
  manualDice = (value == "1");
  Serial.print("Manual Dice set to: ");
  Serial.println(manualDice ? "true" : "false");
  server.send(200, "text/plain", "manualDice updated");
}

// Set game as Classic
void handleSetClassic()
{
  Serial.println("[/setclassic] Request received. Setting game as classic");

  // Only reinitialize led strip if it was Extension before
  if (boardConfig.isExtension)
  {
    boardConfig.isExtension = false;
    ledController.restart(LED_COUNT_CLASSIC);
  }

  createBoardTask();

  // Generate the json data to send to the webpage
  String jsonResponse = generateJSON();

  // Send the JSON response over your web server
  server.send(200, "application/json", jsonResponse);

  // debug
  Serial.println(jsonResponse);
}

void handleSetExtension()
{
  Serial.println("[/setextension] Request received. Setting game as Extension");

  // Only reinitialize led strip if the current count is not already 19
  if (!boardConfig.isExtension)
  {
    boardConfig.isExtension = true;
    ledController.restart(LED_COUNT_EXTENSION);
  }

  createBoardTask();

  // Generate the json data to send to the webpage
  String jsonResponse = generateJSON();

  // Send the JSON response over your web server
  server.send(200, "application/json", jsonResponse);

  // debug
  Serial.println(jsonResponse);
}

// GET current board state
void handleGetBoard()
{
  Serial.println("[/getboard] Request received. Returning current board state.");
  // If board hasn't been generated yet, generate one.
  if (gameLoaded)
  {
    int totalHexes = boardConfig.isExtension ? LED_COUNT_EXTENSION : LED_COUNT_CLASSIC;

    // Generate the json data to send to the webpage
    String jsonResponse = generateJSON();

    // Send the JSON response over your web server
    server.send(200, "application/json", jsonResponse);

    // debug
    Serial.println(jsonResponse);
  }
}

// GET current number state
void handleGetNumber()
{
  Serial.println("[/getnumber] Request received. Returning current selected number.");
  // Send the JSON response over your web server
  server.send(200, "application/json", String(selectedNumber));
}

// Start Game: generate a new board and mark game as started.
void handleStartGame()
{
  Serial.println("[/startgame] Request received. Starting game.");
  gameStarted = true;

  ledController.stopAnimation();
  ledController.startAnimation(START_GAME_ANIMATION, nullptr, 0, 250);

  // Generate the json data to send to the webpage
  String jsonResponse = generateJSON();

  // Save the game state to flash.
  saveGameState();

  // Send the JSON response over your web server
  server.send(200, "application/json", jsonResponse);

  // debug
  Serial.println(jsonResponse);
}

// End Game: mark game as ended and return a new board state.
void handleEndGame()
{
  Serial.println("[/endgame] Request received. Ending game.");
  gameStarted = false;
  selectedNumber = 0;

  // Restart Waiting animation
  ledController.startAnimation(WAITING_ANIMATION, nullptr, 0, 50);

  // Delete the saved game state.
  deleteGameState();

  // Generate the json data to send to the webpage
  String jsonResponse = generateJSON();

  // Send the JSON response over your web server
  server.send(200, "application/json", jsonResponse);

  // debug
  Serial.println(jsonResponse);
}

// Web server handle function that updates the LEDs based on the selected number.
void handleSelectNumber()
{
  // Get the number sent from the client
  String value = server.arg("value");
  selectedNumber = value.toInt();
  Serial.print("[/selectNumber] Number selected: ");
  Serial.println(selectedNumber);

  turnOnNumber();

  saveGameState();

  // Respond to the client.
  server.send(200, "text/plain", value);
}

void turnOnNumber()
{
  // Determine how many tiles to check (19 for classic, 30 for Extension)
  int tileCount = boardConfig.isExtension ? LED_COUNT_EXTENSION : LED_COUNT_CLASSIC;
  ledController.stopAnimation();

  // turn off all leds
  ledController.turnOffAllLeds();

  if (selectedNumber == 7)
  {
    uint8_t requiredTiles = boardConfig.isExtension ? 2 : 1;
    uint8_t foundCount = 0;

    // Allocate the array on the heap.
    uint16_t *robberTiles = new uint16_t[requiredTiles];

    for (int tile = 0; tile < tileCount && foundCount < requiredTiles; tile++)
    {
      // Assuming the desert tile has number 0.
      if (board.numbers[tile] == 0)
      {
        Serial.print("found at tile: ");
        Serial.println(tile);
        robberTiles[foundCount++] = tile;
      }
    }
    // Pass the dynamically allocated array to the animation task.
    ledController.startAnimation(ROBBER_ANIMATION, robberTiles, requiredTiles, 500);
  }
  else
  {
    // Loop through each tile on the board.
    for (int tile = 0; tile < tileCount; tile++)
    {
      // If the board's number on this tile matches the selected number, light the LED;
      // otherwise, turn it off.
      if (board.numbers[tile] == selectedNumber)
      {
        ledController.turnTileOn(tile, ledController.Color(255, 255, 255));
      }
      else
      {
        // Turn off the LED.
        ledController.turnTileOn(tile, 0);
      }
    }

    // Update the LED strip to show the changes.
    ledController.update();
  }
}

void handleRollDice()
{
  // Roll two dice (each die: 1 to 6)
  int die1 = random(1, 7); // random(1, 7) generates numbers from 1 to 6
  int die2 = random(1, 7);
  selectedNumber = die1 + die2;

  // Run the LED animation from the library.
  ledController.rollDiceAnimation();

  // turn off all leds due to animation ending with all on
  ledController.turnOffAllLeds();

  // Convert the total sum into a string
  String result = String(selectedNumber);

  // After the animation, update the board using your turnOnNumber() function.
  turnOnNumber();

  saveGameState();

  // Respond to the client with the dice total
  server.send(200, "text/plain", result);
}

// --------------------------------------------------------------
//                  SETUP & LOOP
// --------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  delay(2000);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Attempt to load the saved game state.
  loadGameState();

  // Initialize board default values
  // If no game state was loaded, then set default board config.
  if (board.resources.size() == 0)
  {
    Serial.print("No settings in flash! Load Default");
    boardConfig.isExtension = DEFAULT_IS_EXTENSION;
    boardConfig.eightSixCanTouch = DEFAULT_EIGHT_SIX_CANTOUCH;
    boardConfig.twoTwelveCanTouch = DEFAULT_TWO_TWELVE_CANTOUCH;
    boardConfig.sameNumbersCanTouch = DEFAULT_SAMENUMBERS_CANTOUCH;
    boardConfig.sameResourceCanTouch = DEFAULT_SAMERESOURCE_CANTOUCH;
    manualDice = DEFAULT_MANUAL_DICE;
    gameStarted = false;
    selectedNumber = 0;
  }

  // Connect to WiFi
  connectWifi(WIFI_SSID, WIFI_PASS);

  // Open and read the HTML file once
  readHtml(htmlPage, server);

  // Seed the random number generator so we get fresh random sequences
  randomSeed(micros());

  // Initialize LED strip
  uint16_t ledCount = boardConfig.isExtension ? LED_COUNT_EXTENSION : LED_COUNT_CLASSIC;
  ledController.begin(ledCount);

  if (board.resources.size() == 0)
  {
    ledController.startAnimation(WAITING_ANIMATION, nullptr, 0, 50);
  }
  else
  {
    turnOnNumber();
  }
  // Set up server routes
  server.on("/", HTTP_GET, handleRoot);
  // Endpoints for updating settings:
  server.on("/eightSixCanTouch", HTTP_GET, handleUpdateEightSixCanTouch);
  server.on("/twoTwelveCanTouch", HTTP_GET, handleUpdateTwoTwelveCanTouch);
  server.on("/sameNumbersCanTouch", HTTP_GET, handleUpdateSameNumbersCanTouch);
  server.on("/sameResourceCanTouch", HTTP_GET, handleUpdateSameResourceCanTouch);
  server.on("/manualDice", HTTP_GET, handleUpdateManualDice);

  // Set up server routes
  server.on("/setclassic", HTTP_GET, handleSetClassic);
  server.on("/setextension", HTTP_GET, handleSetExtension);
  server.on("/getboard", HTTP_GET, handleGetBoard);
  server.on("/getnumber", HTTP_GET, handleGetNumber);
  server.on("/startgame", HTTP_GET, handleStartGame);
  server.on("/endgame", HTTP_GET, handleEndGame);
  server.on("/selectNumber", HTTP_GET, handleSelectNumber);
  server.on("/rollDice", HTTP_GET, handleRollDice);

  // Only generate a new board if none was loaded.
  if (board.resources.size() == 0)
  {
    Serial.print("No board loaded, generate board!");
    xTaskCreatePinnedToCore(
        boardGenerationTask,     // Task function.
        "BoardGenTask",          // Name of task.
        BOARD_GEN_STACK_SIZE,    // Stack size in words.
        NULL,                    // Task input parameter.
        BOARD_GEN_TASK_PRIORITY, // Task priority.
        NULL,                    // Task handle.
        BOARD_GEN_TASK_CORE      // Core where the task should run.
    );
  }
  else
  {
    Serial.println("Using saved board state.");
  }

  // Start server
  server.begin();
  Serial.println("HTTP server started.");

  gameLoaded = true;
}

void loop()
{
  server.handleClient();
}