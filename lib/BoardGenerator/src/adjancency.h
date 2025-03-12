/**
 * adjacency.h
 *
 * This header defines adjacency relationships between hexes on Catan boards.
 * It contains lookup tables for both classic (19-hex) and extension (30-hex) boards.
 *
 * Each hex can have up to 6 neighbors (hexagonal grid).
 * The tables use -1 to indicate no neighbor in that direction.
 */

#ifndef ADJACENCY_H
#define ADJACENCY_H

#include <Arduino.h>

/**
 * Adjacency table for classic 19-hex Catan board
 *
 * Each row represents one hex (0-18), with six columns representing
 * potential neighbors in clockwise order. A value of -1 indicates no
 * neighbor in that direction (board edge).
 *
 * The layout is organized as follows:
 *   - Row 0: 3 hexes (positions 0-2)
 *   - Row 1: 4 hexes (positions 3-6)
 *   - Row 2: 5 hexes (positions 7-11)
 *   - Row 3: 4 hexes (positions 12-15)
 *   - Row 4: 3 hexes (positions 16-18)
 */
static const int adjacencyListClassic[19][6] = {
    // Row 0 (3 tiles): indices 0, 1, 2
    /* tile 0 */ {-1, -1, -1, 1, 3, 4},
    /* tile 1 */ {-1, -1, 0, 2, 4, 5},
    /* tile 2 */ {-1, -1, 1, -1, 5, 6},

    // Row 1 (4 tiles): indices 3, 4, 5, 6
    /* tile 3 */ {-1, 0, -1, 4, 7, 8},
    /* tile 4 */ {0, 1, 3, 5, 8, 9},
    /* tile 5 */ {1, 2, 4, 6, 9, 10},
    /* tile 6 */ {2, -1, 5, -1, 10, 11},

    // Row 2 (5 tiles): indices 7, 8, 9, 10, 11
    /* tile 7 */ {-1, 3, -1, 8, -1, 12},
    /* tile 8 */ {3, 4, 7, 9, 12, 13},
    /* tile 9 */ {4, 5, 8, 10, 13, 14},
    /* tile 10 */ {5, 6, 9, 11, 14, 15},
    /* tile 11 */ {6, -1, 10, -1, 14, 15},

    // Row 3 (4 tiles): indices 12, 13, 14, 15
    /* tile 12 */ {7, 8, -1, 13, -1, 16},
    /* tile 13 */ {8, 9, 12, 14, 16, 17},
    /* tile 14 */ {9, 10, 13, 15, 17, 18},
    /* tile 15 */ {10, 11, 14, -1, 18, -1},

    // Row 4 (3 tiles): indices 16, 17, 18
    /* tile 16 */ {12, 13, -1, 17, -1, -1},
    /* tile 17 */ {13, 14, 16, 18, -1, -1},
    /* tile 18 */ {14, 15, 17, -1, -1, -1}};

/**
 * Adjacency table for extension 30-hex Catan board
 *
 * Each row represents one hex (0-29), with six columns representing
 * potential neighbors in clockwise order. A value of -1 indicates no
 * neighbor in that direction (board edge).
 *
 * The layout is organized as follows:
 *   - Row 0: 4 hexes (positions 0-3)
 *   - Row 1: 5 hexes (positions 4-8)
 *   - Row 2: 6 hexes (positions 9-14)
 *   - Row 3: 6 hexes (positions 15-20)
 *   - Row 4: 5 hexes (positions 21-25)
 *   - Row 5: 4 hexes (positions 26-29)
 */
static const int adjacencyListExtension[30][6] = {
    // Row 0 (4 tiles)
    /* tile 0 */ {-1, -1, -1, 1, 4, 5},
    /* tile 1 */ {-1, -1, 0, 2, 5, 6},
    /* tile 2 */ {-1, -1, 1, 3, 6, 7},
    /* tile 3 */ {-1, -1, 2, -1, 7, 8},

    // Row 1 (5 tiles)
    /* tile 4 */ {-1, 0, -1, 5, 9, 10},
    /* tile 5 */ {0, 1, 4, 6, 10, 11},
    /* tile 6 */ {1, 2, 5, 7, 11, 12},
    /* tile 7 */ {2, 3, 6, 8, 12, 13},
    /* tile 8 */ {3, -1, 7, -1, 13, 14},

    // Row 2 (6 tiles)
    /* tile 9  */ {-1, 4, -1, 10, -1, 15},
    /* tile 10 */ {4, 5, 9, 11, 15, 16},
    /* tile 11 */ {5, 6, 10, 12, 16, 17},
    /* tile 12 */ {6, 7, 11, 13, 17, 18},
    /* tile 13 */ {7, 8, 12, 14, 18, 19},
    /* tile 14 */ {8, -1, 13, -1, 19, 20},

    // Row 3 (6 tiles)
    /* tile 15 */ {9, 10, -1, 16, -1, 21},
    /* tile 16 */ {10, 11, 15, 17, 21, 22},
    /* tile 17 */ {11, 12, 16, 18, 22, 23},
    /* tile 18 */ {12, 13, 17, 19, 23, 24},
    /* tile 19 */ {13, 14, 18, 20, 24, 25},
    /* tile 20 */ {14, -1, 19, -1, 25, -1},

    // Row 4 (5 tiles)
    /* tile 21 */ {15, 16, -1, 22, -1, 26},
    /* tile 22 */ {16, 17, 21, 23, 26, 27},
    /* tile 23 */ {17, 18, 22, 24, 27, 28},
    /* tile 24 */ {18, 19, 23, 25, 28, 29},
    /* tile 25 */ {19, 20, 24, -1, 29, -1},

    // Row 5 (4 tiles)
    /* tile 26 */ {21, 22, -1, 27, -1, -1},
    /* tile 27 */ {22, 23, 26, 28, -1, -1},
    /* tile 28 */ {23, 24, 27, 29, -1, -1},
    /* tile 29 */ {24, 25, 28, -1, -1, -1}};

#endif