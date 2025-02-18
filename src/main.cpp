// External
#include <Arduino.h>
#include <WebServer.h>
#include <FS.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

// Internal
#include "BoardGenerator.h"
#include "password.h"
#include "WebPage.h"

// ------------- WIFI Credentials -------------
#ifndef PASSWORD_H
#define PASSWORD_H
const char *WIFI_SSID = "PlaceholderSSID";
const char *WIFI_PASS = "PlaceholderPassword";
#endif

// ------------- PINS & LED STRIP -------------
#define LED_STRIP_PIN 4
#define LED_COUNT_CLASSIC 19
#define LED_COUNT_EXPANSION 30
Adafruit_NeoPixel *strip = nullptr; // Declare a pointer for our LED strip

//---------------SETTINGS----------------------
bool eight_six_canTouch = true;
bool two_twelve_canTouch = true;
bool sameNumbers_canTouch = true;
bool sameResource_canTouch = true;

bool is_expansion = false;
bool gameStarted = false;

// ------------- WEB SERVER -------------------
WebServer server(80);
// store HTML content
String htmlPage;

// ------------- CATAN DATA -------------------
Board board;
BoardConfig boardConfig;

// Zig-zag mapping from hexagonal tile index -> LED index on the WS2812B strip.
static const int tileToLedIndexClassic[19] = {
    // Row 1 (left to right): 0,1,2
    0, 1, 2,
    // Row 2 (right to left): 6,5,4,3
    6, 5, 4, 3,
    // Row 3 (left to right): 7,8,9,10,11
    7, 8, 9, 10, 11,
    // Row 4 (right to left): 15,14,13,12
    15, 14, 13, 12,
    // Row 5 (left to right): 16,17,18
    16, 17, 18};

// Zig-zag mapping from hexagonal tile index -> LED index on the WS2812B strip.
static const int tileToLedIndexExpansion[30] = {
    // Row 1 (left to right): 0,1,2,3
    0, 1, 2, 3,
    // Row 2 (right to left): 8,7,6,5,4
    8, 7, 6, 5, 4,
    // Row 3 (left to right): 9,10,11,12,13,14
    9, 10, 11, 12, 13, 14,
    // Row 4 (right to left): 20,19,18,17,16,15
    20, 19, 18, 17, 16, 15,
    // Row 5 (left to right): 21,22,23,24,25
    21, 22, 23, 24, 25,
    // Row 6 (right to left): 29,28,27,26
    29, 28, 27, 26};

// --------------------------------------------------------------
//                  LED STRIP
// --------------------------------------------------------------

void restartLedStrip(int numLed)
{
  // Clean up the old LED strip object
  if (strip != nullptr)
  {
    delete strip;
    strip = nullptr;
  }

  // Create a new LED strip object
  Serial.print("RESTART LEDS: ");
  Serial.println(numLed);
  strip = new Adafruit_NeoPixel(numLed, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);
  strip->begin();
  strip->setBrightness(50);
}

