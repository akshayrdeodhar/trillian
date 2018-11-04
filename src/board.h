#ifndef BOARD_H
#define BOARD_H

#include "defs.h"
#include "vector.h"
#include <limits.h>

#define NONE (16)
#define NONE_CO_ORD UCHAR_MAX

/* origin at a1
 * (7, 7) at h8
 * */

typedef struct {
	char pc; /* representation of piece */
	ssint index; /* index in piece array in set */
}square;

typedef struct {
	square brd[8][8]; /* contains characters in SAN notation which represent pieces */
	char player; /* 'w' white 'b' black to move */
	usint castling; /* X X X B-queenside B-kingside W-queenside W-kingside X*/
	position enpass_target; /* the square to which enpass is allowed (there is only one) */
	usint halfmoves; /* number of non-pawn and non capture moves (for 50 move rule) */
	usint fullmoves; /* number of black moves */
	move whiterep;
	usint white_reps;
	move blackrep;
	usint black_reps;
}chessboard;

int fenstring_to_board(chessboard *board, char fenstring[]); /* converts a .fen string to corresponding board position */

void board_to_fenstring(char fenstring[], chessboard *board);

square board_position(chessboard *board, position p); /* returns square at position 'p' on the board */

#endif
