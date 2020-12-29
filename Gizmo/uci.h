#pragma once

#include "board.h"

int parseMove(Board* board, char *move_string);
void parsePosition(Board* board, char *command);
void parseGo(Board* board, SearchInfo* info, char *command);
void uciLoop(Board* board, SearchInfo* info);

int input_waiting();
void read_input(Board* board, SearchInfo* info);
void communicate(Board* board, SearchInfo* info);