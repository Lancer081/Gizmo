#include <stdio.h>
#include "movegen.h"
#include "board.h"
#include "zobrist.h"
#include "search.h"

const U64 NOT_A_FILE = 18374403900871474942ULL;
const U64 NOT_H_FILE = 9187201950435737471ULL;
const U64 NOT_HG_FILE = 4557430888798830399ULL;
const U64 NOT_AB_FILE = 18229723555195321596ULL;

// bishop relevant occupancy bit count for every square on board
const int bishopRelevantBits[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6
};

// rook relevant occupancy bit count for every square on board
const int rookRelevantBits[64] = {
	12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12
};

// rook magic numbers
U64 rookMagicNumbers[64] = {
	0x8a80104000800020ULL,
	0x140002000100040ULL,
	0x2801880a0017001ULL,
	0x100081001000420ULL,
	0x200020010080420ULL,
	0x3001c0002010008ULL,
	0x8480008002000100ULL,
	0x2080088004402900ULL,
	0x800098204000ULL,
	0x2024401000200040ULL,
	0x100802000801000ULL,
	0x120800800801000ULL,
	0x208808088000400ULL,
	0x2802200800400ULL,
	0x2200800100020080ULL,
	0x801000060821100ULL,
	0x80044006422000ULL,
	0x100808020004000ULL,
	0x12108a0010204200ULL,
	0x140848010000802ULL,
	0x481828014002800ULL,
	0x8094004002004100ULL,
	0x4010040010010802ULL,
	0x20008806104ULL,
	0x100400080208000ULL,
	0x2040002120081000ULL,
	0x21200680100081ULL,
	0x20100080080080ULL,
	0x2000a00200410ULL,
	0x20080800400ULL,
	0x80088400100102ULL,
	0x80004600042881ULL,
	0x4040008040800020ULL,
	0x440003000200801ULL,
	0x4200011004500ULL,
	0x188020010100100ULL,
	0x14800401802800ULL,
	0x2080040080800200ULL,
	0x124080204001001ULL,
	0x200046502000484ULL,
	0x480400080088020ULL,
	0x1000422010034000ULL,
	0x30200100110040ULL,
	0x100021010009ULL,
	0x2002080100110004ULL,
	0x202008004008002ULL,
	0x20020004010100ULL,
	0x2048440040820001ULL,
	0x101002200408200ULL,
	0x40802000401080ULL,
	0x4008142004410100ULL,
	0x2060820c0120200ULL,
	0x1001004080100ULL,
	0x20c020080040080ULL,
	0x2935610830022400ULL,
	0x44440041009200ULL,
	0x280001040802101ULL,
	0x2100190040002085ULL,
	0x80c0084100102001ULL,
	0x4024081001000421ULL,
	0x20030a0244872ULL,
	0x12001008414402ULL,
	0x2006104900a0804ULL,
	0x1004081002402ULL
};

