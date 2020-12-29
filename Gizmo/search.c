#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "search.h"
#include "movegen.h"
#include "uci.h"
#include "zobrist.h"

int materialScore[12] = {
    100,      // white pawn score
    300,      // white knight score
    325,      // white bishop score
    500,      // white rook score
    900,      // white queen score
  20000,      // white king score
   -100,      // black pawn score
   -300,      // black knight score
   -325,      // black bishop score
   -500,      // black rook score
   -900,      // black queen score
 -20000,      // black king score
};

const int pawnScore[64] = 
{
    0,  0,  0,  0,  0,  0,  0,  0,
	50, 50, 50, 50, 50, 50, 50, 50,
	10, 10, 20, 30, 30, 20, 10, 10,
 	5,  5, 10, 25, 25, 10,  5,  5,
 	0,  0,  0, 20, 20,  0,  0,  0,
 	5, -5,-10,  0,  0,-10, -5,  5,
 	5, 10, 10,-20,-20, 10, 10,  5,
 	0,  0,  0,  0,  0,  0,  0,  0
};

const int knightScore[64] = 
{
    -50,-40,-30,-30,-30,-30,-40,-50,
	-40,-20,  0,  0,  0,  0,-20,-40,
	-30,  0, 10, 15, 15, 10,  0,-30,
	-30,  5, 15, 20, 20, 15,  5,-30,
	-30,  0, 15, 20, 20, 15,  0,-30,
	-30,  5, 10, 15, 15, 10,  5,-30,
	-40,-20,  0,  5,  5,  0,-20,-40,
	-50,-40,-30,-30,-30,-30,-40,-50
};

const int bishopScore[64] = 
{
    -20,-10,-10,-10,-10,-10,-10,-20,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-10,  0,  5, 10, 10,  5,  0,-10,
	-10,  5,  5, 10, 10,  5,  5,-10,
	-10,  0, 10, 10, 10, 10,  0,-10,
	-10, 10, 10, 10, 10, 10, 10,-10,
	-10,  5,  0,  0,  0,  0,  5,-10,
	-20,-10,-10,-10,-10,-10,-10,-20

};

const int rookScore[64] =
{
     0,  0,  0,  0,  0,  0,  0,  0,
 	 5, 10, 10, 10, 10, 10, 10,  5,
	-5,  0,  0,  0,  0,  0,  0, -5,
 	-5,  0,  0,  0,  0,  0,  0, -5,
 	-5,  0,  0,  0,  0,  0,  0, -5,
	-5,  0,  0,  0,  0,  0,  0, -5,
 	-5,  0,  0,  0,  0,  0,  0, -5,
 	 0,  0,  0,  5,  5,  0,  0,  0

};

const int queenScore[64] = 
{
	-20,-10,-10, -5, -5,-10,-10,-20,
	-10,  0,  0,  0,  0,  0,  0,-10,
	-10,  0,  5,  5,  5,  5,  0,-10,
 	 -5,  0,  5,  5,  5,  5,  0, -5,
	 -5,  0,  5,  5,  5,  5,  0, -5,
	-10,  5,  5,  5,  5,  5,  0,-10,
	-10,  0,  5,  0,  0,  0,  0,-10,
	-20,-10,-10, -5, -5,-10,-10,-20
};

const int kingScore[64] = 
{
    -30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-30,-40,-40,-50,-50,-40,-40,-30,
	-20,-30,-30,-40,-40,-30,-30,-20,
	-10,-20,-20,-20,-20,-20,-20,-10,
	 20, 20,  0,  0,  0,  0, 20, 20,
	 20, 30, 10,  0,  0, 10, 30, 20
};

