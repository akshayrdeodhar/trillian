#include "pieces.h"
#include <ctype.h>
#include <limits.h>

#define isWhite(p) (isupper(p))
#define isBlack(p) (islower(p))
#define isSame(p, q) ((isWhite(p) && isWhite(q)) || (isBlack(p) && isblack(q)))
#define isDifferent(p, q) (!(isSame(p, q))

#define slide_update(word, moves) ((word) = ((1 << (moves)) - 1))
#define can_slide(word, moves) ((word) & ((1) << ((moves) - 1)))

/* origin at a1
 * (7, 7) at h8
 * */


movement find_movement(move mv) {
	movement sl;
	sl.dx = mv.fin.x - mv.ini.x;
	sl.dy = mv.fin.y - mv.ini.x;
	return sl;
}

usint find_dir(movement sl) {
	if (sl.dx < 0 && sl.dy == 0) {
		return 0; /* w */
	}
	else if (sl.dx < 0 && sl.dy > 0) {
		return 1; /* nw */
	}
	else if (sl.dx == 0 && sl.dy > 0) {
		return 2; /* n */
	}
	else if (sl.dx > 0 && sl.dy > 0) {
		return 3; /* ne */
	}
	else if (sl.dx > 0 && sl.dy == 0) {
		return 4; /* e */
	}
	else if (sl.dx > 0 && sl.dy < 0) {
		return 5; /* se */
	}
	else if (sl.dx == 0 && sl.dy < 0) {
		return 6; /* s */
	}
	else if (sl.dx < 0 && sl.dy < 0) {
		return 7; /* sw */
	}
	else {
		return 8;
	}
}

ssint xincr(usint direction) {
	switch(direction) {
		case 0:
			return -1;
			break;
		case 1:
			return -1;
			break;
		case 2:
			return 0;
			break;
		case 3:
			return 1;
			break;
		case 4:
			return 1;
			break;
		case 5:
			return 1;
			break;
		case 6:
			return 0;
			break;
		case 7:
			return -1;
			break;
		default:
			return SCHAR_MIN;
			break;
	}
	return SCHAR_MIN;
}

ssint yincr(usint direction) {
	switch(direction) {
		case 0:
			return 0;
			break;
		case 1:
			return 1;
			break;
		case 2:
			return 1;
			break;
		case 3:
			return 1;
			break;
		case 4:
			return 0;
			break;
		case 5:
			return -1;
			break;
		case 6:
			return -1;
			break;
		case 7:
			return -1;
			break;
		default:
			return SCHAR_MIN;
			break;
	}
	return SCHAR_MIN;
}

int can_move(piece p, square sq, chessboard ch) {
}

usint slide_distance(usint direction) {
}

int piece_can_slide(piece p, square sq) {
}

int knight_move(piece q, square sq){
}

int pawn_move(piece q, square sq) {
}

void calculate(piece *p, chessboard ch) {
}

void update_slide(piece *p, chessboard ch, usint direction) {
}
