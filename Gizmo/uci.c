#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "uci.h"
#include "search.h"
#include "movegen.h"

int parseMove(Board* board, char *move_string)
{
    MoveList move_list[1];
    generateMoves(board, move_list);
    
    int source_square = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    int target_square = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;
    
    for (int move_count = 0; move_count < move_list->count; move_count++)
    {
        int move = move_list->moves[move_count];

        if (source_square == getMoveSource(move) && target_square == getMoveTarget(move))
        {
            int promoted_piece = getMovePromoted(move);

            if (promoted_piece)
            {
                if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
                    return move;
                else if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
                    return move;
                else if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
                    return move;
                else if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
                    return move;

                continue;
            }

            return move;
        }
    }

    return 0;
}


void parsePosition(Board* board, char *command)
{
    command += 9;

    char *current_char = command;

    if (strncmp(command, "startpos", 8) == 0)
        parseFen(board, startPosition);
    else
    {
        current_char = strstr(command, "fen");

        if (current_char == NULL)
            parseFen(board, startPosition);
        else
        {
            current_char += 4;
            parseFen(board, current_char);
        }
    }

    current_char = strstr(command, "moves");

    if (current_char != NULL)
    {
        current_char += 6;

        while(*current_char)
        {
            int move = parseMove(board, current_char);
            
            if (move == 0)
                break;

			board->repTable[++board->repIndex] = board->hashKey;
            
            makeMove(board, move);
            
            while (*current_char && *current_char != ' ') current_char++;

            current_char++;
        }        
    }

    printBoard(board);
}

void parseGo(Board* board, SearchInfo* info, char *command)
{
	info->quit = 0;
    info->movestogo = 30;
    info->movetime = -1;
    info->time = -1;
    info->inc = 0;
    info->starttime = 0;
    info->stoptime = 0;
    info->timeset = 0;
    info->stopped = 0;

    int depth = -1;

    char *argument = NULL;

    if ((argument = strstr(command,"infinite"))) {}

    if ((argument = strstr(command,"binc")) && board->side == BLACK)
        info->inc = atoi(argument + 5);

    if ((argument = strstr(command,"winc")) && board->side == WHITE)
        info->inc = atoi(argument + 5);

    if ((argument = strstr(command,"wtime")) && board->side == WHITE)
        info->time = atoi(argument + 6);
        
    if ((argument = strstr(command,"btime")) && board->side == BLACK)
        info->time = atoi(argument + 6);

    if ((argument = strstr(command,"movestogo")))
        info->movestogo = atoi(argument + 10);

    if ((argument = strstr(command,"movetime")))
        info->movetime = atoi(argument + 9);

    if ((argument = strstr(command,"depth")))
        depth = atoi(argument + 6);

    if(info->movetime != -1)
    {
        info->time = info->movetime;
        info->movestogo = 1;
    }

    info->starttime = getTimeMS();

    depth = depth;

    if(info->time != -1)
    {
        info->timeset = 1;
        info->time /= info->movestogo;
        if (info->time > 1500) info->time -= 50;
        info->stoptime = info->starttime + info->time + info->inc;
        if (info->time < 1500 && info->inc && depth == 64) info->stoptime = info->starttime + info->inc - 50;
    }

    if(depth == -1)
        depth = 64;

    printf("time: %d  start: %u  stop: %u  depth: %d  timeset:%d\n",
            info->time, info->starttime, info->stoptime, depth, info->timeset);

    searchPosition(board, info, depth);
}

void uciLoop(Board* board, SearchInfo* info)
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char input[2000];

    printf("id name Gizmo\n");
    printf("id author Lancer\n");
    printf("uciok\n");

    while (1)
    {
        memset(input, 0, sizeof(input));

        fflush(stdout);

        if (!fgets(input, 2000, stdin))
            continue;
        if (input[0] == '\n')
            continue;
        if (strncmp(input, "isready", 7) == 0)
        {
            printf("readyok\n");
            continue;
        }
        else if (strncmp(input, "position", 8) == 0)
        {
            parsePosition(board, input);
            clearHashTable();
        }
        else if (strncmp(input, "ucinewgame", 10) == 0)
        {
            parsePosition(board, "position startpos");
            clearHashTable();
        }
        else if (strncmp(input, "go", 2) == 0)
            parseGo(board, info, input);
        else if (strncmp(input, "quit", 4) == 0)
            break;
        else if (strncmp(input, "uci", 3) == 0)
        {
            printf("id name Gizmo\n");
            printf("id author Lancer\n");
            printf("uciok\n");
        }
    }
}

int input_waiting()
{
	#ifndef WIN32
        fd_set readfds;
        struct timeval tv;
        FD_ZERO (&readfds);
        FD_SET (fileno(stdin), &readfds);
        tv.tv_sec=0; tv.tv_usec=0;
        select(16, &readfds, 0, 0, &tv);

        return (FD_ISSET(fileno(stdin), &readfds));
    #else
        static int init = 0, pipe;
        static HANDLE inh;
        DWORD dw;

        if (!init)
        {
            init = 1;
            inh = GetStdHandle(STD_INPUT_HANDLE);
            pipe = !GetConsoleMode(inh, &dw);
            if (!pipe)
            {
                SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
                FlushConsoleInputBuffer(inh);
            }
        }
        
        if (pipe)
        {
           if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
           return dw;
        }
        
        else
        {
           GetNumberOfConsoleInputEvents(inh, &dw);
           return dw <= 1 ? 0 : dw;
        }
    #endif
}


// read GUI/user input
void read_input(Board* board, SearchInfo* info)
{
	int bytes;

	char input[256] = "", * endc;

	if (input_waiting())
	{
		info->stopped = 1;

		do
			bytes = read(fileno(stdin), input, 256);
		while (bytes < 0);

		endc = strchr(input, '\n');

		if (endc) *endc = 0;

		if (strlen(input) > 0)
		{
			if (!strncmp(input, "quit", 4))
				info->quit = 1;
			else if (!strncmp(input, "stop", 4))
				info->quit = 1;
		}
	}
}

void communicate(Board* board, SearchInfo* info) {
	if (info->timeset == 1 && getTimeMS() > info->stoptime) {
		info->stopped = 1;
	}

	read_input(board, info);
}
