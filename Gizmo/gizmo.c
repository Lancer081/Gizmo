#include <stdio.h>
#include "board.h"
#include "search.h"
#include "movegen.h"
#include "zobrist.h"
#include "uci.h"
#include "defs.h"

void initAll()
{
	initLeaperMasks();
	initSliderMasks();
	initEvalMasks();
	initZobrist();
	clearHashTable();
}

int main()
{
	initAll();

	Board board[1];
	SearchInfo info[1];

	uciLoop(board, info);

	//parseFen(board, startPosition);
	//searchPosition(board, info, 12);

	return 0;
}
