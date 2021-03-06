#pragma once

#include "board.h"

#define mateValue 49000
#define mateScore 48000

static inline int negamax(Board* board, SearchInfo* info, int depth, int alpha, int beta);
static inline int quiesce(Board* board, SearchInfo* info, int alpha, int beta);

static inline int sortMoves(Board* board, MoveList* moves);
static inline int scoreMove(Board* board, int move);

static inline int probeHash(Board* board, int depth, int alpha, int beta);
static inline void writeHashEntry(Board* board, int score, int depth, int flag);
void clearHashTable();

void searchPosition(Board* pos, SearchInfo* info, int depth);

void perft(Board* board, SearchInfo* info, int depth);

static inline int isReptition(Board* board);

int getTimeMS();
