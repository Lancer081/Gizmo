#pragma once

#include "defs.h"
#include "board.h"

U64 pawnAttacks[2][SQUARE_NUM];
U64 knightAttacks[SQUARE_NUM];
U64 kingAttacks[SQUARE_NUM];
U64 bishopAttacks[SQUARE_NUM][512];
U64 rookAttacks[SQUARE_NUM][4096];
U64 bishopMasks[SQUARE_NUM];
U64 rookMasks[SQUARE_NUM];

U64 maskPawnAttacks(int side, int square);
U64 maskKnightAttacks(int square);
U64 maskKingAttacks(int square);
U64 maskRookAttacks(int square);
U64 maskBishopAttacks(int square);

U64 rookAttacksOTF(int square, U64 blockers);
U64 bishopAttacksOTF(int square, U64 blockers);

U64 getQueenAttacks(int square, U64 occupancy);
U64 getBishopAttacks(int square, U64 occupancy);
U64 getRookAttacks(int square, U64 occupancy);

U64 setOccupancy(int index, int bitsInMask, U64 attackMask);

int countBits(U64 bb);
int getLSB(U64 bb);

void generateMoves(Board* board, MoveList* moves);
void generateCaptures(Board* board, MoveList* moves);

static inline void addMove(MoveList* moves, int move);

void initLeaperMasks();
void initSliderMasks();
