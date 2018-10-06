#include <ctype.h>
#include <limits.h>
#include <stdio.h>

#include "moves.h"
#include "board.h"
#include "pieces.h"
#include "defs.h"
#include "display.h"

#define DEBUG_CALCULATE 1

#define isSame(p, q) ((isWhite(p) && isWhite(q)) || (isBlack(p) && isBlack(q)))
#define isDifferent(p, q) (!(isSame(p, q)))

#define slidingPiece(p) ((toupper(p) == 'R') || (toupper(p) == 'Q') || (toupper(p) == 'B'))

#define slide_update(word, moves) ((word) = ((1 << (moves)) - 1))
#define can_slide(word, moves) ((word) & ((1) << ((moves) - 1)))

#define inrange(x, y) ((x > -1) && (x < 8) && (y > -1) && (y < 8))

#define oppKing(p) (isWhite(p) ? 'k' : 'K')

#define DIR_NONE 8

/* origin at a1
 * (7, 7) at h8
 * */


movement find_movement(move mv) {
	movement sl;
	sl.dfile = mv.fin.file - mv.ini.file;
	sl.drank = mv.fin.rank - mv.ini.rank;
	return sl;
}

usint find_dir(movement sl) {
	if (sl.dfile < 0 && sl.drank == 0) {
		return 0; /* w */
	}
	else if (sl.dfile < 0 && sl.drank > 0) {
		return 1; /* nw */
	}
	else if (sl.dfile == 0 && sl.drank > 0) {
		return 2; /* n */
	}
	else if (sl.dfile > 0 && sl.drank > 0) {
		return 3; /* ne */
	}
	else if (sl.dfile > 0 && sl.drank == 0) {
		return 4; /* e */
	}
	else if (sl.dfile > 0 && sl.drank < 0) {
		return 5; /* se */
	}
	else if (sl.dfile == 0 && sl.drank < 0) {
		return 6; /* s */
	}
	else if (sl.dfile < 0 && sl.drank < 0) {
		return 7; /* sw */
	}
	else {
		return DIR_NONE;
	}
}

ssint fileincr(usint direction) {
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

ssint rankincr(usint direction) {
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

int can_move(piece p, position sq, chessboard ch) {
	return 0;
}

usint slide_distance(usint direction) {
	return 0;
}

int piece_can_slide(piece p, position sq) {
	return 0;
}

int knight_move(piece q, position sq){
	return 0;
}

int pawn_move(piece q, position sq) {
	return 0;
}

void calculate_piece(piece *p, chessboard ch) {
	usint i;
	usint j;
	ssint fileinc, rankinc;
	usint rank, file;
	if (!slidingPiece(p->piece)) {
		if (DEBUG_CALCULATE) {
			printf("Not Sliding: %c\n", p->piece);
		}
		return;
	}

	printf("Piece: %c\n", p->piece);

	chessboard moves = ch;

	for (i = p->dir_start; i < 8; i += p->dir_incr) {

		fileinc = fileincr(i);
		rankinc = rankincr(i);
		if (DEBUG_CALCULATE) {
			/*printf("Direction: %d RankInc: %d FileInc %d\n", i, rankinc, fileinc); */
		}

		rank = p->ps.rank + rankinc;
		file = p->ps.file + fileinc;
		j = 1;
		while(inrange(file, rank) && !ch.brd[rank][file].pc) {
			if (DEBUG_CALCULATE) {
				moves.brd[rank][file].pc = '0' + (char)i;
			}
			++j;
			rank += rankinc;
			file += fileinc;
		}


		if (inrange(file, rank) && isDifferent(ch.brd[rank][file].pc, p->piece) && ch.brd[rank][file].pc != oppKing(p->piece)) {
			if (DEBUG_CALCULATE) {
				moves.brd[rank][file].pc = '0' + (char)i;
			}
		}
		else {
			j--; /* revert to a j that works */
		}

		p->dirs[i] = j;

		if (DEBUG_CALCULATE) {
			printf("%d: %d\n", i, j);
		}
	}
	display(ch, MOVES_MODE);
	display(moves, MOVES_MODE);
}

void update_slide(piece *p, chessboard ch, usint direction) {
	int file, rank;
	int fileinc, rankinc;
	int i;

	rank = p->ps.rank;
	file = p->ps.file;

	rankinc = rankincr(direction);
	fileinc = fileincr(direction);

	i = 1;
	while(inrange(file, rank) && !ch.brd[rank][file].pc) {
		++i;
		rank += rankinc;
		file += fileinc;
	}
	if (inrange(file, rank) && isDifferent(ch.brd[rank][file].pc, p->piece) && ch.brd[rank][file].pc != oppKing(p->piece)) {

	}
	else {
		i--;
	}

	p->dirs[direction] = i;
}

void calculate_all(chesset *set, chessboard board) {
	int i;
	
	if (DEBUG_CALCULATE) {
		printf("WHITE:\n");
	}

	for (i = 0; i < set->n_white; ++i) {
		calculate_piece(&(set->whites[i]), board);
	}

	if (DEBUG_CALCULATE) {
		printf("BLACK\n");
	}

	for (i = 0; i < set->n_black; ++i) {
		calculate_piece(&(set->blacks[i]), board);
	}
}
