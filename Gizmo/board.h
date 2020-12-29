#pragma once

#include "defs.h"

typedef struct {
	long nodes;
	int quit;
	int movestogo;
	int movetime;
	int time;
	int inc;
	int starttime;
	int stoptime;
	int timeset;
	int stopped;
} SearchInfo;

typedef struct {
	int moves[256];
	int count;
} MoveList;

typedef struct {
	U64 hashKey;
	int depth;
	int score;
	int flag;
} TransposTable;

typedef struct {
	U64 bitboard[12];
	U64 occupancy[3];
	
	int side;
	int enPassant;
	int castling;
	int fiftyMove;
	
	int ply;

	U64 hashKey;
	
	U64 repTable[1000];
	int repIndex;
	
	// pvlength and table indexed by ply
	int pvLength[64];
	int pvTable[64][64];

	// 2 killer moves indexed by ply
	int killerMoves[2][64];

	// history moves indexed by piece and squarenum
	int historyMoves[12][64];
} Board;

void printBitboard(U64 bb);
void printBoard(Board* board);
void resetBoard(Board* board);
void parseFen(Board* board, char* fen);

int isSquareAttacked(Board* board, int square, int side);

int makeMove(Board* board, int move);
void undoMove(Board* board);

static inline void updateOccupancy(Board* board);

const char *squareToCoords[64];
