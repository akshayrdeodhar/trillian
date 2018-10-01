#include <stdio.h>
#include <string.h>
#include "display.h"

int main(void) {
	int n;
	char string[128];
	fgets(string, 128, stdin);
	n = strlen(string);
	if (string[n - 1] == '\n') {
		string[n - 1] = '\0';
	}
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