// bishop magic numbers
U64 bishopMagicNumbers[64] = {
	0x40040844404084ULL,
	0x2004208a004208ULL,
	0x10190041080202ULL,
	0x108060845042010ULL,
	0x581104180800210ULL,
	0x2112080446200010ULL,
	0x1080820820060210ULL,
	0x3c0808410220200ULL,
	0x4050404440404ULL,
	0x21001420088ULL,
	0x24d0080801082102ULL,
	0x1020a0a020400ULL,
	0x40308200402ULL,
	0x4011002100800ULL,
	0x401484104104005ULL,
	0x801010402020200ULL,
	0x400210c3880100ULL,
	0x404022024108200ULL,
	0x810018200204102ULL,
	0x4002801a02003ULL,
	0x85040820080400ULL,
	0x810102c808880400ULL,
	0xe900410884800ULL,
	0x8002020480840102ULL,
	0x220200865090201ULL,
	0x2010100a02021202ULL,
	0x152048408022401ULL,
	0x20080002081110ULL,
	0x4001001021004000ULL,
	0x800040400a011002ULL,
	0xe4004081011002ULL,
	0x1c004001012080ULL,
	0x8004200962a00220ULL,
	0x8422100208500202ULL,
	0x2000402200300c08ULL,
	0x8646020080080080ULL,
	0x80020a0200100808ULL,
	0x2010004880111000ULL,
	0x623000a080011400ULL,
	0x42008c0340209202ULL,
	0x209188240001000ULL,
	0x400408a884001800ULL,
	0x110400a6080400ULL,
	0x1840060a44020800ULL,
	0x90080104000041ULL,
	0x201011000808101ULL,
	0x1a2208080504f080ULL,
	0x8012020600211212ULL,
	0x500861011240000ULL,
	0x180806108200800ULL,
	0x4000020e01040044ULL,
	0x300000261044000aULL,
	0x802241102020002ULL,
	0x20906061210001ULL,
	0x5a84841004010310ULL,
	0x4010801011c04ULL,
	0xa010109502200ULL,
	0x4a02012000ULL,
	0x500201010098b028ULL,
	0x8040002811040900ULL,
	0x28000010020204ULL,
	0x6000020202d0240ULL,
	0x8918844842082200ULL,
	0x4010011029020020ULL
};

inline int countBits(U64 bitboard)
{
	int count = 0;
	
	while (bitboard)
	{
		count++;
		bitboard &= bitboard - 1;
	}
	
	return count;
}

inline int getLSB(U64 bitboard)
{
	if (bitboard)
		return countBits((bitboard & -bitboard) - 1);
	else
		return -1;
}

static inline void addMove(MoveList* moves, int move)
{
	moves->moves[moves->count++] = move;
}

