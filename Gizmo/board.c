#include <stdio.h>
#include <string.h>
#include "board.h"
#include "movegen.h"
#include "zobrist.h"

char *unicodePieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

int charToPiece[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k
};

// castling rights update constants
const int castlingRights[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};

const char *squareToCoords[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

inline int makeMove(Board* board, int move)
{
    copyBoard();

	int fromSquare = getMoveSource(move);
    int toSquare = getMoveTarget(move);
    int piece = getMovePiece(move);
    int promotedPiece = getMovePromoted(move);
    int capture = getMoveCapture(move);
    int doublePush = getMoveDouble(move);
    int enpassant = getMoveEnpassant(move);
    int castling = getMoveCastling(move);

	popBit(board->bitboard[piece], fromSquare);
	setBit(board->bitboard[piece], toSquare);

	board->hashKey ^= zobrist[piece][fromSquare];
	board->hashKey ^= zobrist[piece][toSquare];

	board->fiftyMove++;

	if (capture)
	{
		int startPiece, endPiece;

		if (board->side == WHITE)
		{
			startPiece = p;
			endPiece = k;
		}
		else
		{
			startPiece = P;
			endPiece = K;
		}
			
		for (int bbPiece = startPiece; bbPiece <= endPiece; bbPiece++)
		{
			if (getBit(board->bitboard[bbPiece], toSquare))
			{
				popBit(board->bitboard[bbPiece], toSquare);
				board->hashKey ^= zobrist[bbPiece][toSquare];
				break;
			}
		}
		
		board->fiftyMove = 0;
	}

	if (promotedPiece)
	{
		if (board->side == WHITE)
		{
			popBit(board->bitboard[P], toSquare);
			board->hashKey ^= zobrist[P][toSquare];
		}
		else
		{
			popBit(board->bitboard[p], toSquare);
			board->hashKey ^= zobrist[p][toSquare];
		}

		setBit(board->bitboard[promotedPiece], toSquare);
		board->hashKey ^= zobrist[promotedPiece][toSquare];
	}

	if (enpassant)
	{
		if (board->side == WHITE)
		{
			popBit(board->bitboard[p], toSquare + 8);
			board->hashKey ^= zobrist[p][toSquare + 8];
		}
		else
		{
			popBit(board->bitboard[P], toSquare - 8);
			board->hashKey ^= zobrist[P][toSquare - 8];
		}
	}

	if (board->enPassant != NO_SQR) board->hashKey ^= enpassantKey[board->enPassant];

	board->enPassant = NO_SQR;

	if (doublePush)
	{
		if (board->side == WHITE)
		{
			board->enPassant = toSquare + 8;
			board->hashKey ^= enpassantKey[toSquare + 8];
		}
		else
		{
			board->enPassant = toSquare - 8;
			board->hashKey ^= enpassantKey[toSquare - 8];
		}
	}

	if (castling)
    {
        switch (toSquare)
        {
            // white castles king side
            case G1:
                // move H rook
                popBit(board->bitboard[R], H1);
                setBit(board->bitboard[R], F1);
				board->hashKey ^= zobrist[R][H1];
				board->hashKey ^= zobrist[R][F1];
                break;
                
            // white castles queen side
            case C1:
                // move A rook
                popBit(board->bitboard[R], A1);
                setBit(board->bitboard[R], D1);
				board->hashKey ^= zobrist[R][A1];
				board->hashKey ^= zobrist[R][D1];
                break;
                
            // black castles king side
            case G8:
                // move H rook
                popBit(board->bitboard[r], H8);
                setBit(board->bitboard[r], F8);
				board->hashKey ^= zobrist[r][H8];
				board->hashKey ^= zobrist[r][F8];
                break;
                
            // black castles queen side
            case C8:
                // move A rook
                popBit(board->bitboard[r], A8);
                setBit(board->bitboard[r], D8);
				board->hashKey ^= zobrist[r][A8];
				board->hashKey ^= zobrist[r][D8];
                break;
        }
    }

	board->castling &= castlingRights[fromSquare];
	board->castling &= castlingRights[toSquare];

	board->hashKey ^= castlingKey[board->castling];

    updateOccupancy(board);

    board->side ^= 1;
	board->hashKey ^= sideKey;

	if (isSquareAttacked(board, board->side == WHITE ? getLSB(board->bitboard[k]) : getLSB(board->bitboard[K]), board->side))
	{
        takeBack();
		return 0;
	}
	else
		return 1;
}

static inline void updateOccupancy(Board* board)
{
	memset(board->occupancy, 0, sizeof(board->occupancy));

    for (int bb_piece = P; bb_piece <= K; bb_piece++)
        board->occupancy[WHITE] |= board->bitboard[bb_piece];

    for (int bb_piece = p; bb_piece <= k; bb_piece++)
        board->occupancy[BLACK] |= board->bitboard[bb_piece];

	board->occupancy[BOTH] = board->occupancy[WHITE] | board->occupancy[BLACK];
}

inline int isSquareAttacked(Board* board, int square, int side)
{
	if (side == WHITE && (pawnAttacks[BLACK][square] & board->bitboard[P])) return 1;
	if (side == BLACK && (pawnAttacks[WHITE][square] & board->bitboard[p])) return 1;
	if (knightAttacks[square] & (side == WHITE ? board->bitboard[N] : board->bitboard[n])) return 1;
	if (kingAttacks[square] & (side == WHITE ? board->bitboard[K] : board->bitboard[k])) return 1;
	if (getBishopAttacks(square, board->occupancy[BOTH]) & (side == WHITE ? board->bitboard[B] : board->bitboard[b])) return 1;
	if (getRookAttacks(square, board->occupancy[BOTH]) & (side == WHITE ? board->bitboard[R] : board->bitboard[r])) return 1;
	if (getQueenAttacks(square, board->occupancy[BOTH]) & ((side == WHITE ? board->bitboard[Q] : board->bitboard[q]))) return 1;
	
	return 0;
}

void printBoard(Board* board)
{
	for (int i = 0; i < SQUARE_NUM; i++)
	{
		if (i % 8 == 0)
			printf("\n%d ", 8 - (i / 8));
	
		int currentPiece = -1;
	
		for (int piece = P; piece <= k; piece++)
		{
			if (getBit(board->bitboard[piece], i))
			{
				currentPiece = piece;
				break;
			}
		}
		
		printf("%s ", currentPiece == -1 ? "." : unicodePieces[currentPiece]);
	}
	
	printf("\n  a b c d e f g h\n\n");
}

void printBitboard(U64 bb)
{
	for (int i = 0; i < SQUARE_NUM; i++)
	{
		if (i % 8 == 0)
			printf("\n%d ", 8 - (i / 8));
			
		printf("%d ", getBit(bb, i) ? 1 : 0);
	}
	
	printf("\n  a b c d e f g h\n\n");
}

void resetBoard(Board* board)
{
	memset(board->bitboard, 0, sizeof(board->bitboard));
    memset(board->occupancy, 0, sizeof(board->occupancy));
    memset(board->pvLength, 0, sizeof(board->pvLength));
    memset(board->pvTable, 0, sizeof(board->pvTable));
    memset(board->killerMoves, 0, sizeof(board->killerMoves));
    memset(board->historyMoves, 0, sizeof(board->historyMoves));
    memset(board->repTable, 0, sizeof(board->repTable));
	
	board->side = WHITE;
	board->enPassant = NO_SQR;
	board->ply = 0;
	board->repIndex = 0;
    board->castling = 0;
    board->hashKey = 0ULL;
}

void parseFen(Board* board, char* fen)
{
	resetBoard(board);

    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;
            
            if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z'))
            {
                int currentPiece = charToPiece[*fen];
                setBit(board->bitboard[currentPiece], square);
                fen++;
            }
            
            if (*fen >= '0' && *fen <= '9')
            {
                int offset = *fen - '0';
                
                int currentPiece = -1;
                
                for (int piece = P; piece <= k; piece++)
                {
                    if (getBit(board->bitboard[piece], square))
                        currentPiece = piece;
                }
                
                if (currentPiece == -1)
                    file--;

                file += offset;

                fen++;
            }

            if (*fen == '/')
                fen++;
        }
    }

    fen++;

    (*fen == 'w') ? (board->side = WHITE) : (board->side = BLACK);

    fen += 2;

    while (*fen != ' ')
    {
        switch (*fen)
        {
            case 'K': board->castling |= WK; break;
            case 'Q': board->castling |= WQ; break;
            case 'k': board->castling |= BK; break;
            case 'q': board->castling |= BQ; break;
            case '-': break;
        }

        fen++;
    }

    fen++;

    if (*fen != '-')
    {
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');

        board->enPassant = rank * 8 + file;
    }
    else
        board->enPassant = NO_SQR;

    updateOccupancy(board);

    board->hashKey = getZobristKey(board);
}
