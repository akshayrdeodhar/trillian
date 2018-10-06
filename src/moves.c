#include <ctype.h>
#include <limits.h>
#include <stdio.h>

#include "moves.h"
#include "board.h"
#include "pieces.h"
#include "defs.h"
#include "display.h"

#define DEBUG_CALCULATE 0

#define SLIDE_KILL (1 << 7)

#define isSame(p, q) ((isWhite(p) && isWhite(q)) || (isBlack(p) && isBlack(q)))
#define isDifferent(p, q) (!(isSame(p, q)))

#define slidingPiece(p) ((toupper(p) == 'R') || (toupper(p) == 'Q') || (toupper(p) == 'B'))

#define slide_update(word, moves) ((word) = ((1 << (moves)) - 1))
#define can_slide(word, moves) ((word) & ((1) << ((moves) - 1)))

#define inrange(x, y) ((x > -1) && (x < 8) && (y > -1) && (y < 8))

#define oppKing(p) (isWhite(p) ? 'k' : 'K')

#define deltarank(p) (isWhite(p) ? 1: -1)

#define pawndir(p, dir) (isWhite(p) ? (dir == 1 || dir == 2 || dir || 3) : (dir == 5 || dir == 6 || dir == 7))

#define DIR_NONE 8

#define ALTERNATE_UPDATE 1

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

int piece_can_slide(piece p, position sq, chessboard ch) {
	return 0;
}

int knight_move(piece q, position sq, chessboard ch){
	if (q.pin_dir != 8) {
		return 0;
	}
	char piece;
	usint ranksq, filesq;
	move mv;
	movement sl;
	mv.ini = q.ps;
	mv.fin = sq;
	sl = find_movement(mv);

	ranksq = sl.rank * sl.rank;
	filesq = sl.file * sl.file;

	piece = ch.brd[sq.rank][sq.file].pc;

	if (inrange(sq.rank, sq.file) && ((ranksq + filesq) == 5) && ((ranksq == 4) || (filesq == 4)) && (!piece || isDifferent(q.piece, piece))) {
		return 1;
	}
	else {
		return 0;
	}
}

int pawn_move(piece q, position sq, chessboard ch) {
	usint direction;
	ssint val_drank;
	char piece;
	move mv;
	movement sl;
	mv.ini = q.ps;
	mv.fin = sq;
	sl = find_movement(mv);
	piece = ch.brd[sq.file][sq.rank].pc;
	val_drank = deltarank(q.piece);
	direction = find_dir(sl);
	/* pin */
	if (q.pin_dir != 8 && q.pin_dir != direction) {
		return 0;
	}

	/* single push */
	if (sl.drank == val_drank && sl.dfile == 0 && !piece && direction == 2) {
		return 1;
	}
	/* double push */
	else if (q.ps.rank == homerank(q.piece) && sl.drank == val_drank * 2 && sl.dfile == 0 && !piece && !ch.brd[sq.rank - val_dran][sq.file] && direction == 2) {
		return 1;
	}
	/* kill */
	else if (sl.drank == val_drank && (sl.dfile * sl.dfile == 1) && isDifferent(q.piece, piece) && (direction == 1 || direction == 3)) {
		return 1;
	}
	else {
		return 0;
	}
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

#if (ALTERNATE_UPDATE == 1)
		update_slide(p, ch, i);
#else
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
#endif
	}
#if (ALTERNATE_UPDATE == 0)
	if (DEBUG_CALCULATE) {
		display(ch, MOVES_MODE);
		display(moves, MOVES_MODE);
	}
#endif
}

void update_slide(piece *p, chessboard ch, usint direction) {
	int file, rank;
	int fileinc, rankinc;
	int i;

	rankinc = rankincr(direction);
	fileinc = fileincr(direction);
	
	rank = p->ps.rank + rankinc;
	file = p->ps.file + fileinc;

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

void verify_calculation(chesset set, chessboard board) {
	chessboard moves;
	int i, j, k;
	ssint rankinc, fileinc;
	usint rank, file;
	for (i = 0; i < set.n_white; ++i) {
		moves = board;
		printf("%d: %c\n", i, set.whites[i].piece);

		if (!slidingPiece(set.whites[i].piece)) {
			printf("%c is not a sliding piece\n", set.whites[i].piece);
			continue;
		}

		printf("%c is a sliding piece\n", set.whites[i].piece);

		for(j = set.whites[i].dir_start; j < 8; j += set.whites[i].dir_incr) {
			printf("%d: %d\n",j, set.whites[i].dirs[j]);
			rankinc = rankincr(j);
			fileinc = fileincr(j);
			rank = set.whites[i].ps.rank + rankinc;
			file = set.whites[i].ps.file + fileinc;
			for(k = 1; k <= set.whites[i].dirs[j]; ++k) {
				if (!inrange(rank, file)) {
					printf("Error: %c: %d: %d %c%c\n", set.whites[i].piece, j, k, file + 'a', rank + '1');
					break;
				}
				printf("SET\n");
				moves.brd[rank][file].pc = 'X';
				rank += rankinc;
				file += fileinc;
				printf("k: %d\n", k);
			}
		}
		display(board, MOVES_MODE);
		display(moves, MOVES_MODE);
	}
	for (i = 0; i < set.n_black; ++i) {
		moves = board;
		printf("%d: %c\n", i, set.blacks[i].piece);

		if (!slidingPiece(set.blacks[i].piece)) {
			printf("%c is not a sliding piece\n", set.blacks[i].piece);
			continue;
		}

		printf("%c is a sliding piece\n", set.blacks[i].piece);

		for(j = set.blacks[i].dir_start; j < 8; j += set.blacks[i].dir_incr) {
			printf("%d: %d\n",j, set.blacks[i].dirs[j]);
			rankinc = rankincr(j);
			fileinc = fileincr(j);
			rank = set.blacks[i].ps.rank + rankinc;
			file = set.blacks[i].ps.file + fileinc;
			for(k = 1; k <= set.blacks[i].dirs[j]; ++k) {
				printf("k: %d\n", k);
				if (!inrange(rank, file)) {
					printf("Error: %c: %d: %d %c%c\n", set.blacks[i].piece, j, k, file + 'a', rank + '1');
					break;
				}
				moves.brd[rank][file].pc = 'X';
				rank += rankinc;
				file += fileinc;
			}
		}
		display(board, MOVES_MODE);
		display(moves, MOVES_MODE);
	}
}