const int kingEndgameScore[64] = 
{
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

const int mirrorScore[64] =
{
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8
};

// MVV LVA [attacker][victim]
static int MVV_LVA[12][12] = {
 	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};

static const int bishopUnit = 4;
static const int queenUnit = 9;

const int passedPawnBonus[8] = { 0, 10, 30, 50, 75, 100, 150, 200 }; 

const int getRank[64] =
{
    7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0
};

TransposTable hashTable[HASH_SIZE];

int scorePV;
int followPV;

int getTimeMS()
{
	#ifdef WIN64
        return GetTickCount();
    #else
        struct timeval time_value;
        gettimeofday(&time_value, NULL);
        return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
    #endif
}

void perft(Board* board, SearchInfo* info, int depth)
{
    if (depth == 0)
    {
        info->nodes++;
        return;
    }

    MoveList moves[1];
	generateMoves(board, moves);

    for (int i = 0; i < moves->count; i++)
    {
        copyBoard();

        if (!makeMove(board, moves->moves[i]))
            continue;

        perft(board, info, depth - 1);

        takeBack();
    }

    return;
}

static inline void enablePVScoring(Board* board, MoveList* moves)
{
	followPV = 0;

	for (int i = 0; i < moves->count; i++)
	{
		if (board->pvTable[0][board->ply] == moves->moves[i])
		{
			scorePV = 1;
			followPV = 1;
            break;
		}
	}
}

static int scoreMove(Board* board, int move)
{
	if (scorePV)
		if (move == board->pvTable[0][board->ply])
        {
            scorePV = 0;
			return 20000;
        }
		
	if (getMoveCapture(move))
	{
		int targetPiece = P;
		int startPiece, endPiece;
		
		if (board->side == WHITE) { startPiece = p; endPiece = k; }
		else { startPiece = P; endPiece = K; }
		
		for (int bbPiece = startPiece; bbPiece <= endPiece; bbPiece++)
		{
			if (getBit(board->bitboard[bbPiece], getMoveTarget(move)))
			{
				targetPiece = bbPiece;
				break;
			}
		}
	
		return MVV_LVA[getMovePiece(move)][targetPiece] + 10000;
	}
	else
	{
		if (board->killerMoves[0][board->ply] == move)
			return 9000;
		else if (board->killerMoves[1][board->ply] == move)
			return 8000;
		else
			return board->historyMoves[getMovePiece(move)][getMoveTarget(move)];
	}
	
	return 0;
}

static int sortMoves(Board* board, MoveList* moves)
{
	int moveScores[moves->count];

    for (int i = 0; i < moves->count; i++)
    	moveScores[i] = scoreMove(board, moves->moves[i]);
    	
    for (int currentMove = 0; currentMove < moves->count; currentMove++)
	{
		for (int nextMove = currentMove + 1; nextMove < moves->count; nextMove++)
		{
			if (moveScores[currentMove] < moveScores[nextMove])
			{
				int tempScore = moveScores[currentMove];
				moveScores[currentMove] = moveScores[nextMove];
				moveScores[nextMove] = tempScore;
				
				int tempMove = moves->moves[currentMove];
				moves->moves[currentMove] = moves->moves[nextMove];
				moves->moves[nextMove] = tempMove;
			}
		}
	}
}

void searchPosition(Board* pos, SearchInfo* info, int depth)
{
    int score = 0;

    int alpha = -INF;
    int beta = INF;

    int start = getTimeMS();
    
    memset(pos->pvLength, 0, sizeof(pos->pvLength));
    memset(pos->pvTable, 0, sizeof(pos->pvTable));
    memset(pos->killerMoves, 0, sizeof(pos->killerMoves));
    memset(pos->historyMoves, 0, sizeof(pos->historyMoves));

    info->nodes = 0;
    
    scorePV = 0;
	followPV = 0;

    for (int currentDepth = 1; currentDepth <= depth; currentDepth++)
    {
        if (info->stopped)
            break;
            
        followPV = 1;

        score = negamax(pos, info, currentDepth, alpha, beta);

		if ((score <= alpha) || (score >= beta)) 
		{
            alpha = -INF;    
            beta = INF;      
            continue;
        }
        
        alpha = score - 50;
        beta = score + 50;

		if (pos->pvLength[0])
        {
        	printf("info score cp %d depth %d nodes %ld time %d pv ", score, currentDepth, info->nodes, getTimeMS() - start);

            for (int i = 0; i < pos->pvLength[0]; i++)
                printf("%s%s ", squareToCoords[getMoveSource(pos->pvTable[0][i])], squareToCoords[getMoveTarget(pos->pvTable[0][i])]);
        }
        
        printf("\n");
    }

    printf("bestmove %s%s\n", squareToCoords[getMoveSource(pos->pvTable[0][0])], squareToCoords[getMoveTarget(pos->pvTable[0][0])]);
}

static U64 setRankFileMask(int fileNum, int rankNum)
{
	U64 mask = 0ULL;
	
	for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
        	int square = rank * 8 + file;
        	
        	if (fileNum != -1)
        		if (file == fileNum)
        			mask |= setBit(mask, square);
        	else if (rankNum != -1)
        		if (rank == rankNum)
        			mask |= setBit(mask, square);
        }
    }
    
    return mask;
}

