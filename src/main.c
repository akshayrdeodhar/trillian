#include <stdio.h>
#include <string.h>

#include "board.h"
#include "pieces.h"
#include "display.h"

void interface_board_set(chessboard *board, chesset *set); /* sets 'indexes' in board, initialize pieces in chesset */


int main(void) {
	int n;
	char string[128];
	fgets(string, 128, stdin);
	n = strlen(string);
	if (string[n - 1] == '\n') {
		string[n - 1] = '\0';
	}
	chessboard board;
	chesset set;
	if (fenstring_to_board(&board, string)) {
		printf("Impossible:\n");
		display(board);
	}
	else {
		display(board);
		return 1;
	}

	interface_board_set(&board, &set);
	show_set(set);
	verify_interface(board, set);

	return 0;
}
