#include "display.h"

void display(chessboard board) {
	int i, j;
	for (i = 7; i > -1; --i) {
		for (j = 0; j < 8; ++j) {
			if(board.brd[i][j]) {
				putchar(board.brd[i][j]);
				putchar(' ');
			}
			else {
				putchar('.');
				putchar(' ');
			}
		}
		putchar('\n');
	}
	putchar('\n');

	printf("Move: %c\n", board.player);
	
	printf("Enpass Target: %c%c\n", board.enpass_target.y + 'a', board.enpass_target.x + '1');

	printf("Halfmoves %d\nFullmoves %d\n", board.halfmoves, board.fullmoves);
}
