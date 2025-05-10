/**
 * Catan Board Generator for ESP32
 *
 * This project provides a web interface for generating and displaying
 * randomized Catan boards with configurable placement rules. It includes
 * LED visualization using WS2812B LED strips, configurable game settings,
 * and support for both classic and extension Catan board layouts.
 */

// External Libraries
#include <Arduino.h>
#include <WebServer.h>
#include <FS.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <DNSServer.h>

// Internal Project Headers
#include "BoardGenerator.h"
#include "WebPage.h"
#include "LedController.h"
#include "HomeAssistantTrigger.h"
#include "WiFiManager.h"  // Add the new WiFiManager header

// Configuration for background task handling
#define BOARD_GEN_STACK_SIZE 8192 // Stack size for board generation task
#define BOARD_GEN_TASK_PRIORITY 1 // Priority level for the task
#define BOARD_GEN_TASK_CORE 1     // Core to run the task on (ESP32 has 2 cores)

// Global State Variables
volatile bool boardReady = true; // Indicates if board generation is complete
bool gameLoaded = false;         // Indicates if a saved game was loaded

// Hardware Configuration
#define LED_STRIP_PIN 4        // GPIO pin connected to the WS2812B data line
#define LED_COUNT_CLASSIC 19   // Number of LEDs for classic board
#define LED_COUNT_EXTENSION 30 // Number of LEDs for extension board

// Initialize LED controller
LedController ledController(LED_STRIP_PIN, LED_COUNT_CLASSIC);

// Initialize WiFi manager
WiFiManager wifiManager;

// Currently selected dice number (2-12, 0 means none selected)
int selectedNumber;

// Default Configuration Settings
#define DEFAULT_EIGHT_SIX_CANTOUCH true    // Can 6 & 8 tokens be adjacent?
#define DEFAULT_TWO_TWELVE_CANTOUCH true   // Can 2 & 12 tokens be adjacent?
#define DEFAULT_SAMENUMBERS_CANTOUCH true  // Can identical token numbers be adjacent?
#define DEFAULT_SAMERESOURCE_CANTOUCH true // Can identical resources be adjacent?
#define DEFAULT_MANUAL_DICE false          // Allow manual dice number selection?
#define DEFAULT_IS_EXTENSION false         // Start in extension mode?

// Game state variables
bool manualDice;  // Manual dice selection enabled?
bool gameStarted; // Is game currently active?

// Web Server Setup
WebServer server(80); // HTTP server on port 80
String htmlPage;      // Stores the HTML content

// Catan Game Data
Board board;             // Current board layout
BoardConfig boardConfig; // Board configuration settings

// DNS Server
DNSServer dnsServer;
const byte DNS_PORT = 53; // Standard DNS port

//---------------------------------------------------------------
//                 UTILITY FUNCTIONS
//---------------------------------------------------------------

/**
 * Creates a JSON representation of the current game state
 *
 * @return String containing JSON data with board configuration,
 *         resource placement, number tokens, and game settings
 */
String generateJSON()
{
  // Create a JSON document
  JsonDocument doc;
  int ledNumber;

  // Determine number of hexes/LEDs based on board mode
  if (!boardConfig.isExtension)
  {
    ledNumber = LED_COUNT_CLASSIC;
  }
  else
  {
    ledNumber = LED_COUNT_EXTENSION;
  }

  // Add the resources array
  JsonArray resources = doc["resources"].to<JsonArray>();
  for (int i = 0; i < ledNumber; i++)
  {
    resources.add(board.resources[i]);
  }

  // Add the numbers array
  JsonArray numbers = doc["numbers"].to<JsonArray>();
  for (int i = 0; i < ledNumber; i++)
  {
    numbers.add(board.numbers[i]);
  }

  // Include the game mode and state flags
  doc["extension"] = boardConfig.isExtension;
  doc["gameStarted"] = gameStarted;

  // Include the game settings
  doc["eightSixCanTouch"] = boardConfig.eightSixCanTouch;
  doc["twoTwelveCanTouch"] = boardConfig.twoTwelveCanTouch;
  doc["sameNumbersCanTouch"] = boardConfig.sameNumbersCanTouch;
  doc["sameResourceCanTouch"] = boardConfig.sameResourceCanTouch;
  doc["manualDice"] = manualDice;

  // Include currently selected number
  doc["selectedNumber"] = selectedNumber;

  // Serialize the JSON document to a string
  String jsonResponse;
  serializeJson(doc, jsonResponse);

  return jsonResponse;
}