inline void generateMoves(Board* board, MoveList* moves)
{
	U64 bitboard, attacks;
	
	int fromSquare, toSquare;
	
	moves->count = 0;
	
	for (int piece = P; piece <= k; piece++)
	{
		bitboard = board->bitboard[piece];
	
		if (board->side == WHITE)
		{
			if (piece == P)
			{
				while (bitboard)
				{
					fromSquare = getLSB(bitboard);
					toSquare = fromSquare - 8;
					
					if (!getBit(board->occupancy[BOTH], toSquare))
					{
						if (fromSquare >= A7 && fromSquare <= H7)
						{
							addMove(moves, encodeMove(fromSquare, toSquare, piece, Q, 0, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, B, 0, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, N, 0, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, R, 0, 0, 0, 0));
						}
						else
						{
							addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 0, 0, 0, 0));
						
							if ((fromSquare >= A2 && fromSquare <= H2) && !getBit(board->occupancy[BOTH], toSquare - 8))
								addMove(moves, encodeMove(fromSquare, toSquare - 8, piece, 0, 0, 1, 0, 0));
						}
					}
					
					attacks = pawnAttacks[board->side][fromSquare] & board->occupancy[BLACK];

					while (attacks)
					{
						toSquare = getLSB(attacks);

						if (fromSquare >= A7 && fromSquare <= H7)
						{
							addMove(moves, encodeMove(fromSquare, toSquare, piece, Q, 1, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, B, 1, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, N, 1, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, R, 1, 0, 0, 0));
						}
						else
							addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));
							
						popBit(attacks, toSquare);
					}

                    if (board->enPassant != NO_SQR)
                    {
                        U64 enpassantAttacks = pawnAttacks[board->side][fromSquare] & (1ULL << board->enPassant);

                        if (enpassantAttacks)
                        {
                            int targetEnpassant = getLSB(enpassantAttacks);
                            addMove(moves, encodeMove(fromSquare, targetEnpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }

					popBit(bitboard, fromSquare);
				}
			}
			else if (piece == K)
			{
				if (board->castling & WK)
				{
					if (!getBit(board->occupancy[BOTH], F1) && !getBit(board->occupancy[BOTH], G1))
					{
						if (!isSquareAttacked(board, E1, BLACK) && !isSquareAttacked(board, F1, BLACK))
							addMove(moves, encodeMove(E1, G1, piece, 0, 0, 0, 0, 1));
					}
				}
				if (board->castling & WQ)
				{
					if (!getBit(board->occupancy[BOTH], D1) && !getBit(board->occupancy[BOTH], C1) && !getBit(board->occupancy[BOTH], B1))
					{
						if (!isSquareAttacked(board, E1, BLACK) && !isSquareAttacked(board, D1, BLACK))
							addMove(moves, encodeMove(E1, C1, piece, 0, 0, 0, 0, 1));
					}
				}
			}
		}
		else
		{
			if (piece == p)
			{
				while (bitboard)
				{
					fromSquare = getLSB(bitboard);
					toSquare = fromSquare + 8;
					
					if (!getBit(board->occupancy[BOTH], toSquare))
					{
						if (fromSquare >= A2 && fromSquare <= H2)
						{
							addMove(moves, encodeMove(fromSquare, toSquare, piece, q, 0, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, b, 0, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, n, 0, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, r, 0, 0, 0, 0));
						}
						else
						{	
							addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 0, 0, 0, 0));
						
							if ((fromSquare >= A7 && fromSquare <= H7) && !getBit(board->occupancy[BOTH], toSquare + 8))
								addMove(moves, encodeMove(fromSquare, toSquare + 8, piece, 0, 0, 1, 0, 0));
						}
					}
					
					attacks = pawnAttacks[board->side][fromSquare] & board->occupancy[WHITE];

					while (attacks)
					{
						toSquare = getLSB(attacks);
					
						if (fromSquare >= A2 && fromSquare <= H2)
						{
							addMove(moves, encodeMove(fromSquare, toSquare, piece, q, 1, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, b, 1, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, n, 1, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, r, 1, 0, 0, 0));
						}
						else
							addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));
							
						popBit(attacks, toSquare);
					}

					if (board->enPassant != NO_SQR)
                    {
                        U64 enpassantAttacks = pawnAttacks[board->side][fromSquare] & (1ULL << board->enPassant);

                        if (enpassantAttacks)
                        {
                            int targetEnpassant = getLSB(enpassantAttacks);
                            addMove(moves, encodeMove(fromSquare, targetEnpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }
					
					popBit(bitboard, fromSquare);
				}
			}
			else if (piece == k)
			{
				if (board->castling & BK)
				{
					if (!getBit(board->occupancy[BOTH], F8) && !getBit(board->occupancy[BOTH], G8))
					{
						if (!isSquareAttacked(board, E8, WHITE) && !isSquareAttacked(board, F8, WHITE))
							addMove(moves, encodeMove(E8, G8, piece, 0, 0, 0, 0, 1));
					}
				}
				if (board->castling & BQ)
				{
					if (!getBit(board->occupancy[BOTH], D8) && !getBit(board->occupancy[BOTH], C8) && !getBit(board->occupancy[BOTH], B8))
					{
						if (!isSquareAttacked(board, E8, WHITE) && !isSquareAttacked(board, D8, WHITE))
							addMove(moves, encodeMove(E8, C8, piece, 0, 0, 0, 0, 1));
					}
				}
			}
		}

		if ((board->side == WHITE) ? piece == N : piece == n)
		{
			while(bitboard)
			{
				fromSquare = getLSB(bitboard);
				attacks = knightAttacks[fromSquare] & ((board->side == WHITE) ? ~board->occupancy[WHITE] : ~board->occupancy[BLACK]);

				while (attacks)
				{
					toSquare = getLSB(attacks);

					if (!getBit((board->side == WHITE)? board->occupancy[BLACK] : board->occupancy[WHITE], toSquare))
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 0, 0, 0, 0));
					else
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));

					popBit(attacks, toSquare);
				}

				popBit(bitboard, fromSquare);
			}
		}

		if ((board->side == WHITE) ? piece == B : piece == b)
		{
			while(bitboard)
			{
				fromSquare = getLSB(bitboard);
				attacks = getBishopAttacks(fromSquare, board->occupancy[BOTH]) & ((board->side == WHITE) ? ~board->occupancy[WHITE] : ~board->occupancy[BLACK]);

				while (attacks)
				{
					toSquare = getLSB(attacks);

					if (!getBit(board->side == WHITE ? board->occupancy[BLACK] : board->occupancy[WHITE], toSquare))
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 0, 0, 0, 0));
					else
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));

					popBit(attacks, toSquare);
				}

				popBit(bitboard, fromSquare);
			}
		}

		if ((board->side == WHITE) ? piece == R : piece == r)
		{
			while(bitboard)
			{
				fromSquare = getLSB(bitboard);
				attacks = getRookAttacks(fromSquare, board->occupancy[BOTH]) & ((board->side == WHITE) ? ~board->occupancy[WHITE] : ~board->occupancy[BLACK]);

				while (attacks)
				{
					toSquare = getLSB(attacks);

					if (!getBit(board->side == WHITE ? board->occupancy[BLACK] : board->occupancy[WHITE], toSquare))
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 0, 0, 0, 0));
					else
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));

					popBit(attacks, toSquare);
				}

				popBit(bitboard, fromSquare);
			}
		}

		if ((board->side == WHITE) ? piece == Q : piece == q)
		{
			while(bitboard)
			{
				fromSquare = getLSB(bitboard);
				attacks = getQueenAttacks(fromSquare, board->occupancy[BOTH]) & ((board->side == WHITE) ? ~board->occupancy[WHITE] : ~board->occupancy[BLACK]);

				while (attacks)
				{
					toSquare = getLSB(attacks);

					if (!getBit(board->side == WHITE ? board->occupancy[BLACK] : board->occupancy[WHITE], toSquare))
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 0, 0, 0, 0));
					else
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));

					popBit(attacks, toSquare);
				}

				popBit(bitboard, fromSquare);
			}
		}

		if ((board->side == WHITE) ? piece == K : piece == k)
		{
			while(bitboard)
			{
				fromSquare = getLSB(bitboard);
				attacks = kingAttacks[fromSquare] & ((board->side == WHITE) ? ~board->occupancy[WHITE] : ~board->occupancy[BLACK]);

				while (attacks)
				{
					toSquare = getLSB(attacks);

					if (!getBit(board->side == WHITE ? board->occupancy[BLACK] : board->occupancy[WHITE], toSquare))
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 0, 0, 0, 0));
					else
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));

					popBit(attacks, toSquare);
				}

				popBit(bitboard, fromSquare);
			}
		}
	}
}

