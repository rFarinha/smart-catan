#include "BoardGenerator.h"
#include "adjancency.h"
#include <algorithm>
#include <random>
#include <cstring>

// Helper function: Shuffles a vector using the Fisher-Yates algorithm.
void shuffleVector(std::vector<int> &vec)
{
    std::random_device rd;
    std::mt19937 g(rd());
    for (int i = vec.size() - 1; i > 0; i--)
    {
        std::uniform_int_distribution<> dis(0, i);
        int j = dis(g);
        std::swap(vec[i], vec[j]);
    }
}

// Generates the resource layout and returns it as a vector.
std::vector<int> generateResources(bool isExpansion, bool sameResourceCanTouch)
{
    int totalHexes = isExpansion ? 30 : 19;
    std::vector<int> resources;
    resources.reserve(totalHexes);

    if (!isExpansion)
    {
        // Classic game counts:
        // 4 sheep (0), 4 wood (1), 4 wheat (2), 3 brick (3), 3 ore (4), 1 desert (5)
        for (int i = 0; i < 4; i++)
            resources.push_back(0);
        for (int i = 0; i < 4; i++)
            resources.push_back(1);
        for (int i = 0; i < 4; i++)
            resources.push_back(2);
        for (int i = 0; i < 3; i++)
            resources.push_back(3);
        for (int i = 0; i < 3; i++)
            resources.push_back(4);
        for (int i = 0; i < 1; i++)
            resources.push_back(5);
    }
    else
    {
        // Expansion counts:
        // 6 sheep (0), 6 wood (1), 6 wheat (2), 5 brick (3), 5 ore (4), 2 deserts (5)
        for (int i = 0; i < 6; i++)
            resources.push_back(0);
        for (int i = 0; i < 6; i++)
            resources.push_back(1);
        for (int i = 0; i < 6; i++)
            resources.push_back(2);
        for (int i = 0; i < 5; i++)
            resources.push_back(3);
        for (int i = 0; i < 5; i++)
            resources.push_back(4);
        for (int i = 0; i < 2; i++)
            resources.push_back(5);
    }

    // Shuffle the resources vector.
    shuffleVector(resources);

    return resources;
}

// Generates the number tokens and returns them as a vector.
// The resourceMap is passed in because desert hexes (represented by 5) receive a token of 0.
std::vector<int> generateNumbers(bool isExpansion,
                                 bool eightSixCanTouch,
                                 bool twoTwelveCanTouch,
                                 bool sameNumbersCanTouch,
                                 bool sameResourceCanTouch,
                                 const std::vector<int> &resourceMap)
{
    int totalHexes = isExpansion ? 30 : 19;
    int tokenCount = isExpansion ? 28 : 18; // non-desert hexes

    std::vector<int> tokens;
    if (!isExpansion)
    {
        // Classic tokens: 18 numbers.
        int classic[18] = {2, 3, 3, 4, 4, 5, 5, 6, 6, 8, 8, 9, 9, 10, 10, 11, 11, 12};
        tokens.assign(classic, classic + 18);
    }
    else
    {
        // Expansion tokens: 28 numbers.
        int expTokens[28] = {2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 6,
                             8, 8, 8, 8, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12};
        tokens.assign(expTokens, expTokens + 28);
    }

    // Shuffle the tokens vector.
    shuffleVector(tokens);

    // Prepare the board numbers vector.
    std::vector<int> boardNumbers(totalHexes, 0);
    int tokenIndex = 0;
    for (int i = 0; i < totalHexes; i++)
    {
        if (resourceMap[i] == 5)
        {
            boardNumbers[i] = 0; // no token on desert hex
        }
        else
        {
            boardNumbers[i] = tokens[tokenIndex++];
        }
    }
    return boardNumbers;
}

// Generates the complete board by combining resource and number layouts.
Board generateBoard(const BoardConfig &config)
{
    Board board;
    board.resources = generateResources(config.isExpansion, config.sameResourceCanTouch);
    board.numbers = generateNumbers(config.isExpansion,
                                    config.eightSixCanTouch,
                                    config.twoTwelveCanTouch,
                                    config.sameNumbersCanTouch,
                                    config.sameResourceCanTouch,
                                    board.resources);
    return board;
}