/**
 * Saves the current game state to the ESP32's flash memory
 * This allows persisting the game through power cycles
 */
void saveGameState()
{
  // Generate the json data
  String jsonString = generateJSON();

  // Write to SPIFFS (SPI Flash File System)
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

/**
 * Deletes any saved game state from flash memory
 */
void deleteGameState()
{
  if (SPIFFS.exists("/gamestate.json"))
  {
    SPIFFS.remove("/gamestate.json");
    Serial.println("Game state deleted from flash.");
  }
}

/**
 * Loads a previously saved game state from flash memory
 */
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

    // Parse the JSON document
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error)
    {
      Serial.print("Failed to parse game state: ");
      Serial.println(error.f_str());
      return;
    }

    // Load board configuration
    boardConfig.isExtension = doc["extension"];
    boardConfig.eightSixCanTouch = doc["eightSixCanTouch"];
    boardConfig.twoTwelveCanTouch = doc["twoTwelveCanTouch"];
    boardConfig.sameNumbersCanTouch = doc["sameNumbersCanTouch"];
    boardConfig.sameResourceCanTouch = doc["sameResourceCanTouch"];
    manualDice = doc["manualDice"];

    gameStarted = doc["gameStarted"];
    selectedNumber = doc["selectedNumber"];

    // Load board resources and numbers
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
    Serial.println("No saved game state found in flash!");
  }
}

// --------------------------------------------------------------
//                  SERVER HANDLER FUNCTIONS
// --------------------------------------------------------------

/**
 * FreeRTOS task that handles board generation in a separate thread
 * This prevents blocking the main loop during board calculation
 */
void boardGenerationTask(void *pvParameters)
{
  Serial.println("Board generation task started.");

  // Use the current boardConfig to generate a board
  board = generateBoard(boardConfig);

  // Signal that the board is ready
  Serial.println("Board generation complete.");
  boardReady = true;

  // Delete the task when finished
  vTaskDelete(NULL);
}

/**
 * Creates a new task for board generation
 */
void createBoardTask()
{
  // Reset flag and create the board generation task
  if (boardReady)
  {
    boardReady = false;
    xTaskCreatePinnedToCore(
        boardGenerationTask, // Task function
        "BoardGenTask",      // Task name
        8192,                // Stack size (bytes)
        NULL,                // Parameters
        1,                   // Priority
        NULL,                // Task handle
        1                    // Run on core 1
    );

    // Wait until the board generation task is done
    while (!boardReady)
    {
      delay(10); // Yield to other tasks
    }
  }
}

/**
 * Web server handler for the root path
 * Serves the main HTML page
 */
void handleRoot() {
  // Always serve the main Catan board interface
  server.send(200, "text/html", htmlPage);
}

/**
 * Web server handler to update the "6 & 8 Can Touch" setting
 */
void handleUpdateEightSixCanTouch()
{
  String value = server.arg("value"); // "1" for true or "0" for false
  boardConfig.eightSixCanTouch = (value == "1");
  Serial.print("8 & 6 Can Touch set to: ");
  Serial.println(boardConfig.eightSixCanTouch ? "true" : "false");
  server.send(200, "text/plain", "eightSixCanTouch updated");
}

/**
 * Web server handler to update the "2 & 12 Can Touch" setting
 */
void handleUpdateTwoTwelveCanTouch()
{
  String value = server.arg("value");
  boardConfig.twoTwelveCanTouch = (value == "1");
  Serial.print("2 & 12 Can Touch set to: ");
  Serial.println(boardConfig.twoTwelveCanTouch ? "true" : "false");
  server.send(200, "text/plain", "twoTwelveCanTouch updated");
}

/**
 * Web server handler to update the "Same Numbers Can Touch" setting
 */
void handleUpdateSameNumbersCanTouch()
{
  String value = server.arg("value");
  boardConfig.sameNumbersCanTouch = (value == "1");
  Serial.print("Same Numbers Can Touch set to: ");
  Serial.println(boardConfig.sameNumbersCanTouch ? "true" : "false");
  server.send(200, "text/plain", "sameNumbersCanTouch updated");
}

/**
 * Web server handler to update the "Same Resource Can Touch" setting
 */
