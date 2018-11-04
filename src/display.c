#include "display.h"
#include <stdio.h>
void display(chessboard *board, int mode) {
	int i, j;
	for (i = 7; i > -1; --i) {
		putchar(i + '1');
		putchar(' ');
		putchar('|');
		for (j = 0; j < 8; ++j) {
			if(board->brd[i][j].pc) {
				putchar(board->brd[i][j].pc);
			}
			else {
				putchar('.');
			}
			putchar(' ');
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

		printf("Meta:%c %c%c %d %d\n\n", board->player, board->enpass_target.file + 'a', board->enpass_target.rank + '1', board->halfmoves, board->fullmoves);
		printf("Castling: ");
		if (board->castling & (1 << white_kingside)) {
			putchar('K');
		}
		if (board->castling & (1 << white_queenside)) {
			putchar('Q');
		}
		if (board->castling & (1 << black_kingside)) {
			putchar('k');
		}
		if (board->castling & (1 << black_queenside)) {
			putchar('q');
		}
		if (!board->castling) {
			putchar('-');
		}
		putchar('\n');
	}
}

#define BORDER '#'
#define WCHAR ' '
#define BCHAR '.'
#define BLANK ' '
#define CHANGESTATE(state) (((state) == WCHAR) ? BCHAR : WCHAR)
#define WIDTH  8
#define HEIGHT  4
#define WIDTH_BY_2 ((WIDTH) / 2)
#define HEIGHT_BY_2 ((HEIGHT) / 2)

/* puts c n times on screen */
void printn(char c, unsigned n) {
	int i;
	for (i = 0; i < n; i++) {
		putchar(c);
	}
}

void filled_display(chessboard *board) {
	int i, j, rank, file;
	rank = file = 8;
	int ni, nj;
	char state;
	char pc;
	char player = board->player;
	state = BCHAR;
	ni = HEIGHT * 8 + 1;
	nj = 8 * WIDTH + 1;
	for (i = 0; i < ni; i++) {
		if (i % HEIGHT == HEIGHT_BY_2) {
			printn(BLANK, WIDTH_BY_2 - 1);
			rank = (player == 'w') ? 7 - (i / HEIGHT) : (i / HEIGHT);
			putchar('1' + rank);
			printn(BLANK, WIDTH_BY_2 - 1);
		}
		else {
			printn(BLANK, WIDTH - 1);
		}
		state = CHANGESTATE(state); /* flipping is happening twice (once at end and beginning- make it thrice! */
		if (i % HEIGHT == 0) {
			printn(BORDER, nj);
		}
		else {
			for (j = 0; j < nj; j++) {
				if (j % WIDTH == 0) {
					state = CHANGESTATE(state); /* everytime you cross a border, flip */
					putchar(BORDER);
				}
				else if((i % HEIGHT == HEIGHT_BY_2) && (j % WIDTH == WIDTH_BY_2)) {
					file = (player == 'b') ? 7 - (j / WIDTH) : (j / WIDTH);
					if ((pc = (board->brd[rank][file].pc))) {
						putchar(pc);
					}
					else {
						putchar(state);
					}
				}
				else {
					putchar(state);
				}
			}
		}
		putchar('\n');
	}

	printn('\n', HEIGHT_BY_2 - 1);
	printn(BLANK, WIDTH - 1);
	for (j = 0; j < nj; j++) {
		if (j % WIDTH == WIDTH_BY_2) {
			file = (player == 'b') ? 7 - (j / WIDTH) : (j / WIDTH);
			putchar('a' + file);
		}
		else {
			putchar(BLANK);
		}
	}
	printn('\n', HEIGHT_BY_2);
}
