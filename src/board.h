#include "defs.h"

/* origin at a1
 * (7, 7) at h8
 * */

typedef struct {
	usint x, y;
}square;

typedef struct {
	char brd[8][8]; /* contains characters in SAN notation which represent pieces */
	char player; /* 'w' white 'b' black to move */
	usint castling; /* X X X X B-queenside B-kingside W-kingside W-queenside */
	square enpass_target; /* the square to which enpass is allowed (there is only one) */
	usint halfmoves; /* number of non-pawn and non capture moves (for 50 move rule) */
	usint fullmoves; /* number of black moves */
}chessboard;

int fenstring_to_board(chessboard *board, char fenstring[]);

void board_to_fenstring(char fenstring[], chessboard board);
