#include <stdio.h>
#include "board.h"
#include "search.h"
#include "movegen.h"
#include "zobrist.h"
#include "uci.h"
#include "defs.h"
#include "eval.h"
#include "nnue_eval.h"

void initAll()
{
	initLeaperMasks();
	initSliderMasks();
	initZobrist();
	clearHashTable();
	init_nnue("nn-62ef826d1a6d.nnue");
}

int main()
{
	Board board[1];
	SearchInfo info[1];

	initAll();
	uciLoop(board, info);

	//parseFen(board, startPosition);
	//printBoard(board);

	//printf("Score FEN: %d\n", evaluate_fen_nnue(startPosition));
	//searchPosition(board, info, 12);

	return 0;
}
