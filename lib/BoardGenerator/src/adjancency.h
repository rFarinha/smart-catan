#ifndef ADJACENCY_H
#define ADJACENCY_H

#include <Arduino.h>

static const int adjacencyListClassic[19][6] = {
  // tile 0
  {1, 3, 4, -1, -1, -1},
  // tile 1
  {0, 2, 4, 5, -1, -1},
  // tile 2
  {1, 5, 6, -1, -1, -1},
  // tile 3
  {0, 4, 7, 8, -1, -1},
  // tile 4
  {0, 1, 3, 5, 8, 9},
  // tile 5
  {1, 2, 4, 6, 9, 10},
  // tile 6
  {2, 5, 10, 11, -1, -1},
  // tile 7
  {3, 8, 12, -1, -1, -1},
  // tile 8
  {3, 4, 7, 9, 12, 13},
  // tile 9
  {4, 5, 8, 10, 13, 14},
  // tile 10
  {5, 6, 9, 11, 14, 15},
  // tile 11
  {6, 10, 15, -1, -1, -1},
  // tile 12
  {7, 8, 13, 16, 17, -1},
  // tile 13
  {8, 9, 12, 14, 17, 18},
  // tile 14
  {9, 10, 13, 15, 18, -1},
  // tile 15
  {10, 11, 14, 18, -1, -1},
  // tile 16
  {12, 17, -1, -1, -1, -1},
  // tile 17
  {12, 13, 16, 18, -1, -1},
  // tile 18
  {13, 14, 15, 17, -1, -1}
};

static const int adjacencyListExpansion[30][6] = {
  // tile 0
  {1, 3, 4, -1, -1, -1},
  // tile 1
  {0, 2, 4, 5, -1, -1},
  // tile 2
  {1, 5, 6, -1, -1, -1},
  // tile 3
  {0, 4, 7, 8, -1, -1},
  // tile 4
  {0, 1, 3, 5, 8, 9},
  // tile 5
  {1, 2, 4, 6, 9, 10},
  // tile 6
  {2, 5, 10, 11, -1, -1},
  // tile 7
  {3, 8, 12, -1, -1, -1},
  // tile 8
  {3, 4, 7, 9, 12, 13},
  // tile 9
  {4, 5, 8, 10, 13, 14},
  // tile 10
  {5, 6, 9, 11, 14, 15},
  // tile 11
  {6, 10, 15, -1, -1, -1},
  // tile 12
  {7, 8, 13, 16, 17, -1},
  // tile 13
  {8, 9, 12, 14, 17, 18},
  // tile 14
  {9, 10, 13, 15, 18, -1},
  // tile 15
  {10, 11, 14, 18, -1, -1},
  // tile 16
  {12, 17, -1, -1, -1, -1},
  // tile 17
  {12, 13, 16, 18, -1, -1},
  // tile 18
  {13, 14, 15, 17, -1, -1}
};

// Zig-zag mapping from hexagonal tile index -> LED index on the WS2812B strip.
static const int tileToLedIndexClassic[19] = {
  // Row 1 (left to right): 0,1,2
  0, 1, 2,
  // Row 2 (right to left): 6,5,4,3
  6, 5, 4, 3,
  // Row 3 (left to right): 7,8,9,10,11
  7, 8, 9, 10, 11,
  // Row 4 (right to left): 15,14,13,12
  15, 14, 13, 12,
  // Row 5 (left to right): 16,17,18
  16, 17, 18
};

// Zig-zag mapping from hexagonal tile index -> LED index on the WS2812B strip.
static const int tileToLedIndexExpansion[30] = {
  // Row 1 (left to right): 0,1,2,3
  0, 1, 2, 3,
  // Row 2 (right to left): 8,7,6,5,4
  8, 7, 6, 5, 4,
  // Row 3 (left to right): 9,10,11,12,13,14
  9, 10, 11, 12, 13, 14,
  // Row 4 (right to left): 20,19,18,17,16,15
  20, 19, 18, 17, 16, 15,
  // Row 5 (left to right): 21,22,23,24,25
  21, 22, 23, 24, 25,
  // Row 6 (right to left): 29,28,27,26
  29, 28, 27, 26
};

#endif