inline void generateCaptures(Board* board, MoveList* moves)
{
	U64 bitboard, attacks;
	
	int fromSquare, toSquare;
	
	moves->count = 0;
	
	for (int piece = P; piece <= k; piece++)
	{
		bitboard = board->bitboard[piece];
	
		if (board->side == WHITE)
		{
			if (piece == P)
			{
				while (bitboard)
				{
					fromSquare = getLSB(bitboard);
					attacks = pawnAttacks[board->side][fromSquare] & board->occupancy[BLACK];

					while (attacks)
					{
						toSquare = getLSB(attacks);

						if (fromSquare >= A7 && fromSquare <= H7)
						{
							addMove(moves, encodeMove(fromSquare, toSquare, piece, Q, 1, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, B, 1, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, N, 1, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, R, 1, 0, 0, 0));
						}
						else
							addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));
							
						popBit(attacks, toSquare);
					}

                    if (board->enPassant != NO_SQR)
                    {
                        U64 enpassantAttacks = pawnAttacks[board->side][fromSquare] & (1ULL << board->enPassant);

                        if (enpassantAttacks)
                        {
                            int targetEnpassant = getLSB(enpassantAttacks);
                            addMove(moves, encodeMove(fromSquare, targetEnpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }

					popBit(bitboard, fromSquare);
				}
			}
		}
		else
		{
			if (piece == p)
			{
				while (bitboard)
				{
					fromSquare = getLSB(bitboard);
					attacks = pawnAttacks[board->side][fromSquare] & board->occupancy[WHITE];

					while (attacks)
					{
						toSquare = getLSB(attacks);
					
						if (fromSquare >= A2 && fromSquare <= H2)
						{
							addMove(moves, encodeMove(fromSquare, toSquare, piece, q, 1, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, b, 1, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, n, 1, 0, 0, 0));
							addMove(moves, encodeMove(fromSquare, toSquare, piece, r, 1, 0, 0, 0));
						}
						else
							addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));
							
						popBit(attacks, toSquare);
					}

					if (board->enPassant != NO_SQR)
                    {
                        U64 enpassantAttacks = pawnAttacks[board->side][fromSquare] & (1ULL << board->enPassant);

                        if (enpassantAttacks)
                        {
                            int targetEnpassant = getLSB(enpassantAttacks);
                            addMove(moves, encodeMove(fromSquare, targetEnpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }
					
					popBit(bitboard, fromSquare);
				}
			}
		}

		if ((board->side == WHITE) ? piece == N : piece == n)
		{
			while(bitboard)
			{
				fromSquare = getLSB(bitboard);
				attacks = knightAttacks[fromSquare] & ((board->side == WHITE) ? ~board->occupancy[WHITE] : ~board->occupancy[BLACK]);

				while (attacks)
				{
					toSquare = getLSB(attacks);

					if (getBit((board->side == WHITE)? board->occupancy[BLACK] : board->occupancy[WHITE], toSquare))
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));

					popBit(attacks, toSquare);
				}

				popBit(bitboard, fromSquare);
			}
		}

		if ((board->side == WHITE) ? piece == B : piece == b)
		{
			while(bitboard)
			{
				fromSquare = getLSB(bitboard);
				attacks = getBishopAttacks(fromSquare, board->occupancy[BOTH]) & ((board->side == WHITE) ? ~board->occupancy[WHITE] : ~board->occupancy[BLACK]);

				while (attacks)
				{
					toSquare = getLSB(attacks);

					if (getBit(board->side == WHITE ? board->occupancy[BLACK] : board->occupancy[WHITE], toSquare))
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));

					popBit(attacks, toSquare);
				}

				popBit(bitboard, fromSquare);
			}
		}

		if ((board->side == WHITE) ? piece == R : piece == r)
		{
			while(bitboard)
			{
				fromSquare = getLSB(bitboard);
				attacks = getRookAttacks(fromSquare, board->occupancy[BOTH]) & ((board->side == WHITE) ? ~board->occupancy[WHITE] : ~board->occupancy[BLACK]);

				while (attacks)
				{
					toSquare = getLSB(attacks);

					if (getBit(board->side == WHITE ? board->occupancy[BLACK] : board->occupancy[WHITE], toSquare))
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));

					popBit(attacks, toSquare);
				}

				popBit(bitboard, fromSquare);
			}
		}

		if ((board->side == WHITE) ? piece == Q : piece == q)
		{
			while(bitboard)
			{
				fromSquare = getLSB(bitboard);
				attacks = getQueenAttacks(fromSquare, board->occupancy[BOTH]) & ((board->side == WHITE) ? ~board->occupancy[WHITE] : ~board->occupancy[BLACK]);

				while (attacks)
				{
					toSquare = getLSB(attacks);

					if (getBit(board->side == WHITE ? board->occupancy[BLACK] : board->occupancy[WHITE], toSquare))
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));

					popBit(attacks, toSquare);
				}

				popBit(bitboard, fromSquare);
			}
		}

		if ((board->side == WHITE) ? piece == K : piece == k)
		{
			while(bitboard)
			{
				fromSquare = getLSB(bitboard);
				attacks = kingAttacks[fromSquare] & ((board->side == WHITE) ? ~board->occupancy[WHITE] : ~board->occupancy[BLACK]);

				while (attacks)
				{
					toSquare = getLSB(attacks);

					if (getBit(board->side == WHITE ? board->occupancy[BLACK] : board->occupancy[WHITE], toSquare))
						addMove(moves, encodeMove(fromSquare, toSquare, piece, 0, 1, 0, 0, 0));

					popBit(attacks, toSquare);
				}

				popBit(bitboard, fromSquare);
			}
		}
	}
}

