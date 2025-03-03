#ifndef LEDINDEX_H
#define LEDINDEX_H

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
    16, 17, 18};

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
    29, 28, 27, 26};

static const int spiralLedIndexClassic[19] = {
    0, 1, 2, 3, 11, 12, 18, 17, 16, 15, 7, 6, 5, 4, 10, 13, 14, 8, 9};

static const int spiralLedIndexExpansion[30] = {
    0, 1, 2, 3, 4, 14, 15, 25, 26, 27, 28, 29, 21, 20, 9, 8, 7, 6, 5, 13, 16, 24, 23, 22, 19, 10, 11, 12, 17, 18};

#endif