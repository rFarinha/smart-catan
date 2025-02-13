#ifndef WEBHANDLERS_H
#define WEBHANDLERS_H

#include <Arduino.h>
#include <WebServer.h>
#include "LedStripControl.h"
#include "BoardGenerator.h"


// Context struct for the server handlers.
struct ServerContext {
    BoardConfig *boardConfig;
    String htmlPage;
    LEDStripControl *ledStripControl;
};

void startServer(WebServer &server, ServerContext &serverCtx);

// Update Config: 6 & 8 Can Touch
void handleUpdateEightSixCanTouch(WebServer &server, BoardConfig &boardConfig);

// Update Config: 2 & 12 Can Touch
void handleUpdateTwoTwelveCanTouch(WebServer &server, BoardConfig &boardConfig);

// Update Config: Same Numbers Can Touch
void handleUpdateSameNumbersCanTouch(WebServer &server, BoardConfig &boardConfig);

// Update Config: Same Resource Can Touch
void handleUpdateSameResourceCanTouch(WebServer &server, BoardConfig &boardConfig);

void handleRoot(WebServer &server, String htmlPage);

void handleSetClassic(WebServer &server, BoardConfig &boardConfig, LEDStripControl ledStripControl);

#endif