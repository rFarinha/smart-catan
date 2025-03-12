/**
 * LedIndex.h
 *
 * This header defines mappings between tile indices and LED indices
 * for the physical LED strip visualization of the Catan board.
 *
 * Two board types are supported:
 * - Classic (19 hexes)
 * - Extension (30 hexes)
 *
 * For each board type, we have two mappings:
 * 1. Normal mapping (tile index to LED index)
 * 2. Spiral mapping (used for animations)
 */

#ifndef LEDINDEX_H
#define LEDINDEX_H

/**
 * Classic board mapping from tile index to LED index
 *
 * Maps the logical hex position (0-18) to the physical LED position
 * on the WS2812B LED strip for the classic 19-hex board.
 *
 * The mapping follows a zig-zag pattern to simplify wiring:
 * - Row 1 (left to right): LEDs 0-2
 * - Row 2 (right to left): LEDs 3-6
 * - Row 3 (left to right): LEDs 7-11
 * - Row 4 (right to left): LEDs 12-15
 * - Row 5 (left to right): LEDs 16-18
 */
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

/**
 * Extension board mapping from tile index to LED index
 *
 * Maps the logical hex position (0-29) to the physical LED position
 * on the WS2812B LED strip for the extension 30-hex board.
 *
 * The mapping follows a zig-zag pattern to simplify wiring:
 * - Row 1 (left to right): LEDs 0-3
 * - Row 2 (right to left): LEDs 4-8
 * - Row 3 (left to right): LEDs 9-14
 * - Row 4 (right to left): LEDs 15-20
 * - Row 5 (left to right): LEDs 21-25
 * - Row 6 (right to left): LEDs 26-29
 */
static const int tileToLedIndexExtension[30] = {
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

/**
 * Spiral ordering of LEDs for classic board animations
 *
 * Maps sequential spiral steps (0-18) to LED indices for
 * creating spiral animation effects on the classic board.
 *
 * The spiral starts from the outside and works inward.
 */
static const int spiralLedIndexClassic[19] = {
    0, 1, 2, 3, 11, 12, 18, 17, 16, 15, 7, 6, 5, 4, 10, 13, 14, 8, 9};

/**
 * Spiral ordering of LEDs for extension board animations
 *
 * Maps sequential spiral steps (0-29) to LED indices for
 * creating spiral animation effects on the extension board.
 *
 * The spiral starts from the outside and works inward.
 */
static const int spiralLedIndexExtension[30] = {
    0, 1, 2, 3, 4, 14, 15, 25, 26, 27, 28, 29, 21, 20, 9, 8, 7, 6, 5, 13, 16, 24, 23, 22, 19, 10, 11, 12, 17, 18};

#endif