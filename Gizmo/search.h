#pragma once

#include "board.h"

U64 fileMasks[SQUARE_NUM];
U64 rankMasks[SQUARE_NUM];
U64 wPassedMasks[SQUARE_NUM];
U64 bPassedMasks[SQUARE_NUM];
U64 isolatedMasks[SQUARE_NUM];

static inline int evaluate(Board* board);
static inline int negamax(Board* board, SearchInfo* info, int depth, int alpha, int beta);
static inline int quiesce(Board* board, SearchInfo* info, int alpha, int beta);

static int sortMoves(Board* board, MoveList* moves);
static int scoreMove(Board* board, int move);

void initEvalMasks();
static U64 setRankFileMask(int fileNum, int rankNum);

static inline int probeHash(Board* board, int depth, int alpha, int beta);
static inline void writeHashEntry(Board* board, int score, int depth, int flag);
void clearHashTable();

void searchPosition(Board* pos, SearchInfo* info, int depth);

void perft(Board* board, SearchInfo* info, int depth);

static inline int isReptition(Board* board);

int getTimeMS();
