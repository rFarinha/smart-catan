#ifndef BOARDGENERATOR_H
#define BOARDGENERATOR_H

#include <Arduino.h>
#include <vector>

// The Board structure encapsulates the board configuration.
struct Board
{
    std::vector<int> resources; // The resource values for each hex.
    std::vector<int> numbers;   // The number tokens for each hex.
};

// Structure to hold board generation options.
struct BoardConfig
{
    bool isExpansion = false;          // Use expansion board if true.
    bool eightSixCanTouch = false;     // Option for 8/6 adjacency rules.
    bool twoTwelveCanTouch = false;    // Option for 2/12 adjacency rules.
    bool sameNumbersCanTouch = false;  // Option for same numbers touching.
    bool sameResourceCanTouch = false; // Option for same resources touching.
};

// Generates the complete board configuration based on the given options.
Board generateBoard(const BoardConfig &config);

#endif // BOARDGENERATOR_H
