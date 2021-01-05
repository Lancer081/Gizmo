#include "eval.h"
#include "movegen.h"
#include "nnue_eval.h"

enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

int nnue_pieces[12] = { 6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7 };

int nnue_squares[64] = {
    a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

inline int evaluate(Board* board)
{
    int score = 0;
    int square = 0;
    
    U64 bitboard = 0ULL;
    
    int doublePawns = 0;

    int pieces[33];
    int squares[33];

    int index = 2;

    for (int piece = P; piece <= k; piece++)
    {
        bitboard = board->bitboard[piece];

        while(bitboard)
        {
            square = getLSB(bitboard);

            if (piece == K)
            {
                pieces[0] = nnue_pieces[piece];
                squares[0] = nnue_squares[square];
            }
            else if (piece == k)
            {
                pieces[1] = nnue_pieces[piece];
                squares[1] = nnue_squares[square];
            }
            else
            {
                pieces[index] = nnue_pieces[piece];
                squares[index] = nnue_squares[square];
                index++;
            }

            popBit(bitboard, square);
        }
    }

    pieces[index] = 0;
    squares[index] = 0;

    return evaluate_nnue(board->side, pieces, squares) * (100 - board->fiftyMove) / 100;
}