void initEvalMasks()
{
	for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
        	int square = rank * 8 + file;
        	
        	rankMasks[square] |= setRankFileMask(file, -1);
        	fileMasks[square] |= setRankFileMask(-1, rank);
        	
        	isolatedMasks[square] |= setRankFileMask(file - 1, -1);
            isolatedMasks[square] |= setRankFileMask(file + 1, -1);
            
            wPassedMasks[square] |= setRankFileMask(file - 1, -1);
            wPassedMasks[square] |= setRankFileMask(file, -1);
            wPassedMasks[square] |= setRankFileMask(file + 1, -1);

            for (int i = 0; i < (8 - rank); i++)
                wPassedMasks[square] &= ~rankMasks[(7 - i) * 8 + file];
                
            bPassedMasks[square] |= setRankFileMask(file - 1, -1);
            bPassedMasks[square] |= setRankFileMask(file, -1);
            bPassedMasks[square] |= setRankFileMask(file + 1, -1);

            for (int i = 0; i < rank + 1; i++)
                bPassedMasks[square] &= ~rankMasks[i * 8 + file];
        }
    }
}

static inline int isEndgame(Board *board)
{
    return (countBits(board->bitboard[Q]) == 0 && countBits(board->bitboard[q]) == 0 && 
           (countBits(board->bitboard[R]) == 1 || countBits(board->bitboard[r] == 1) ||
           countBits(board->bitboard[P] | board->bitboard[p]) <= 3));
}

static inline int evaluate(Board* board)
{
    int score = 0;
    int square = 0;
    
    U64 bitboard = 0ULL;
    
    int doublePawns = 0;

    for (int piece = P; piece <= k; piece++)
    {
        bitboard = board->bitboard[piece];

        while(bitboard)
        {
            square = getLSB(bitboard);

            score += materialScore[piece];

            switch (piece)
            {
            case P:
                score += pawnScore[square];
                //score += countBits(pawnAttacks[WHITE][square] & board->occupancy[BLACK]);

                /*doublePawns = countBits(board->bitboard[P] & fileMasks[square]);
                
                if (doublePawns > 1)
                	score += isEndgame(board) ? (doublePawns - 1) * -10 : (doublePawns - 1) * -5;
                
                if ((board->bitboard[P] & isolatedMasks[square]) == 0)
                    (isEndgame(board)) ? (score += -10) : (score += -5);
  
                if ((wPassedMasks[square] & board->bitboard[p]) == 0)
                    score += passedPawnBonus[getRank[square]];*/
            
                break;
            case N: 
				score += knightScore[square];
                //score += countBits(knightAttacks[square] & ~board->occupancy[WHITE]);
                //score += countBits(board->bitboard[P] | board->bitboard[p]); // decreasing value as pawns dissapear
                /*if (board->bitboard[N] & knightAttacks[square])
                    score += 5;*/
				break;
            case B: 
				score += bishopScore[square];
				score += countBits(getBishopAttacks(square, board->occupancy[BOTH])) / 2;
				break;
            case R: 
				score += rookScore[square];
				score += countBits(getRookAttacks(square, board->occupancy[BOTH])) / 2;
                //score -= countBits(board->bitboard[P] | board->bitboard[p]); // increasing value as pawns dissapear

				if ((board->bitboard[P] & fileMasks[square]) == 0)
					score += 10;
					
				if (((board->bitboard[P] | board->bitboard[p]) & fileMasks[square]) == 0)
					score += 15;

				break;
            case Q: 
				score += queenScore[square];
				score += countBits(getQueenAttacks(square, board->occupancy[BOTH])) / 2;
                //score += isEndgame(board) ? (countBits(getQueenAttacks(square, board->occupancy[BOTH])) - queenUnit) * 2 : countBits(getQueenAttacks(square, board->occupancy[BOTH])) - queenUnit;
				break;
            case K:
				score += kingScore[square];

                //score += countBits((kingAttacks[square] & ~board->occupancy[WHITE]) | (kingAttacks[square] & board->occupancy[BLACK]));
				
                score += countBits(kingAttacks[square] & board->occupancy[WHITE]) * 5; // king safety
                	
				break;

            case p: 
				score -= pawnScore[mirrorScore[square]];
                //score -= countBits(pawnAttacks[BLACK][square] & board->occupancy[WHITE]);

				/*doublePawns = countBits(board->bitboard[p] & fileMasks[square]);
				
				if (doublePawns > 1)
					score -= isEndgame(board) ? (doublePawns - 1) * -10 : (doublePawns - 1) * -5;
					
				if ((board->bitboard[p] & isolatedMasks[square]) == 0)
                    (isEndgame(board)) ? (score -= -10) : (score -= -5);
  
                if ((bPassedMasks[square] & board->bitboard[P]) == 0)
                    score -= passedPawnBonus[getRank[square]];*/

				break;
            case n: 
				score -= knightScore[mirrorScore[square]];
                //score -= countBits(knightAttacks[square] & ~board->occupancy[BLACK]);
                //score -= countBits(board->bitboard[P] | board->bitboard[p]);
                /*if (board->bitboard[N] & knightAttacks[square])
                    score -= 5;*/
				break;
            case b: 
				score -= bishopScore[mirrorScore[square]];
				score -= countBits(getBishopAttacks(square, board->occupancy[BOTH])) / 2;
				break;
            case r: 
				score -= rookScore[mirrorScore[square]];
                score -= countBits(getRookAttacks(square, board->occupancy[BOTH])) / 2;
                //score += countBits(board->bitboard[P] | board->bitboard[p]);

				if ((board->bitboard[p] & fileMasks[square]) == 0)
					score -= 10;
					
				if (((board->bitboard[P] | board->bitboard[p]) & fileMasks[square]) == 0)
					score -= 15;
				break;
            case q:
				score -= queenScore[mirrorScore[square]];
				score -= countBits(getQueenAttacks(square, board->occupancy[BOTH])) / 2;
				break;
            case k: 
				score -= kingScore[mirrorScore[square]];
				
                //score -= countBits((kingAttacks[square] & ~board->occupancy[BLACK]) | (kingAttacks[square] & board->occupancy[WHITE]));

                score -= countBits(kingAttacks[square] & board->occupancy[BLACK]) * 5; // king safety
                
				break;
            }

            popBit(bitboard, square);
        }
    }

    return board->side == WHITE ? score : -score;
}

