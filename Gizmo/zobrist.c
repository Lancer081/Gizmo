#include <stdlib.h>
#include "zobrist.h"
#include "board.h"
#include "movegen.h"

U64 rand64()
{
    return rand() ^ ((U64)rand() << 15) ^ ((U64)rand() << 30) ^ ((U64)rand() << 45) ^ ((U64)rand() << 60);
}

U64 getZobristKey(Board* board)
{
    U64 finalKey = 0ULL;
    U64 bitboard = 0ULL;

    for (int piece = P; piece <= k; piece++)
    {
        bitboard = board->bitboard[piece];

        while (bitboard)
        {
            int square = getLSB(bitboard);
            finalKey ^= zobrist[piece][square];
            popBit(bitboard, square);
        }
    }

    if (board->enPassant != NO_SQR)
        finalKey ^= enpassantKey[board->enPassant];

    finalKey ^= castlingKey[board->castling];

    if (board->side == BLACK)
        finalKey ^= sideKey;

    return finalKey;
}

void initZobrist()
{
    for (int piece = P; piece <= k; piece++)
        for (int square = 0; square < SQUARE_NUM; square++)
             zobrist[piece][square] = rand64();

    for (int square = 0; square < SQUARE_NUM; square++)
        enpassantKey[square] = rand64();

    for (int i = 0; i < 16; i++)
        castlingKey[i] = rand64();

    sideKey = rand64();
}