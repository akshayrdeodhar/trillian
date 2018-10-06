#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "board.h"
#include "pieces.h"
#include "display.h"
#include "moves.h"

#define DEFAULT_PATH "../dat/default.fen"

void interface_board_set(chessboard *board, chesset *set); /* sets 'indexes' in board, initialize pieces in chesset */


int main(int argc, char *argv[]) {

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
	/*show_set(set); */
	verify_interface(board, set);

	calculate_all(&set, board);

	display(board, MOVES_MODE);

	verify_calculation(set, board);

	return 0;
}
