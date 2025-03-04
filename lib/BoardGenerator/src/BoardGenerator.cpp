#include "BoardGenerator.h"
#include "adjancency.h"
#include <algorithm>
#include <random>
#include <cstring>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include "esp_system.h"

// Helper function: Shuffles a vector using the Fisher-Yates algorithm.
void shuffleVector(std::vector<int> &vec)
{
    // Use the ESP32 hardware random number generator to get a more variable seed.
    unsigned int seed = esp_random();
    static std::mt19937 g(seed);
    for (int i = vec.size() - 1; i > 0; i--)
    {
        std::uniform_int_distribution<> dis(0, i);
        int j = dis(g);
        std::swap(vec[i], vec[j]);
    }
}

std::vector<int> generateResources(bool isExtension, bool sameResourceCanTouch)
{
    Serial.println("Start generating resources");
    int totalHexes = isExtension ? 30 : 19;

    // Define resource counts:
    // Classic: 4 sheep (0), 4 wood (1), 4 wheat (2), 3 brick (3), 3 ore (4), 1 desert (5)
    // Extension: 6 sheep (0), 6 wood (1), 6 wheat (2), 5 brick (3), 5 ore (4), 2 deserts (5)
    std::vector<int> resourceCounts = isExtension ? std::vector<int>{6, 6, 6, 5, 5, 2}
                                                  : std::vector<int>{4, 4, 4, 3, 3, 1};

    // If sameResourceCanTouch is true, simply generate the resource vector and shuffle it.
    if (sameResourceCanTouch)
    {
        std::vector<int> resources;
        resources.reserve(totalHexes);
        for (int type = 0; type < static_cast<int>(resourceCounts.size()); type++)
        {
            for (int i = 0; i < resourceCounts[type]; i++)
            {
                resources.push_back(type);
            }
        }
        shuffleVector(resources);
        Serial.println("Ended generating resources");
        return resources;
    }
    else
    {
        // Otherwise, enforce that adjacent tiles don't share the same resource.
        std::vector<int> board(totalHexes, -1);
        const int(*adjacencyList)[6] = isExtension ? adjacencyListExtension : adjacencyListClassic;

        std::mt19937 rng(esp_random());

        // Recursive lambda for backtracking tile assignment.
        std::function<bool(int, std::vector<int> &)> assignTile = [&](int index, std::vector<int> &counts) -> bool
        {
            // If all tiles are assigned, we're done.
            if (index == totalHexes)
                return true;

            // Determine which resource types are disallowed because of assigned neighbors.
            std::unordered_set<int> disallowed;
            for (int j = 0; j < 6; j++)
            {
                int neighbor = adjacencyList[index][j];
                if (neighbor != -1 && board[neighbor] != -1)
                    disallowed.insert(board[neighbor]);
            }

            // Build a list of candidate resource types (available and not disallowed).
            std::vector<int> candidates;
            for (int type = 0; type < static_cast<int>(counts.size()); type++)
            {
                if (counts[type] > 0 && disallowed.find(type) == disallowed.end())
                {
                    candidates.push_back(type);
                }
            }

            // Randomize the candidate order.
            std::shuffle(candidates.begin(), candidates.end(), rng);

            // Try each candidate resource type.
            for (int candidate : candidates)
            {
                board[index] = candidate;
                counts[candidate]--;

                if (assignTile(index + 1, counts))
                    return true;

                // Backtrack.
                board[index] = -1;
                counts[candidate]++;
            }

            return false; // No valid candidate found.
        };

        if (assignTile(0, resourceCounts))
        {
            Serial.println("Ended generating resources");
            return board; // Successfully generated a valid configuration.
        }
        else
        {
            Serial.println("Failed to generate resources");
            return std::vector<int>(); // Failed to generate a valid configuration.
        }
    }
}

