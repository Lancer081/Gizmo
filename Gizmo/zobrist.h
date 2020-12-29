#pragma once

#include "defs.h"
#include "board.h"

U64 enpassantKey[SQUARE_NUM];
U64 castlingKey[16];
U64 sideKey;

U64 zobrist[12][SQUARE_NUM];

U64 rand64();

U64 getZobristKey(Board* board);

void initZobrist();