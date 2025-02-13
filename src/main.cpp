// External
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <WebServer.h>
#include <FS.h>

// Internal
#include "BoardGenerator.h"
#include "password.h"
#include "WebPage.h"
#include "WebHandlers.h"

// ------------- WIFI Credentials -------------
#ifndef PASSWORD_H
#define PASSWORD_H
const char *WIFI_SSID = "PlaceholderSSID";
const char *WIFI_PASS = "PlaceholderPassword";
#endif

// ------------- PINS & LED STRIP -------------
#define LED_STRIP_PIN 4
int currentNumLeds = 19;            // default to classic (19 LEDs)
Adafruit_NeoPixel *strip = nullptr; // Declare a pointer for our LED strip

//---------------SETTINGS----------------------
bool eight_six_canTouch = true;
bool two_twelve_canTouch = true;
bool sameNumbers_canTouch = true;
bool sameResource_canTouch = true;

bool is_expansion = false;

// ------------- WEB SERVER -------------------
WebServer server(80);
// store HTML content
String htmlPage;

// ------------- CATAN DATA -------------------
Board board;
BoardConfig boardConfig;

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

  // Initialize LED strip with currentNumLeds
  strip = new Adafruit_NeoPixel(currentNumLeds, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

  // Initialize board default values
  boardConfig.isExpansion = false;
  boardConfig.eightSixCanTouch = true;
  boardConfig.twoTwelveCanTouch = true;
  boardConfig.sameNumbersCanTouch = true;
  boardConfig.sameResourceCanTouch = true;

  // Set server handlers and start server
  startServer(server, boardConfig, htmlPage, *strip, currentNumLeds);
}

void loop()
{
  server.handleClient();
}
