#pragma once

#define U64 unsigned long long

#define SQUARE_NUM 64
#define INF 50000

#define HASH_SIZE 0x400000
#define NO_HASH_ENTRY -1
#define HFLAG_EXACT 0
#define HFLAG_ALPHA 1
#define HFLAG_BETA 2

#define startPosition "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define trickyPosition "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killerPosition "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmkPosition "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "
#define repetitions "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40 "

#define encodeMove(source, target, piece, promoted, capture, doublePawn, enpassant, castling) \
    (source) |          \
    (target << 6) |     \
    (piece << 12) |     \
    (promoted << 16) |  \
    (capture << 20) |   \
    (doublePawn << 21) |    \
    (enpassant << 22) | \
    (castling << 23)    \

#define getMoveSource(move) (move & 0x3f)
#define getMoveTarget(move) ((move & 0xfc0) >> 6)
#define getMovePiece(move) ((move & 0xf000) >> 12)
#define getMovePromoted(move) ((move & 0xf0000) >> 16)
#define getMoveCapture(move) (move & 0x100000)
#define getMoveDouble(move) (move & 0x200000)
#define getMoveEnpassant(move) (move & 0x400000)
#define getMoveCastling(move) (move & 0x800000)

#define copyBoard() \
    U64 bitboardsCopy[12], occupancyCopy[3], hashKeyCopy = board->hashKey; \
    int sideCopy = board->side, enpassantCopy = board->enPassant, castleCopy = board->castling, fiftyMoveCopy = board->fiftyMove; \
    memcpy(bitboardsCopy, board->bitboard, 96); \
    memcpy(occupancyCopy, board->occupancy, 24); 

#define takeBack() \
    memcpy(board->bitboard, bitboardsCopy, 96); \
    memcpy(board->occupancy, occupancyCopy, 24); \
    board->side = sideCopy, board->enPassant = enpassantCopy, board->castling = castleCopy, board->fiftyMove = fiftyMoveCopy; \
    board->hashKey = hashKeyCopy;

// macros
#define setBit(bitboard, square) (bitboard |= (1ULL << square))
#define getBit(bitboard, square) (bitboard & (1ULL << square))
#define popBit(bitboard, square) (bitboard &= ~(1ULL << square))

// enums
enum Square {
	A8, B8, C8, D8, E8, F8, G8, H8,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A1, B1, C1, D1, E1, F1, G1, H1, NO_SQR
};

enum Piece { P, N, B, R, Q, K, p, n, b, r, q, k };

enum Side { WHITE, BLACK, BOTH };

enum Castling { WK = 1, WQ = 2, BK = 4, BQ = 8 };

enum { ALL_MOVES, ONLY_CAPTURES };