U64 setOccupancy(int index, int bitsInMask, U64 attackMask)
{
	U64 occupancy = 0ULL;

	for (int count = 0; count < bitsInMask; count++)
	{
		int square = getLSB(attackMask);

		popBit(attackMask, square);

		if (index & (1 << count))
			occupancy |= (1ULL << square);
	}

	return occupancy;
}

U64 maskPawnAttacks(int side, int square)
{
	U64 attacks = 0ULL;
	U64 bitboard = 0ULL;
	
	setBit(bitboard, square);
	
	if (side == WHITE)
	{
		attacks |= (bitboard >> 7) & NOT_A_FILE;
		attacks |= (bitboard >> 9) & NOT_H_FILE;
	}
	else
	{
		attacks |= (bitboard << 7) & NOT_H_FILE;
		attacks |= (bitboard << 9) & NOT_A_FILE;
	}
	
	return attacks;
}

U64 maskKnightAttacks(int square)
{
	U64 bitboard = 0ULL;
	U64 attacks = 0ULL;

	setBit(bitboard, square);

	if ((bitboard >> 15) & NOT_A_FILE) attacks |= bitboard >> 15;
	if ((bitboard >> 17) & NOT_H_FILE) attacks |= bitboard >> 17;
	if ((bitboard >> 10) & NOT_HG_FILE) attacks |= bitboard >> 10;
	if ((bitboard >> 6) & NOT_AB_FILE) attacks |= bitboard >> 6;
	if ((bitboard << 15) & NOT_H_FILE) attacks |= bitboard << 15;
	if ((bitboard << 17) & NOT_A_FILE) attacks |= bitboard << 17;
	if ((bitboard << 10) & NOT_AB_FILE) attacks |= bitboard << 10;
	if ((bitboard << 6) & NOT_HG_FILE) attacks |= bitboard << 6;

	return attacks;
}

