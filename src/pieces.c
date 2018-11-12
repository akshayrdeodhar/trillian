/* pieces initialisation, manipulation, interfacing with board */

#include "pieces.h"
#include "defs.h"
#include <ctype.h>
#include <stdio.h>

#define DEBUG_PIECES (0)
#define DEBUG_KILL (1 << 0)

/* direction macros */
#define DIR_NONE 16

void set_piece(piece *p, char pc, int rank, int file) {
	switch(toupper(pc)) {
		/* but for king, range is restricted */
		case 'Q': case 'K':
			p->dir_start = 0;
			p->dir_incr = 1;
			p->dir_end = 7;
			break;
		case 'R':
			p->dir_start = 0;
			p->dir_incr = 2;
			p->dir_end = 7;
			break;
		case 'B':
			p->dir_start = 1;
			p->dir_incr = 2;
			p->dir_end = 7;
			break;
		/* not really just a direction. same variable is used for different purposes */
		case 'N':
			p->dir_start = 8;
			p->dir_incr = 1;
			p->dir_end = 15;
			break;
		case 'P':
			p->dir_start = isWhite(pc) ? 1 : 5;
			p->dir_incr = 1;
			p->dir_end = isWhite(pc) ? 3 : 7;
			break;
		default:
			break;
	}

	p->piece = pc;
	p->pin_dir = NONE;

	position temp;
	temp.file = file;
	temp.rank = rank;
	p->ps = temp;
}


void interface_board_set(chessboard *board, chesset *set) {
	int rank, file;
	int w_index, b_index;

	w_index = b_index = 1; /* leave space for the king */

	char rep;

	for (rank = 0; rank < 8; ++rank) {
		for (file = 0; file < 8; ++file) {
			rep = board->brd[rank][file].pc;
			if (rep == 'K') {
				set_piece(&(set->whites[0]), rep, rank, file);
				board->brd[rank][file].index = 0;
			}
			else if (rep == 'k') {
				set_piece(&(set->blacks[0]), rep, rank, file);
				board->brd[rank][file].index = 0;
			}
			else if (isWhite(rep)) {
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


void kill_piece(chessboard *board, chesset *set, square sq) {
	piece *array;
	piece container;
	int *index;
	int i;
	if (isWhite(sq.pc)) {
		array = (set->whites);
		index = &(set->n_white);
	}
	else {
		array = (set->blacks);
		index = &(set->n_black);
	}
	
#if (DEBUG_PIECES & DEBUG_KILL)
	printf("%c killed at %c%c\n", sq.pc, array[sq.index].ps.file + 'a', array[sq.index].ps.rank + '1');
#endif

	for (i = sq.index; i < *index - 1; ++i) {
		container = array[i + 1];
		board->brd[container.ps.rank][container.ps.file].index -= 1;
		array[i] = container;
	}
	*index -= 1;
}