static inline int isRepetition(Board* board)
{
	for (int i = 0; i < board->repIndex; i++)
		if (board->hashKey == board->repTable[i])
			return 1;
	
	return 0;
}

static inline int quiesce(Board* board, SearchInfo* info, int alpha, int beta)
{
    if ((info->nodes & 2047) == 0)
        communicate(board, info);

	info->nodes++;

    int score = evaluate(board);

    if (score >= beta)
        return beta;
    if (score > alpha)
        alpha = score;

    MoveList moves[1];
    generateCaptures(board, moves);
    sortMoves(board, moves);

    for (int i = 0; i < moves->count; i++)
    {
        copyBoard();

        if (!makeMove(board, moves->moves[i]))
            continue;

        score = -quiesce(board, info, -beta, -alpha);

        takeBack();

        if (info->stopped)
            return 0;

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

static inline int probeHash(Board* board, int depth, int alpha, int beta)
{
    TransposTable *hashEntry = &hashTable[board->hashKey % HASH_SIZE];

    if (hashEntry->hashKey == board->hashKey)
    {
        if (hashEntry->depth >= depth)
        {
            int score = hashEntry->score;

            if (hashEntry->flag == HFLAG_EXACT)
                return score;
            if ((hashEntry->flag == HFLAG_ALPHA) && (score <= alpha))
                return alpha;
            if ((hashEntry->flag == HFLAG_BETA) && (score >= beta))
                return beta;
        }
    }

    return NO_HASH_ENTRY;
}

static inline void writeHashEntry(Board* board, int score, int depth, int flag)
{
    TransposTable *hashEntry = &hashTable[board->hashKey % HASH_SIZE];

    hashEntry->hashKey = board->hashKey;
    hashEntry->score = score;
    hashEntry->depth = depth;
    hashEntry->flag = flag;
}

void clearHashTable()
{
    for (int i = 0; i < HASH_SIZE; i++)
    {
        hashTable[i].hashKey = 0;
        hashTable[i].depth = 0;
        hashTable[i].flag = 0;
        hashTable[i].score = 0;
    }
}

const int fullDepthMoves = 4;
const int reductionLimit = 3;

static inline int negamax(Board* board, SearchInfo* info, int depth, int alpha, int beta)
{
    int score = 0;
	int pvNode = beta - alpha > 1;
    int hashFlag = HFLAG_ALPHA;

    board->pvLength[board->ply] = board->ply;

    if ((info->nodes & 2047) == 0)
        communicate(board, info);
        
	info->nodes++;

	if (board->ply && (isRepetition(board) || board->fiftyMove >= 100))
		return 0;
	
	if (board->ply && (score = probeHash(board, depth, alpha, beta)) != NO_HASH_ENTRY && pvNode == 0)
		return score;

    if (depth == 0)
        return quiesce(board, info, alpha, beta);

    int inCheck = isSquareAttacked(board, board->side == WHITE ? getLSB(board->bitboard[K]) : getLSB(board->bitboard[k]), board->side ^ 1);
    if (inCheck) depth++;
    
    // null move pruning
    if (depth >= 3 && !inCheck && board->ply)
    {
    	copyBoard();
    	
    	board->ply++;
    	board->repTable[++board->repIndex] = board->hashKey;
    	
    	board->side ^= 1;
    	board->hashKey ^= sideKey;
    	
    	if (board->enPassant != NO_SQR) 
            board->hashKey ^= enpassantKey[board->enPassant];
    	
		board->enPassant = NO_SQR;
		
		score = -negamax(board, info, depth - 3, -beta, -beta + 1);
		
		board->ply--;
		board->repIndex--;
		
		takeBack();
		
		if (info->stopped)
			return 0;
			
		if (score >= beta)
			return beta;
    }

    int legalMoves = 0;
    int movesSearched = 0;
    
    MoveList moves[1];
    generateMoves(board, moves);
    
    if (followPV)
    	enablePVScoring(board, moves);
    
    sortMoves(board, moves);

    for (int i = 0; i < moves->count; i++)
    {
        copyBoard();

        if (!makeMove(board, moves->moves[i]))
            continue;

        board->ply++;
        board->repTable[++board->repIndex] = board->hashKey;
        legalMoves++;

		// PVS - principal variation search
		if (movesSearched == 0)
        	score = -negamax(board, info, depth - 1, -beta, -alpha);
        else
        {
        	// LMR - late move reduction
        	if (movesSearched >= fullDepthMoves && depth >= reductionLimit && !inCheck && !getMoveCapture(moves->moves[i]) && !getMovePromoted(moves->moves[i]))
        		score = -negamax(board, info, depth - 2, -alpha - 1, -alpha);
        	else
        		score = alpha + 1;
        		
        	if (score > alpha)
			{
                score = -negamax(board, info, depth - 1, -alpha - 1, -alpha);
            
                if ((score > alpha) && (score < beta))
                    score = -negamax(board, info, depth - 1, -beta, -alpha);
			}
        }

        board->ply--;
        board->repIndex--;

        takeBack();
        
        movesSearched++;

        if (info->stopped)
            return 0;

        if (score >= beta)
        {
            writeHashEntry(board, beta, depth, HFLAG_BETA);

            if (!getMoveCapture(moves->moves[i]))
			{
				board->killerMoves[1][board->ply] = board->killerMoves[0][board->ply];
				board->killerMoves[0][board->ply] = moves->moves[i];
			}

            return beta;
        }

        if (score > alpha)
        {
            hashFlag = HFLAG_EXACT;
            alpha = score;

            board->pvTable[board->ply][board->ply] = moves->moves[i];

            for (int nextPly = board->ply + 1; nextPly < board->pvLength[board->ply + 1]; nextPly++)
                board->pvTable[board->ply][nextPly] = board->pvTable[board->ply + 1][nextPly];

            board->pvLength[board->ply] = board->pvLength[board->ply + 1];

            if (!getMoveCapture(moves->moves[i]))
                board->historyMoves[getMovePiece(moves->moves[i])][getMoveTarget(moves->moves[i])] += depth;
        }
    }

    if (legalMoves == 0)
    {
        if (inCheck)
            return -49000 + board->ply;
        else
            return 0;
    }

    writeHashEntry(board, alpha, depth, hashFlag);

    return alpha;
}