U64 maskKingAttacks(int square)
{
	U64 bitboard = 0ULL;
	U64 attacks = 0ULL;

	setBit(bitboard, square);

	if ((bitboard >> 7) & NOT_A_FILE) attacks |= bitboard >> 7;
	if (bitboard >> 8) attacks |= bitboard >> 8;
	if ((bitboard >> 9) & NOT_H_FILE) attacks |= bitboard >> 9;
	if ((bitboard >> 1) & NOT_H_FILE) attacks |= bitboard >> 1;
	if ((bitboard << 7) & NOT_H_FILE) attacks |= bitboard << 7;
	if (bitboard << 8) attacks |= bitboard << 8;
	if ((bitboard << 9) & NOT_A_FILE) attacks |= bitboard << 9;
	if ((bitboard << 1) & NOT_A_FILE) attacks |= bitboard << 1;

	return attacks;
}

U64 maskBishopAttacks(int square)
{
	U64 attacks = 0ULL;

	int r, f;

	int tr = square / 8;
	int tf = square % 8;

	for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
	for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
	for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
	for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));

	return attacks;
}

U64 maskRookAttacks(int square)
{
	U64 attacks = 0ULL;

	int r, f;

	int tr = square / 8;
	int tf = square % 8;

	for (r = tr + 1; r <= 6; r++) attacks |= 1ULL << (r * 8 + tf);
	for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
	for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
	for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));

	return attacks;
}

U64 bishopAttacksOTF(int sqr, U64 blockers)
{
	U64 attacks = 0ULL;

	int r, f;

	int tr = sqr / 8;
	int tf = sqr % 8;

	for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
	{
		attacks |= (1ULL << (r * 8 + f));
		if ((1ULL << (r * 8 + f)) & blockers) break;
	}
	for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
	{
		attacks |= (1ULL << (r * 8 + f));
		if ((1ULL << (r * 8 + f)) & blockers) break;
	}
	for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
	{
		attacks |= (1ULL << (r * 8 + f));
		if ((1ULL << (r * 8 + f)) & blockers) break;
	}
	for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
	{
		attacks |= (1ULL << (r * 8 + f));
		if ((1ULL << (r * 8 + f)) & blockers) break;
	}

	return attacks;
}

