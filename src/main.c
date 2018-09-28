#include <stdio.h>
#include "display.h"

int main(void) {
	char string[128];
	fgets(string, 128, stdin);
	chessboard board;
	if (fenstring_to_board(&board, string)) {
		printf("Impossible:\n");
		display(board);
	}
	else {
		display(board);
	}
	return 0;
}