void handleUpdateSameResourceCanTouch()
{
  String value = server.arg("value");
  boardConfig.sameResourceCanTouch = (value == "1");
  Serial.print("Same Resource Can Touch set to: ");
  Serial.println(boardConfig.sameResourceCanTouch ? "true" : "false");
  server.send(200, "text/plain", "sameResourceCanTouch updated");
}

/**
 * Web server handler to update the "Manual Dice" setting
 */
void handleUpdateManualDice()
{
  String value = server.arg("value");
  manualDice = (value == "1");
  Serial.print("Manual Dice set to: ");
  Serial.println(manualDice ? "true" : "false");
  server.send(200, "text/plain", "manualDice updated");
}

/**
 * Web server handler to set or shuffle classic board mode
 */
void handleSetClassic()
{
  Serial.println("[/setclassic] Request received. Setting game as classic");

  // Only reinitialize LED strip if it was in Extension mode before
  if (boardConfig.isExtension)
  {
    boardConfig.isExtension = false;
    ledController.restart(LED_COUNT_CLASSIC);
  }

  // Generate a new board
  createBoardTask();

  // Send the JSON response
  String jsonResponse = generateJSON();
  server.send(200, "application/json", jsonResponse);

  // Debug output
  Serial.println(jsonResponse);
}

/**
 * Web server handler to set or shuffle extension board mode
 */
void handleSetExtension()
{
  Serial.println("[/setextension] Request received. Setting game as Extension");

  // Only reinitialize LED strip if it was in Classic mode before
  if (!boardConfig.isExtension)
  {
    boardConfig.isExtension = true;
    ledController.restart(LED_COUNT_EXTENSION);
  }

  // Generate a new board
  createBoardTask();

  // Send the JSON response
  String jsonResponse = generateJSON();
  server.send(200, "application/json", jsonResponse);

  // Debug output
  Serial.println(jsonResponse);
}

/**
 * Web server handler to get current board state
 */
void handleGetBoard()
{
  Serial.println("[/getboard] Request received. Returning current board state.");
  // Only respond if the board has been initialized
  if (gameLoaded)
  {
    int totalHexes = boardConfig.isExtension ? LED_COUNT_EXTENSION : LED_COUNT_CLASSIC;

    // Generate the json data
    String jsonResponse = generateJSON();

    // Send the JSON response
    server.send(200, "application/json", jsonResponse);

    // Debug output
    Serial.println(jsonResponse);
  }
}

/**
 * Web server handler to get currently selected number
 */
void handleGetNumber()
{
  Serial.println("[/getnumber] Request received. Returning current selected number.");
  server.send(200, "application/json", String(selectedNumber));
}

/**
 * Web server handler to start a new game
 * Locks the board configuration and begins gameplay
 */
void handleStartGame()
{
  Serial.println("[/startgame] Request received. Starting game.");
  gameStarted = true;

  // Run start game animation on LEDs
  ledController.stopAnimation();
  ledController.startAnimation(START_GAME_ANIMATION, nullptr, 0, 250);

  // Generate JSON response
  String jsonResponse = generateJSON();

  // Save game state to flash for persistence
  saveGameState();

  // Send the response
  server.send(200, "application/json", jsonResponse);

  // Debug output
  Serial.println(jsonResponse);
}

/**
 * Web server handler to end the current game
 * Unlocks board configuration
 */
void handleEndGame()
{
  Serial.println("[/endgame] Request received. Ending game.");
  gameStarted = false;
  selectedNumber = 0;

  // Restart the waiting animation
  ledController.startAnimation(WAITING_ANIMATION, nullptr, 0, 50);

  // Delete the saved game state
  deleteGameState();

  // Generate JSON response
  String jsonResponse = generateJSON();

  // Send the response
  server.send(200, "application/json", jsonResponse);

  // Debug output
  Serial.println(jsonResponse);
}

/**
 * Web server handler to get WiFi and Home Assistant connection status
 * Returns JSON with connection details
 */
