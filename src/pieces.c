#include "pieces.h"
#include "defs.h"
#include <ctype.h>
#include <stdio.h>

void set_piece(piece *p, char pc, int rank, int file) {
	switch(toupper(pc)) {
		case 'Q':
			p->dir_start = 0;
			p->dir_incr = 1;
			break;
		case 'R':
			p->dir_start = 0;
			p->dir_incr = 2;
			break;
		case 'B':
			p->dir_start = 1;
			p->dir_incr = 1;
			break;
		default:
			break;
	}

	p->piece = pc;

	position temp;
	temp.file = file;
	temp.rank = rank;
	p->ps = temp;
}


void interface_board_set(chessboard *board, chesset *set) {
	int rank, file;
	int w_index, b_index;

	w_index = b_index = 0;

	char rep;

	for (rank = 0; rank < 8; ++rank) {
		for (file = 0; file < 8; ++file) {
			rep = board->brd[rank][file].pc;
			if (isWhite(rep)) {
				set_piece(&(set->whites[w_index]), rep, rank, file);
				board->brd[rank][file].index = w_index;
				w_index++;
			}
			else if (isBlack(rep)) {
				set_piece(&(set->blacks[b_index]), rep, rank, file);
				board->brd[rank][file].index = b_index;
				b_index++;
			}
		}
	}

	set->n_white = w_index;
	set->n_black = b_index;
}


void show_set(chesset set) {
	int i;
	printf("\nWHITE\n");
	for (i = 0; i < set.n_white; ++i) {
		if (set.whites[i].piece) {
			printf("\nPiece: %c\nPin_Status: %d\nDir_Start: %d\tDir_Incr = %d\nPosition: %c%c\n", set.whites[i].piece, set.whites[i].pin_dir, set.whites[i].dir_start, set.whites[i].dir_incr, set.whites[i].ps.file + 'a', set.whites[i].ps.rank + '1');
		}
	}
	printf("\nBLACK\n");

	for (i = 0; i < set.n_black; ++i) {
		if (set.blacks[i].piece) {
			printf("\nPiece: %c\nPin_Status: %d\nDir_Start: %d\tDir_Incr = %d\nPosition: %c%c\n", set.blacks[i].piece, set.blacks[i].pin_dir, set.blacks[i].dir_start, set.blacks[i].dir_incr, set.blacks[i].ps.file + 'a', set.blacks[i].ps.rank + '1');
		}
	}


}

void verify_interface(chessboard board, chesset set) {
	int i;	
	square sq;
	printf("\nWHITE\n");
	for (i = 0; i < set.n_white; ++i) {
		if (set.whites[i].piece) {
			sq = board_position(board, set.whites[i].ps);
			if (sq.index != i) {
				printf("Problem at %c%c\n", set.whites[i].ps.file + 'a', set.whites[i].ps.rank + '1');
			}
		}
	}
	printf("\nBLACK\n");

	for (i = 0; i < set.n_black; ++i) {
		if (set.blacks[i].piece) {
			sq = board_position(board, set.blacks[i].ps);
			if (sq.index != i) {
				printf("Problem at %c%c\n", set.blacks[i].ps.file + 'a', set.blacks[i].ps.rank + '1');
			}
		}
	}
}