std::vector<int> generateNumbers(bool isExtension,
                                 bool eightSixCanTouch,
                                 bool twoTwelveCanTouch,
                                 bool sameNumbersCanTouch,
                                 const std::vector<int> &resourceMap)
{
    Serial.println("Start generating numbers");
    int totalHexes = isExtension ? 30 : 19;
    std::mt19937 rng(esp_random());

    // Helper lambda to initialize token counts.
    auto initTokenCounts = [isExtension]() -> std::unordered_map<int, int>
    {
        std::unordered_map<int, int> tokenCounts;
        if (!isExtension)
        {
            tokenCounts[2] = 1;
            tokenCounts[3] = 2;
            tokenCounts[4] = 2;
            tokenCounts[5] = 2;
            tokenCounts[6] = 2;
            tokenCounts[8] = 2;
            tokenCounts[9] = 2;
            tokenCounts[10] = 2;
            tokenCounts[11] = 2;
            tokenCounts[12] = 1;
        }
        else
        {
            tokenCounts[2] = 2;
            tokenCounts[3] = 3;
            tokenCounts[4] = 3;
            tokenCounts[5] = 3;
            tokenCounts[6] = 3;
            tokenCounts[8] = 3;
            tokenCounts[9] = 3;
            tokenCounts[10] = 3;
            tokenCounts[11] = 3;
            tokenCounts[12] = 2;
        }
        return tokenCounts;
    };

    // Select the appropriate adjacency list.
    const int(*adjacencyList)[6] = isExtension ? adjacencyListExtension : adjacencyListClassic;

    // Outer loop: Keep trying until a complete board is generated.
    while (true)
    {
        std::unordered_map<int, int> tokenCounts = initTokenCounts();
        std::vector<int> boardNumbers(totalHexes, 0);
        bool restart = false; // flag to indicate if we must start over

        // Fill the board sequentially.
        for (int index = 0; index < totalHexes; index++)
        {
            Serial.print("Index: ");
            Serial.println(index);
            // For desert hexes, assign token 0.
            if (resourceMap[index] == 5)
            {
                boardNumbers[index] = 0;
                continue;
            }

            // Build the list of candidate tokens for this tile.
            std::vector<int> candidates;
            std::vector<int> possibleTokens = {2, 3, 4, 5, 6, 8, 9, 10, 11, 12};
            for (int token : possibleTokens)
            {
                if (tokenCounts[token] <= 0)
                    continue; // no tokens of this type left

                bool allowed = true;
                // Check each neighbor that's already been assigned.
                for (int j = 0; j < 6 && allowed; j++)
                {
                    int neighbor = adjacencyList[index][j];
                    if (neighbor != -1 && boardNumbers[neighbor] != 0)
                    { // neighbor is assigned (non-desert)
                        int neighborToken = boardNumbers[neighbor];

                        // Eight/Six Rule: If eightSixCanTouch is false, then a 6 or 8 cannot be adjacent to any 6 or 8.
                        if (!eightSixCanTouch && (token == 6 || token == 8))
                        {
                            if (neighborToken == 6 || neighborToken == 8)
                            {
                                allowed = false;
                                break;
                            }
                        }

                        // Two/Twelve Rule: If twoTwelveCanTouch is false and candidate is 2 or 12,
                        // then no neighbor may be 2 or 12.
                        if (!twoTwelveCanTouch && (token == 2 || token == 12))
                        {
                            if (neighborToken == 2 || neighborToken == 12)
                            {
                                allowed = false;
                                break;
                            }
                        }

                        // Same Numbers Rule: If sameNumbersCanTouch is false, no neighbor may have the same token.
                        if (!sameNumbersCanTouch && token == neighborToken)
                        {
                            allowed = false;
                            break;
                        }
                    }
                }
                if (allowed)
                    candidates.push_back(token);
            }

            // If no candidates are available, restart the entire assignment.
            if (candidates.empty())
            {
                restart = true;
                break;
            }

            // Otherwise, choose one candidate randomly.
            std::shuffle(candidates.begin(), candidates.end(), rng);
            int chosen = candidates.front();
            boardNumbers[index] = chosen;
            tokenCounts[chosen]--;
        }

        // If we successfully filled all tiles, return the board.
        if (!restart)
        {
            Serial.println("Ended generating numbers");
            return boardNumbers;
        }
        // Otherwise, log the restart and try again.
        Serial.println("No candidate possible at some tile, restarting board generation...");
    }
}

// Generates the complete board by combining resource and number layouts.
Board generateBoard(const BoardConfig &config)
{
    Board board;
    board.resources = generateResources(config.isExtension, config.sameResourceCanTouch);
    board.numbers = generateNumbers(config.isExtension,
                                    config.eightSixCanTouch,
                                    config.twoTwelveCanTouch,
                                    config.sameNumbersCanTouch,
                                    board.resources);
    return board;
}
