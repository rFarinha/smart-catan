/**
 * BoardGenerator.h
 *
 * Header file defining data structures and functions for generating
 * randomized Catan game boards with configurable rules.
 */

#ifndef BOARDGENERATOR_H
#define BOARDGENERATOR_H

#include <Arduino.h>
#include <vector>

/**
 * Board structure
 *
 * Encapsulates the complete board configuration with
 * resource types and number tokens for each hex.
 */
struct Board
{
    std::vector<int> resources; // Resource values for each hex
                                // 0=sheep, 1=wood, 2=wheat, 3=brick, 4=ore, 5=desert

    std::vector<int> numbers; // Number tokens for each hex
                              // Values 2-12 represent token numbers
                              // Desert hexes have value 0
};

/**
 * BoardConfig structure
 *
 * Holds configuration options for board generation
 * that control placement rules and adjacency constraints.
 */
struct BoardConfig
{
    bool isExtension = false;          // Classic (19 hexes) or extension board (30 hexes)
    bool eightSixCanTouch = false;     // Whether 6 and 8 tokens can be adjacent
    bool twoTwelveCanTouch = false;    // Whether 2 and 12 tokens can be adjacent
    bool sameNumbersCanTouch = false;  // Whether identical numbers can be adjacent
    bool sameResourceCanTouch = false; // Whether identical resources can be adjacent
};

/**
 * Generates a complete Catan board configuration
 *
 * Creates a randomized board that respects the specified configuration rules
 * for both resource placement and number token assignment.
 *
 * @param config BoardConfig with desired generation rules
 * @return Board object containing the generated board layout
 */
Board generateBoard(const BoardConfig &config);

#endif // BOARDGENERATOR_H