#include "display.h"
#include <stdio.h>
void display(chessboard board, int mode) {
	int i, j;
	for (i = 7; i > -1; --i) {
		putchar(i + '1');
		putchar(' ');
		putchar('|');
		for (j = 0; j < 8; ++j) {
			if(board.brd[i][j].pc) {
				putchar(board.brd[i][j].pc);
				putchar(' ');
			}
			else {
				putchar('.');
				putchar(' ');
			}
		}
		putchar('\n');
	}
	putchar(' ');
	putchar(' ');
	putchar(' ');
	for (i = 0; i < 8; ++i) {
		putchar('_');
		putchar(' ');
	}
	putchar('\n');
	putchar(' ');
	putchar(' ');
	putchar(' ');
	for (i = 0; i < 8; ++i) {
		putchar(i + 'a');
		putchar(' ');
	}
	putchar('\n');
	putchar('\n');

	if (mode == READ_MODE) {

		printf("Meta:%c %c%c %d %d\n\n", board.player, board.enpass_target.file + 'a', board.enpass_target.rank + '1', board.halfmoves, board.fullmoves);
	}
}
