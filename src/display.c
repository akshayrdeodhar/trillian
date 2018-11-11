/* board and metainfo display functions */

#include "display.h"
#include <stdio.h>
#include "chessescape.h"
#include "player.h"

#define COOL_PIECE 1

void printpiece(char pc) {
	switch(pc) {
		case 'P':
			printf(white_pawn);
			break;
		case 'p':
			printf(black_pawn);
			break;
		case 'B':
			printf(white_bishop);
			break;
		case 'b':
			printf(black_bishop);
			break;
		case 'N':
			printf(white_knight);
			break;
		case 'n':
			printf(black_knight);
			break;
		case 'R':
			printf(white_rook);
			break;
		case 'r':
			printf(black_rook);
			break;
		case 'Q':
			printf(white_queen);
			break;
		case 'q':
			printf(black_queen);
			break;
		case 'K':
			printf(white_king);
			break;
		case 'k':
			printf(black_king);
			break;
		default:
			printf("That Was not a Piece\n");
			break;
	}
}

/* to be removed: display */
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

#define VBORDER '|'
#define HBORDER '-'
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

void filled_display(chessboard *board, int mode, char color) {
	int i, j, rank, file;
	rank = file = 8;
	int ni, nj;
	char state;
	char pc;
	state = BCHAR;
	if (color != 'w' && color != 'b') {
		fprintf(stderr, "Invalid perspective: color should be white or black\n");
		return;
	}
	ni = HEIGHT * 8 + 1;
	nj = 8 * WIDTH + 1;
	for (i = 0; i < ni; i++) {
		if (i % HEIGHT == HEIGHT_BY_2) {
			printn(BLANK, WIDTH_BY_2 - 1);
			rank = (color == 'w') ? 7 - (i / HEIGHT) : (i / HEIGHT);
			putchar('1' + rank);
			printn(BLANK, WIDTH_BY_2 - 1);
		}
		else {
			printn(BLANK, WIDTH - 1);
		}
		state = CHANGESTATE(state); /* flipping is happening twice (once at end and beginning- make it thrice! */
		if (i % HEIGHT == 0) {
			printn(HBORDER, nj);
		}
		else {
			for (j = 0; j < nj; j++) {
				if (j % WIDTH == 0) {
					state = CHANGESTATE(state); /* everytime you cross a border, flip */
					putchar(VBORDER);
				}
				else if((i % HEIGHT == HEIGHT_BY_2) && (j % WIDTH == WIDTH_BY_2)) {
					file = (color == 'b') ? 7 - (j / WIDTH) : (j / WIDTH);
					if ((pc = (board->brd[rank][file].pc))) {
						if (COOL_PIECE) {
							printpiece(pc);
						}
						else {
							putchar(pc);
						}
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
			file = (color == 'b') ? 7 - (j / WIDTH) : (j / WIDTH);
			putchar('a' + file);
		}
		else {
			putchar(BLANK);
		}
	}
	printn('\n', HEIGHT_BY_2);
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

