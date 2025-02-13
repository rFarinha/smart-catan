// External
#include <Arduino.h>
#include <WebServer.h>
#include <FS.h>

// Internal
#include "BoardGenerator.h"
//#include "password.h"
#include "WebPage.h"
#include "WebHandlers.h"
#include "LedStripControl.h"

// ------------- WIFI Credentials -------------
#ifndef PASSWORD_H
#define PASSWORD_H
const char *WIFI_SSID = "PlaceholderSSID";
const char *WIFI_PASS = "PlaceholderPassword";
#endif

// ------------- PINS & LED STRIP -------------
#define LED_STRIP_PIN 4
LEDStripControl *ledControl = new LEDStripControl(19, LED_STRIP_PIN); // default to classic (19 LEDs)

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
ServerContext serverCtx;

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

  // Initialize board default values
  boardConfig.isExpansion = false;
  boardConfig.eightSixCanTouch = true;
  boardConfig.twoTwelveCanTouch = true;
  boardConfig.sameNumbersCanTouch = true;
  boardConfig.sameResourceCanTouch = true;
   
  serverCtx.boardConfig = &boardConfig;
  serverCtx.htmlPage = htmlPage;
  serverCtx.ledStripControl = ledControl;

  // Set server handlers and start server
  startServer(server, serverCtx);
}

void loop()
{
  server.handleClient();
}