void handleConnectionStatus() {
  JsonDocument doc;
  
  // WiFi status
  doc["wifiConnected"] = WiFi.status() == WL_CONNECTED;
  
  if (WiFi.status() == WL_CONNECTED) {
    // Connected to WiFi
    doc["ip"] = WiFi.localIP().toString();
    doc["ssid"] = WiFi.SSID();
    doc["apMode"] = false;
  } else if (wifiManager.isInApMode()) {
    // In AP mode
    doc["apMode"] = true;
    doc["ip"] = WiFi.softAPIP().toString();
    doc["apName"] = AP_NAME;
  } else {
    // Not connected to WiFi and not in AP mode
    doc["apMode"] = false;
  }
  
  // Home Assistant status
  #ifdef ENABLE_HOME_ASSISTANT
  doc["haEnabled"] = wifiManager.isHaEnabled();
  doc["haIp"] = wifiManager.getHaIp();
  doc["haPort"] = wifiManager.getHaPort();
  // Don't send the full token for security, just indicate if it exists
  doc["haTokenSet"] = wifiManager.getHaAccessToken().length() > 0;
  #else
  doc["haEnabled"] = false;
  #endif
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

/**
 * Web server handler to scan for available WiFi networks
 * Returns JSON array with network details
 */
void handleScanNetworks() {
  Serial.println("Scanning for networks...");
  
  int n = WiFi.scanNetworks();
  JsonDocument doc;
  JsonArray networks = doc.createNestedArray("networks");
  
  for (int i = 0; i < n; ++i) {
    JsonObject network = networks.createNestedObject();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["secure"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
  
  // Clean up scan results
  WiFi.scanDelete();
}

/**
 * Web server handler to save WiFi configuration
 * Accepts form data with SSID and password
 */
void handleSaveWifi() {
  if (!server.hasArg("ssid")) {
    server.send(400, "text/plain", "Missing SSID parameter");
    return;
  }
  
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  
  // Save to WiFi manager
  bool success = wifiManager.saveCredentials(ssid, password);
  
  if (success) {
    server.send(200, "text/plain", "WiFi configuration saved! Restarting...");
    
    // Give the browser time to receive the response
    delay(1000);
    
    // Restart the ESP32 to apply new settings
    ESP.restart();
  } else {
    server.send(500, "text/plain", "Failed to save WiFi configuration");
  }
}

/**
 * Web server handler to save Home Assistant configuration
 * Accepts form data with HA server details
 */
void handleSaveHA() {
  if (!server.hasArg("ha_ip")) {
    server.send(400, "text/plain", "Missing Home Assistant IP parameter");
    return;
  }
  
  String haIp = server.arg("ha_ip");
  String haPortStr = server.arg("ha_port");
  String haToken = server.arg("ha_token");
  
  // Parse port with validation
  uint16_t haPort = haPortStr.length() > 0 ? haPortStr.toInt() : 8123;
  
  // Save to WiFi manager
  bool success = wifiManager.saveHAConfig(haIp, haPort, haToken);
  
  if (success) {
    server.send(200, "text/plain", "Home Assistant configuration saved!");
    
    // Initialize Home Assistant with the new settings
    #ifdef ENABLE_HOME_ASSISTANT
    initHomeAssistant(
      haIp.c_str(),
      haPort,
      haToken.c_str(),
      "/api/services/script/turn_on"
    );
    #endif
  } else {
    server.send(500, "text/plain", "Failed to save Home Assistant configuration");
  }
}

void handleNotFound() {
    // If in AP mode, capture all requests and redirect to the root page
    if (wifiManager.isInApMode()) {
        // Check if the request is for a web page (not for assets like CSS/JS)
        if (server.hostHeader() != WiFi.softAPIP().toString() 
            && server.uri().indexOf(".") == -1) {
            Serial.print("Captive portal redirect: ");
            Serial.println(server.uri());
            
            // Redirect to the main page
            String redirectURL = "http://" + WiFi.softAPIP().toString();
            server.sendHeader("Location", redirectURL, true);
            server.send(302, "text/plain", "");
            return;
        }
    }
    
    // For other 404 cases, return a standard 404 message
    server.send(404, "text/plain", "Not Found");
}

/**
 * Updates the LED display based on the currently selected number
 * For normal numbers (2-6, 8-12): Lights up hexes with that number
 * For 7 (robber): Triggers the robber animation
 */
void turnOnNumber()
{
  // Determine number of tiles based on board mode
  int tileCount = boardConfig.isExtension ? LED_COUNT_EXTENSION : LED_COUNT_CLASSIC;
  ledController.stopAnimation();

  // Trigger Home Assistant with the selected number if enabled
  if (wifiManager.isHaEnabled() && wifiManager.isConnected()) {
    // Initialize Home Assistant with configuration from WiFiManager
    #ifdef ENABLE_HOME_ASSISTANT
    initHomeAssistant(
      wifiManager.getHaIp().c_str(),
      wifiManager.getHaPort(),
      wifiManager.getHaAccessToken().c_str(),
      "/api/services/script/turn_on"
    );
    triggerHomeAssistantScript(selectedNumber);
    #endif
  }

  // Turn off all LEDs before applying new state
  ledController.turnOffAllLeds();

  if (selectedNumber == 7)
  {
    // Special handling for the robber (7)
    uint8_t requiredTiles = boardConfig.isExtension ? 2 : 1;
    uint8_t foundCount = 0;

    // Allocate array for desert tiles (robber locations)
    uint16_t *robberTiles = new uint16_t[requiredTiles];

    // Find desert tiles (number token 0)
    for (int tile = 0; tile < tileCount && foundCount < requiredTiles; tile++)
    {
      if (board.numbers[tile] == 0)
      {
        Serial.print("Desert found at tile: ");
        Serial.println(tile);
        robberTiles[foundCount++] = tile;
      }
    }

    // Start robber animation from the desert tiles
    ledController.startAnimation(ROBBER_ANIMATION, robberTiles, foundCount, 500);
  }
  else
  {
    // For regular numbers, highlight matching hexes
    for (int tile = 0; tile < tileCount; tile++)
    {
      if (board.numbers[tile] == selectedNumber)
      {
        ledController.turnTileOn(tile, ledController.Color(255, 255, 255));
      }
      else
      {
        // Turn off LEDs for non-matching tiles
        ledController.turnTileOn(tile, 0);
      }
    }

    // Update the LED strip
    ledController.update();
  }
}

/**
 * Web server handler to select a number during gameplay
 * Updates the selected number and highlights corresponding tiles
 */
void handleSelectNumber()
{
  // Get the number sent from the client
  String value = server.arg("value");
  selectedNumber = value.toInt();
  Serial.print("[/selectNumber] Number selected: ");
  Serial.println(selectedNumber);

  // Update LEDs to reflect the selected number
  turnOnNumber();

  // Save the current game state
  saveGameState();

  // Respond to the client
  server.send(200, "text/plain", value);
}

/**
 * Web server handler to simulate rolling dice
 * Generates random dice values and updates the display
 */
void handleRollDice()
{
  // Roll two dice (each die: 1 to 6)
  int die1 = random(1, 7);
  int die2 = random(1, 7);
  selectedNumber = die1 + die2;

  // Run a dice roll animation on the LEDs
  ledController.rollDiceAnimation();

  // Turn off all LEDs (animation ends with all on)
  ledController.turnOffAllLeds();

  // Convert the total to a string
  String result = String(selectedNumber);

  // Update the board display
  turnOnNumber();

  // Save the current game state
  saveGameState();

  // Respond to the client with the dice result
  server.send(200, "text/plain", result);
}

// --------------------------------------------------------------
//                  SETUP & LOOP
// --------------------------------------------------------------

/**
 * Arduino setup function - runs once at startup
 * Initializes hardware, loads configuration, and starts the web server
 */
void setup()
{
  // Initialize serial communication
  Serial.begin(115200);
  delay(2000); // Short delay for serial port to initialize

  // Initialize the SPI Flash File System
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Seed the random number generator
  randomSeed(micros());

  // Initialize LED strip for classic board initially
  ledController.begin(LED_COUNT_CLASSIC);

  // Initialize the WiFi manager
  wifiManager.begin();

  // Start DNS server for captive portal if in AP mode
  if (wifiManager.isInApMode()) {
      // Initialize the DNS server to redirect all domains to our IP
      IPAddress apIP = WiFi.softAPIP();
      dnsServer.start(DNS_PORT, "*", apIP);
      Serial.println("DNS server started for captive portal");
  } else {
      // Only attempt mDNS when in station mode
      if (!MDNS.begin("smartcatan")) {
          Serial.println("Error setting up MDNS responder!");
      } else {
          Serial.println("mDNS responder started");
      }
    }
  
  // Always initialize the web server with the main interface, regardless of connection state
  readHtml(htmlPage, server);
  server.begin();
  
  // Initialize LED strip based on board mode
  uint16_t ledCount = boardConfig.isExtension ? LED_COUNT_EXTENSION : LED_COUNT_CLASSIC;
  ledController.restart(ledCount);
  
  // Show the waiting animation while we determine the board state
  ledController.startAnimation(WAITING_ANIMATION, nullptr, 0, 50);

  // Continue with normal Catan board setup since we're connected to WiFi
  // Attempt to load saved game state
  loadGameState();

  // If no game state was loaded, set default configuration
  if (board.resources.size() == 0)
  {
    Serial.print("No settings in flash! Loading defaults");
    boardConfig.isExtension = DEFAULT_IS_EXTENSION;
    boardConfig.eightSixCanTouch = DEFAULT_EIGHT_SIX_CANTOUCH;
    boardConfig.twoTwelveCanTouch = DEFAULT_TWO_TWELVE_CANTOUCH;
    boardConfig.sameNumbersCanTouch = DEFAULT_SAMENUMBERS_CANTOUCH;
    boardConfig.sameResourceCanTouch = DEFAULT_SAMERESOURCE_CANTOUCH;
    manualDice = DEFAULT_MANUAL_DICE;
    gameStarted = false;
    selectedNumber = 0;
  }

  // Load HTML content from SPIFFS
  readHtml(htmlPage, server);

  // Initialize LED strip based on board mode
  ledCount = boardConfig.isExtension ? LED_COUNT_EXTENSION : LED_COUNT_CLASSIC;
  ledController.restart(ledCount);

  // Start appropriate LED animation
  if (board.resources.size() == 0)
  {
    // If no board loaded, show waiting animation
    ledController.startAnimation(WAITING_ANIMATION, nullptr, 0, 50);
  }
  else
  {
    // If board loaded, show current selected number
    turnOnNumber();
  }

  // Initialize Home Assistant if enabled and configured through WiFiManager
  if (wifiManager.isHaEnabled()) {
    #ifdef ENABLE_HOME_ASSISTANT
    initHomeAssistant(
      wifiManager.getHaIp().c_str(),
      wifiManager.getHaPort(),
      wifiManager.getHaAccessToken().c_str(),
      "/api/services/script/turn_on"
    );
    Serial.println("Home Assistant integration enabled with settings from config portal");
    #endif
  }

  // Set up server routes
  server.on("/", HTTP_GET, handleRoot);

  // Settings endpoints
  server.on("/eightSixCanTouch", HTTP_GET, handleUpdateEightSixCanTouch);
  server.on("/twoTwelveCanTouch", HTTP_GET, handleUpdateTwoTwelveCanTouch);
  server.on("/sameNumbersCanTouch", HTTP_GET, handleUpdateSameNumbersCanTouch);
  server.on("/sameResourceCanTouch", HTTP_GET, handleUpdateSameResourceCanTouch);
  server.on("/manualDice", HTTP_GET, handleUpdateManualDice);

  // Game control endpoints
  server.on("/setclassic", HTTP_GET, handleSetClassic);
  server.on("/setextension", HTTP_GET, handleSetExtension);
  server.on("/getboard", HTTP_GET, handleGetBoard);
  server.on("/getnumber", HTTP_GET, handleGetNumber);
  server.on("/startgame", HTTP_GET, handleStartGame);
  server.on("/endgame", HTTP_GET, handleEndGame);
  server.on("/selectNumber", HTTP_GET, handleSelectNumber);
  server.on("/rollDice", HTTP_GET, handleRollDice);

  // Add new API endpoints for the integrated WiFi and HA management
  server.on("/connection-status", HTTP_GET, handleConnectionStatus);
  server.on("/scan-networks", HTTP_GET, handleScanNetworks);
  server.on("/save-wifi", HTTP_POST, handleSaveWifi);
  server.on("/save-ha", HTTP_POST, handleSaveHA);
  server.onNotFound(handleNotFound);

  // Generate a new board if none was loaded
  if (board.resources.size() == 0)
  {
    Serial.print("No board loaded, generating new board!");
    xTaskCreatePinnedToCore(
        boardGenerationTask,     // Task function
        "BoardGenTask",          // Task name
        BOARD_GEN_STACK_SIZE,    // Stack size
        NULL,                    // Parameters
        BOARD_GEN_TASK_PRIORITY, // Priority
        NULL,                    // Task handle
        BOARD_GEN_TASK_CORE      // Core to run on
    );
  }
  else
  {
    Serial.println("Using saved board state.");
  }

  // Start the web server
  server.begin();
  Serial.println("HTTP server started.");

  // Mark initialization as complete
  gameLoaded = true;
}

/**
 * Arduino loop function - runs repeatedly
 * Handles web server client requests
 */
void loop() {
    // Process DNS requests if in AP mode
    if (wifiManager.isInApMode()) {
        dnsServer.processNextRequest();
    }

  // Always handle web server requests
  server.handleClient();
}