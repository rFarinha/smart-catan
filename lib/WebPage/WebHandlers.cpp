// --------------------------------------------------------------
//                  SERVER HANDLERS
// --------------------------------------------------------------

#include "WebHandlers.h"
#include "LedStripControl.h"

void startServer(WebServer &server, ServerContext &serverCtx)
{
    // Set up server routes
    server.on("/", HTTP_GET, [&server, &serverCtx]()
              { handleRoot(server, serverCtx.htmlPage); });
    // Endpoints for updating settings:
    server.on("/eightSixCanTouch", HTTP_GET, [&server, &serverCtx]()
              { handleUpdateEightSixCanTouch(server, *serverCtx.boardConfig); });
    server.on("/twoTwelveCanTouch", HTTP_GET, [&server, &serverCtx]()
              { handleUpdateTwoTwelveCanTouch(server, *serverCtx.boardConfig); });
    server.on("/sameNumbersCanTouch", HTTP_GET, [&server, &serverCtx]()
              { handleUpdateSameNumbersCanTouch(server, *serverCtx.boardConfig); });
    server.on("/sameResourceCanTouch", HTTP_GET, [&server, &serverCtx]()
              { handleUpdateSameResourceCanTouch(server, *serverCtx.boardConfig); });

    // Endpoints for Classic and Expansion buttons
    server.on("/setClassic", HTTP_GET, [&server, &serverCtx]()
              { handleSetClassic(server, *serverCtx.boardConfig, *serverCtx.ledStripControl); });

    // Start server
    server.begin();
    Serial.println("HTTP server started.");
}

// Update Config: 6 & 8 Can Touch
void handleUpdateEightSixCanTouch(WebServer &server, BoardConfig &boardConfig)
{
    String value = server.arg("value"); // Expecting "1" for true or "0" for false
    boardConfig.eightSixCanTouch = (value == "1");
    Serial.print("8 & 6 Can Touch set to: ");
    Serial.println(boardConfig.eightSixCanTouch ? "true" : "false");
    server.send(200, "text/plain", "eightSixCanTouch updated");
}

// Update Config: 2 & 12 Can Touch
void handleUpdateTwoTwelveCanTouch(WebServer &server, BoardConfig &boardConfig)
{
    String value = server.arg("value");
    boardConfig.twoTwelveCanTouch = (value == "1");
    Serial.print("2 & 12 Can Touch set to: ");
    Serial.println(boardConfig.twoTwelveCanTouch ? "true" : "false");
    server.send(200, "text/plain", "twoTwelveCanTouch updated");
}

// Update Config: Same Numbers Can Touch
void handleUpdateSameNumbersCanTouch(WebServer &server, BoardConfig &boardConfig)
{
    String value = server.arg("value");
    boardConfig.sameNumbersCanTouch = (value == "1");
    Serial.print("Same Numbers Can Touch set to: ");
    Serial.println(boardConfig.sameNumbersCanTouch ? "true" : "false");
    server.send(200, "text/plain", "sameNumbersCanTouch updated");
}

// Update Config: Same Resource Can Touch
void handleUpdateSameResourceCanTouch(WebServer &server, BoardConfig &boardConfig)
{
    String value = server.arg("value");
    boardConfig.sameResourceCanTouch = (value == "1");
    Serial.print("Same Resource Can Touch set to: ");
    Serial.println(boardConfig.sameResourceCanTouch ? "true" : "false");
    server.send(200, "text/plain", "sameResourceCanTouch updated");
}

// Serve the main HTML page
void handleRoot(WebServer &server, String htmlPage)
{
    server.send(200, "text/html", htmlPage);
}

// Set game as Classic
void handleSetClassic(WebServer &server, BoardConfig &boardConfig, LEDStripControl ledStripControl)
{
    Serial.println("[/setclassic] Request received. Setting game as classic");
    boardConfig.isExpansion = false;

    // Only reinitialize led strip if the current count is not already 19
    if (ledStripControl.getNumLeds() != 19)
    {
        // This calls restart() internally
        ledStripControl.setNumLeds(19);  
    }

    // Shuffle
    Board board = generateBoard(boardConfig);

    Serial.println("RESOURCES:");
    for (int i = 0; i < 19; i++)
    {
        Serial.print(board.resources[i]);
        Serial.print(" ");
    }
    Serial.println("Numbers:");
    for (int i = 0; i < 19; i++)
    {
        Serial.print(board.numbers[i]);
        Serial.print(" ");
    }
}

/*
// Set game as Expansion
void handleSetExpansion()
{
    Serial.println("[/setexpansion] Request received. Setting game as expansion");
    is_expansion = true;

    // Only reinitialize if the current count is not already 30
    if (currentNumLeds != 30)
    {
        currentNumLeds = 30;

        // Clean up the old LED strip object
        restartLedStrip();
    }
}*/