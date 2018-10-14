#ifndef PIECES_H
#define PIECES_H

#include "board.h"

#define MAX_PIECES 16
/* assumption: no legal chess position can have more than 16 pieces of the same color */

typedef struct {
	char piece;
	usint pin_dir;
	usint dir_start, dir_incr, dir_end; /*Q-> 0, 1. R-> 0, 2. B-> 1, 2 */
	position ps;
	usint dirs[8]; /* stores number of squares that piece can move in particular direction */
	char end[8]; /* stores piece at the end of movement in direction */
}piece;


/* king is always stored at index 0: this allows use of common functions, and still allows direct access to the king */
typedef struct {
	int n_white;
	piece whites[MAX_PIECES];

	int n_black;
	piece blacks[MAX_PIECES];

	char threat_to;
	position threat_source;
	usint threat_count;
}chesset;

void interface_board_set(chessboard *board, chesset *set); /* sets 'indexes' in board, initialize pieces in chesset */

/* assumption: no legal chess position can have more than 16 pieces of the same color */

void set_piece(piece *p, char pc, int rank, int file); /* initialise piece pointed to by p to pc, with position rank, file. also initialise direction metadata */

void show_set(chesset set); /* show piece, position*/

void verify_interface(chessboard board, chesset set); /* check whether board[piece.rank][piece.file].index == piece.index */

void kill_piece(chessboard *board, chesset *set, square sq); /* remove the piece which occupies 'sq' on board from set, adjust indexes in board */

#endif
