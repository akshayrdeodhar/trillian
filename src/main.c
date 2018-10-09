#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "board.h"
#include "pieces.h"
#include "display.h"
#include "moves.h"
#include "input.h"

#define DEFAULT_PATH "../dat/default.fen"
#define DEBUG (0)
#define DEBUG_INTERFACE 1
#define DEBUG_CALCULATE 2

int main(int argc, char *argv[]) {

	char command[32];
	move mv;

	if (argc > 2) {
		fprintf(stderr, "usage: ./chess <file.fen>\n");
		return EINVAL;
	}

	FILE *fp;

	if (argc == 1) {
		fp = fopen(DEFAULT_PATH, "r");
		if (fp == NULL) {
			perror("invalid default file");
			return errno;
		}
	}
	else if (argc == 2) {
		fp = fopen(argv[1], "r");
		if(fp == NULL) {
			perror("invalid filename");
			return errno;
		}
	}


	int n;
	char string[128];
	fgets(string, 128, fp);
	n = strlen(string);
	if (string[n - 1] == '\n') {
		string[n - 1] = '\0';
	}
	chessboard board;
	chesset set;
	if (fenstring_to_board(&board, string)) {
		printf("Impossible:\n");
		display(board, READ_MODE);
	}
	else {
		display(board, READ_MODE);
		return 1;
	}

	interface_board_set(&board, &set);

#if (DEBUG & DEBUG_INTERFACE)
	show_set(set); 

	verify_interface(board, set);
#endif

	calculate_all(&set, board);

	display(board, MOVES_MODE);

#if (DEBUG & DEBUG_CALCULATE)
	verify_calculation(set, board);
#endif

	while(1) {
		display(board, MOVES_MODE);
		printf("COMMAND:");
		readline(command, 32);
		if (!(strcmp(command, "quit"))) {
			verify_calculation(set, board);
			return 0;
		}
		else {
			mv = extract_move(command);
			print_move(mv);
			make_move(&board, &set, mv);
			verify_interface(board, set);
			show_set(set);
			update_sliding_pieces(board, &set, mv);
#if (DEBUG & DEBUG_INTERFACE)
	show_set(set); 

	verify_interface(board, set);
#endif


#if (DEBUG & DEBUG_CALCULATE)
			verify_calculation(set, board);
#endif
		}
	}
	return 0;
}