U64 rookAttacksOTF(int square, U64 blockers)
{
	U64 attacks = 0ULL;

	int r, f;

	int tr = square / 8;
	int tf = square % 8;

	for (r = tr + 1; r <= 7; r++)
	{
		attacks |= (1ULL << (r * 8 + tf));
		if ((1ULL << (r * 8 + tf)) & blockers) break;
	}

	for (r = tr - 1; r >= 0; r--)
	{
		attacks |= (1ULL << (r * 8 + tf));
		if ((1ULL << (r * 8 + tf)) & blockers) break;
	}

	for (f = tf + 1; f <= 7; f++)
	{
		attacks |= (1ULL << (tr * 8 + f));
		if ((1ULL << (tr * 8 + f)) & blockers) break;
	}

	for (f = tf - 1; f >= 0; f--)
	{
		attacks |= (1ULL << (tr * 8 + f));
		if ((1ULL << (tr * 8 + f)) & blockers) break;
	}

	return attacks;
}

inline U64 getRookAttacks(int sqr, U64 occupancy)
{
	occupancy &= rookMasks[sqr];
	occupancy *= rookMagicNumbers[sqr];
	occupancy >>= 64 - rookRelevantBits[sqr];

	return rookAttacks[sqr][occupancy];
}

inline U64 getBishopAttacks(int sqr, U64 occupancy)
{
	occupancy &= bishopMasks[sqr];
	occupancy *= bishopMagicNumbers[sqr];
	occupancy >>= 64 - bishopRelevantBits[sqr];

	return bishopAttacks[sqr][occupancy];
}

inline U64 getQueenAttacks(int sqr, U64 occupancy) { return getBishopAttacks(sqr, occupancy) | getRookAttacks(sqr, occupancy); }

void initLeaperMasks()
{
	for (int square = 0; square < SQUARE_NUM; square++)
	{
		pawnAttacks[WHITE][square] = maskPawnAttacks(WHITE, square);
		pawnAttacks[BLACK][square] = maskPawnAttacks(BLACK, square);
		knightAttacks[square] = maskKnightAttacks(square);
		kingAttacks[square] = maskKingAttacks(square);
	}
}

void initSliderMasks()
{
	U64 attackMask;
	
	int relevantBitsCount;
	int occupancyIndices;

	for (int sqr = 0; sqr < SQUARE_NUM; sqr++)
	{
		bishopMasks[sqr] = maskBishopAttacks(sqr);
		rookMasks[sqr] = maskRookAttacks(sqr);

		attackMask = bishopMasks[sqr];
		relevantBitsCount = countBits(attackMask);
		occupancyIndices = (1 << relevantBitsCount);

		for (int index = 0; index < occupancyIndices; index++)
		{
			U64 occupancy = setOccupancy(index, relevantBitsCount, attackMask);
			int magicIndex = (occupancy * bishopMagicNumbers[sqr]) >> (64 - bishopRelevantBits[sqr]);
			bishopAttacks[sqr][magicIndex] = bishopAttacksOTF(sqr, occupancy);
		}
		
		attackMask = rookMasks[sqr];
		relevantBitsCount = countBits(attackMask);
		occupancyIndices = (1 << relevantBitsCount);

		for (int index = 0; index < occupancyIndices; index++)
		{
			U64 occupancy = setOccupancy(index, relevantBitsCount, attackMask);
			int magicIndex = (occupancy * rookMagicNumbers[sqr]) >> (64 - rookRelevantBits[sqr]);
			rookAttacks[sqr][magicIndex] = rookAttacksOTF(sqr, occupancy);
		}
	}
}
