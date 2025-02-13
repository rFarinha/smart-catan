#ifndef WEBHANDLERS_H
#define WEBHANDLERS_H

#include <Arduino.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include "BoardGenerator.h"

void startServer(WebServer &server, BoardConfig &boardConfig, String htmlPage, Adafruit_NeoPixel &strip, int currentNumLeds);

// Update Config: 6 & 8 Can Touch
void handleUpdateEightSixCanTouch(WebServer &server, BoardConfig &boardConfig);

// Update Config: 2 & 12 Can Touch
void handleUpdateTwoTwelveCanTouch(WebServer &server, BoardConfig &boardConfig);

// Update Config: Same Numbers Can Touch
void handleUpdateSameNumbersCanTouch(WebServer &server, BoardConfig &boardConfig);

// Update Config: Same Resource Can Touch
void handleUpdateSameResourceCanTouch(WebServer &server, BoardConfig &boardConfig);

#endif