//---------------------------------------------------------------
//                 UTILS
//---------------------------------------------------------------
String generateJSON()
{
  // Create a JSON document.
  // Allocate the JSON document
  JsonDocument doc;
  int ledNumber;

  if (!boardConfig.isExpansion)
  {
    ledNumber = LED_COUNT_CLASSIC;
  }
  else
  {
    ledNumber = LED_COUNT_EXPANSION;
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

  // Include the game mode flag (classic mode => expansion is false)
  doc["expansion"] = boardConfig.isExpansion;
  doc["gameStarted"] = gameStarted;

  // Serialize the JSON document to a string.
  String jsonResponse;
  serializeJson(doc, jsonResponse);

  return jsonResponse;
}

// --------------------------------------------------------------
//                  SERVER HANDLERS
// --------------------------------------------------------------

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

// Set game as Classic
void handleSetClassic()
{
  Serial.println("[/setclassic] Request received. Setting game as classic");

  // Only reinitialize led strip if it was expansion before
  if (boardConfig.isExpansion)
  {
    boardConfig.isExpansion = false;
    restartLedStrip(LED_COUNT_CLASSIC);
  }

  // Generate the board using the classic configuration
  board = generateBoard(boardConfig);

  // Generate the json data to send to the webpage
  String jsonResponse = generateJSON();

  // Send the JSON response over your web server
  server.send(200, "application/json", jsonResponse);

  // debug
  Serial.println(jsonResponse);
}

void handleSetExpansion()
{
  Serial.println("[/setexpansion] Request received. Setting game as expansion");

  // Only reinitialize led strip if the current count is not already 19
  if (!boardConfig.isExpansion)
  {
    boardConfig.isExpansion = true;
    restartLedStrip(LED_COUNT_EXPANSION);
  }

  // Generate the board using the expansion configuration.
  board = generateBoard(boardConfig);

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
  if (board.resources.size() == 0)
  {
    board = generateBoard(boardConfig);
  }
  int totalHexes = boardConfig.isExpansion ? 30 : 19;

  // Generate the json data to send to the webpage
  String jsonResponse = generateJSON();

  // Send the JSON response over your web server
  server.send(200, "application/json", jsonResponse);

  // debug
  Serial.println(jsonResponse);
}

// Start Game: generate a new board and mark game as started.
void handleStartGame()
{
  Serial.println("[/startgame] Request received. Starting game.");
  gameStarted = true;

  // Generate the json data to send to the webpage
  String jsonResponse = generateJSON();

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
  int selectedNumber = value.toInt();
  Serial.print("[/selectNumber] Number selected: ");
  Serial.println(selectedNumber);

  if (strip == nullptr)
  {
    Serial.println("Error: LED strip not initialized!");
    server.send(500, "text/plain", "LED strip not initialized");
    return;
  }

  // Determine how many tiles to check (19 for classic, 30 for expansion)
  int tileCount = boardConfig.isExpansion ? 30 : 19;

  // Loop through each tile on the board.
  for (int tile = 0; tile < tileCount; tile++)
  {
    // Pick the correct LED index for this tile
    int ledIndex = boardConfig.isExpansion ? tileToLedIndexExpansion[tile] : tileToLedIndexClassic[tile];

    Serial.print("Tile ");
    Serial.print(tile);
    Serial.print(" -> LED ");
    Serial.print(ledIndex);
    Serial.print(" with number ");
    Serial.println(board.numbers[tile]);

    // If the board's number on this tile matches the selected number, light the LED;
    // otherwise, turn it off.
    if (board.numbers[tile] == selectedNumber)
    {
      // For example, set matching LED to red.
      Serial.println("FOUND NUMBER ");
      Serial.println(sizeof(strip));
      strip->setPixelColor(ledIndex, strip->Color(255, 255, 255));
    }
    else
    {
      // Turn off the LED.
      strip->setPixelColor(ledIndex, 0);
    }
  }

  // Update the LED strip to show the changes.
  strip->show();

  // Respond to the client.
  server.send(200, "text/plain", "Number " + value + " received and LEDs updated");
}

// --------------------------------------------------------------
//                  SETUP & LOOP
// --------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  delay(500);

  // Connect to WiFi
  connectWifi(WIFI_SSID, WIFI_PASS);

  // Open and read the HTML file once
  readHtml(htmlPage, server);

  // Seed the random number generator so we get fresh random sequences
  randomSeed(micros());

  // Initialize board default values
  boardConfig.isExpansion = false;
  boardConfig.eightSixCanTouch = true;
  boardConfig.twoTwelveCanTouch = true;
  boardConfig.sameNumbersCanTouch = true;
  boardConfig.sameResourceCanTouch = true;

  // Initialize LED strip with currentNumLeds
  restartLedStrip(LED_COUNT_CLASSIC);

  // Set up server routes
  server.on("/", HTTP_GET, handleRoot);
  // Endpoints for updating settings:
  server.on("/eightSixCanTouch", HTTP_GET, handleUpdateEightSixCanTouch);
  server.on("/twoTwelveCanTouch", HTTP_GET, handleUpdateTwoTwelveCanTouch);
  server.on("/sameNumbersCanTouch", HTTP_GET, handleUpdateSameNumbersCanTouch);
  server.on("/sameResourceCanTouch", HTTP_GET, handleUpdateSameResourceCanTouch);

  // Set up server routes
  server.on("/setclassic", HTTP_GET, handleSetClassic);
  server.on("/setexpansion", HTTP_GET, handleSetExpansion);
  server.on("/getboard", HTTP_GET, handleGetBoard);
  server.on("/startgame", HTTP_GET, handleStartGame);
  server.on("/endgame", HTTP_GET, handleEndGame);
  server.on("/selectNumber", HTTP_GET, handleSelectNumber);

  // Start server
  server.begin();
  Serial.println("HTTP server started.");
}

void loop()
{
  server.handleClient();
}