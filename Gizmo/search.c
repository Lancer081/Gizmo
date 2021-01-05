#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "search.h"
#include "movegen.h"
#include "zobrist.h"
#include "uci.h"
#include "eval.h"

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

TransposTable hashTable[HASH_SIZE];

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

static inline int scoreMove(Board* board, int move)
{
	if (move == board->pvTable[0][board->ply])
		return 20000;
		
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

static inline int sortMoves(Board* board, MoveList* moves)
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

    for (int currentDepth = 1; currentDepth <= depth; currentDepth++)
    {
        if (info->stopped)
            break;

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
            if (score > -mateValue && score < -mateScore)
                printf("info score mate %d depth %d nodes %ld time %d pv ", -(score + mateValue) / 2 - 1, currentDepth, info->nodes, getTimeMS() - start);
            else if (score > mateScore && score < mateValue)
                printf("info score mate %d depth %d nodes %ld time %d pv ", (mateValue - score) / 2 + 1, currentDepth, info->nodes, getTimeMS() - start);   
            else
        	    printf("info score cp %d depth %d nodes %ld time %d pv ", score, currentDepth, info->nodes, getTimeMS() - start);

            for (int i = 0; i < pos->pvLength[0]; i++)
                printf("%s%s ", squareToCoords[getMoveSource(pos->pvTable[0][i])], squareToCoords[getMoveTarget(pos->pvTable[0][i])]);
        }
        
        printf("\n");
    }

    printf("bestmove %s%s\n", squareToCoords[getMoveSource(pos->pvTable[0][0])], squareToCoords[getMoveTarget(pos->pvTable[0][0])]);
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
    if (!(info->nodes & 2047))
        communicate(board, info);

	info->nodes++;

    if (board->ply && (isRepetition(board) || board->fiftyMove >= 100))
		return 0;

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

            if (score < -mateScore) score += board->ply;
            if (score > mateScore) score -= board->ply;

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

    if (score < -mateScore) score -= board->ply;
    if (score > mateScore) score += board->ply;

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

    if (!(info->nodes & 2047))
        communicate(board, info);
        
	info->nodes++;
	
	board->pvLength[board->ply] = board->ply;

	if (board->ply && (score = probeHash(board, depth, alpha, beta)) != NO_HASH_ENTRY && !pvNode)
		return score;

	if (board->ply && (isRepetition(board) || board->fiftyMove >= 100))
		return 0;

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
    sortMoves(board, moves);

    for (int i = 0; i < moves->count; i++)
    {
        copyBoard();

        if (!makeMove(board, moves->moves[i]))
            continue;

        board->ply++;
        board->repTable[++board->repIndex] = board->hashKey;
        legalMoves++;

		if (!movesSearched)
        	score = -negamax(board, info, depth - 1, -beta, -alpha);
        else
        {
        	// LMR - late move reduction
        	if (movesSearched >= fullDepthMoves && depth >= reductionLimit && !inCheck && !getMoveCapture(moves->moves[i]) && !getMovePromoted(moves->moves[i]))
        		score = -negamax(board, info, depth - 2, -alpha - 1, -alpha);
        	else
        		score = alpha + 1;
        	
            // PVS - principal variation search
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
