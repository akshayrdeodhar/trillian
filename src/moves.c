#include <ctype.h>
#include <limits.h>
#include <stdio.h>

#include "moves.h"
#include "board.h"
#include "pieces.h"
#include "defs.h"
#include "display.h"

/* compilation macros */
#define DEBUG_MOVES (0 | DEBUG_CALCULATE)
#define DEBUG_CALCULATE 2
#define DEBUG_UPDATE 1

/* piece nature macros */
#define isSame(p, q) ((isWhite(p) && isWhite(q)) || (isBlack(p) && isBlack(q))) /* p, q SAN chars */
#define isDifferent(p, q) (!(isSame(p, q))) /* p, q SAN chars */
#define slidingPiece(p) ((toupper(p) == 'R') || (toupper(p) == 'Q') || (toupper(p) == 'B')) /* p SAN char */

/* piece movement macros */

/* structure of a direction word: X X X F K D D D
 * 7, 6, 5 dont care
 * 4 -> friendly piece at last square
 * 3 -> kill at last square
 * 2, 1, 0: Represent a number from 0-7 which specifies the range
 * So much can be done in 8 bits
 * */
#define slide_attack(word, moves) (word >= (moves & 7)) 
#define slide_move(word, moves) ((word > (moves & 7)) || ((word == (moves & 7)) && !(word & (1 << 4))))
#define slide_smooth(word) (word & 7)

#define oppKing(p) (isWhite(p) ? 'k' : 'K') /* p SAN char */
#define deltarank(p) (isWhite(p) ? 1: -1) /* p SAN char */
#define pawndir(p, dir) (isWhite(p) ? (dir == 1 || dir == 2 || dir || 3) : (dir == 5 || dir == 6 || dir == 7)) /* p SAN char -> pawn, dir directoin (as defined in xincr, yincr */

#define mkpositive(u) (u > 0 ? u : (-u)) /* u is any number */
#define dist(sl) mkpositive((sl.drank != 0 ? sl.drank : sl.dfile)) /* slide_distance in direction. sl is movement (i.e slope-like quantity */

/* direction macros */
#define DIR_NONE 8
#define oppDir(d) ((d + 4) & 7) /* d is direction as defined */

#define MASK_BLACK_CASTLE (3 << 0) /* q k Q K  are the 4 castling bits in an unsigned number */
#define MASK_WHITE_CASTLE (3 << 2)
/* origin at a1
 * (7, 7) at h8
 * */

movement find_movement(position ini, position fin) {
	movement sl;
	sl.dfile = fin.file - ini.file;
	sl.drank = fin.rank - ini.rank;
	return sl;
}

usint find_dir(movement sl) {
	usint dir;
	if (sl.dfile < 0 && sl.drank == 0) {
		dir = 0; /* w */
	}
	else if (sl.dfile < 0 && sl.drank > 0) {
		dir = 1; /* nw */
	}
	else if (sl.dfile == 0 && sl.drank > 0) {
		dir = 2; /* n */
	}
	else if (sl.dfile > 0 && sl.drank > 0) {
		dir = 3; /* ne */
	}
	else if (sl.dfile > 0 && sl.drank == 0) {
		dir = 4; /* e */
	}
	else if (sl.dfile > 0 && sl.drank < 0) {
		dir = 5; /* se */
	}
	else if (sl.dfile == 0 && sl.drank < 0) {
		dir = 6; /* s */
	}
	else if (sl.dfile < 0 && sl.drank < 0) {
		dir = 7; /* sw */
	}
	else {
		dir = DIR_NONE;
	}
	return dir;
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

/* mostly used for king movements, also called from can_move, as for non-pawn pieces, can_move requires can attack */
int can_attack(piece p, position ps, chessboard ch) {
	usint direction;
	usint distance;
	movement sl;
	usint ranksq, filesq;
	if (!inrange(ps.rank, ps.file)) {
		return 0;
	}
	/* square is on the board */
	sl = find_movement(p.ps, ps);
	direction = find_dir(sl);
	distance = dist(sl);
	/* square is on the board */
	switch(toupper(p.piece)) {
		/* piece has been updated at the end of last move */
		case 'Q': case 'R': case 'B':
			if (direction == DIR_NONE) {
				return 0;
			}
			/* piece can move 'distance' squares in 'direction'*/
			if (p.dirs[direction] >= distance) {
				return 1;
			}
			else {
				return 0;
			}
			break;

			/* distance is sqrt(5) and two squares movement in one direction */
		case 'N':
			ranksq = sl.drank * sl.drank;
			filesq = sl.dfile * sl.dfile;
			if ((ranksq + filesq == 5) && ((ranksq == 4) || (filesq == 4))) {
				return 1;
			}
			else {
				return 0;
			}
			break;

			/* rank movement specific according to color, file movement 1 or -1 */
		case 'P':
			if ((sl.drank == deltarank(p.piece)) && (mkpositive(sl.dfile) == 1)) {
				return 1;
			}
			else {
				return 0;
			}
			break;

			/* move distance is 1, direction is one of the eight directions */
		case 'K':
			if (direction == DIR_NONE || distance != 1) {
				return 0;
			}
			else {
				return 1;
			}
			break;

		default:
			fprintf(stderr, "Invalid piece in can_attack, %c\n", p.piece);
			return 0;
			break;
	}
	return 0;
}

int can_move(piece p, position sq, chessboard ch) {
	char piece = ch.brd[sq.file][sq.rank].pc;
	if (!(inrange(sq.rank, sq.file) && (!piece || (isDifferent(p.piece, piece) && (piece != oppKing(p.piece)))))) {
		return 0;
	}
	/* in board, unoccupied or enemy piece */
	/* TODO: Take pins int account */
	return 0;
}

int piece_can_slide(piece p, position sq, chessboard ch) {
	return 0;
}

void calculate_piece(piece *p, chessboard ch) {
	usint i;
	if (!slidingPiece(p->piece)) {
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("Not Sliding: %c\n", p->piece);
		}
		return;
	}

	if (DEBUG_MOVES & DEBUG_CALCULATE) {
		printf("Piece: %c\n", p->piece);
	}
	for (i = p->dir_start; i < 8; i += p->dir_incr) {
		update_slide(p, ch, i);
	}
}

void update_sliding_pieces(chessboard board, chesset *set, move mv) {
	usint i;
	movement sl1, sl2;
	usint dir1, dir2;
	usint dir_start, dir_incr;
	piece pc;
	square sq;

	/* if a sliding piece has moved, recalculate it ?*/

	for (i = 0; i < set->n_white; ++i) {
		pc = set->whites[i];
		if(!slidingPiece(pc.piece)) {
			continue;
		}
		pc = set->blacks[i];

		sl1 = find_movement(pc.ps, mv.ini);
		sl2 = find_movement(pc.ps, mv.fin);

		dir_start = pc.dir_start;
		dir_incr = pc.dir_incr;

		if ((dir1 = find_dir(sl1)) != DIR_NONE && (dir1 % dir_incr == dir_start)) {
			update_slide(&(set->whites[i]), board, dir1);
		}
		if ((dir2 = find_dir(sl2)) != DIR_NONE && (dir2 % dir_incr == dir_start)) {
			update_slide(&(set->whites[i]), board, dir2);
		}
	}
	for (i = 0; i < set->n_black; ++i) {

		pc = set->blacks[i];
		if(!slidingPiece(pc.piece)) {

			continue;
		}
		sl1 = find_movement(pc.ps, mv.ini);
		sl2 = find_movement(pc.ps, mv.fin);

		dir_start = pc.dir_start;
		dir_incr = pc.dir_incr;

		if ((dir1 = find_dir(sl1)) != DIR_NONE && (dir1 % dir_incr == dir_start)) {
			update_slide(&(set->blacks[i]), board, dir1);
		}
		if ((dir2 = find_dir(sl2)) != DIR_NONE && (dir2 % dir_incr == dir_start)) {
			update_slide(&(set->blacks[i]), board, dir2);
		}
	}

	sq = board_position(board, mv.ini);
	if (isWhite(sq.pc)) {
		calculate_piece(&(set->whites[(usint)sq.index]), board);
	}
	else {
		calculate_piece(&(set->blacks[(usint)sq.index]), board);
	}

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
	/* encountered a piece */
	if (!inrange(file, rank)) {
		--i;
		p->dirs[direction] = i;
		return;
	}
	/* slayable piece */
	if (isDifferent(ch.brd[rank][file].pc, p->piece) && ch.brd[rank][file].pc != oppKing(p->piece)) {
		i |= (1 << 3);
	}
	/* non-slayable piece */
	else {
		i |= (1 << 4);
	}

	p->dirs[direction] = i;
}

void calculate_all(chesset *set, chessboard board) {
	int i;
	for (i = 0; i < set->n_white; ++i) {
		calculate_piece(&(set->whites[i]), board);
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

		if (!slidingPiece(set.whites[i].piece)) {
			printf("%c is not a sliding piece\n", set.whites[i].piece);
			continue;
		}

		printf("%c is a sliding piece\n", set.whites[i].piece);

		for(j = set.whites[i].dir_start; j < 8; j += set.whites[i].dir_incr) {
			rankinc = rankincr(j);
			fileinc = fileincr(j);
			rank = set.whites[i].ps.rank + rankinc;
			file = set.whites[i].ps.file + fileinc;
			for(k = 1; k <= slide_smooth(set.whites[i].dirs[j]); ++k) {
				if (!inrange(rank, file)) {
					printf("Error: %c: %d: %d %c%c\n", set.whites[i].piece, j, k, file + 'a', rank + '1');
					break;
				}
				moves.brd[rank][file].pc = 'M';
				rank += rankinc;
				file += fileinc;
			}
			/* k == slide_smooth(set.whites[i].dirs[j]) */
			/*if (set.whites[i].dirs[j] & (1 << 3)) {
			  moves.brd[rank][file].pc = 'X';
			  }*/

		}
		display(board, MOVES_MODE);
		display(moves, MOVES_MODE);
	}
	for (i = 0; i < set.n_black; ++i) {
		moves = board;

		if (!slidingPiece(set.blacks[i].piece)) {
			printf("%c is not a sliding piece\n", set.blacks[i].piece);
			continue;
		}

		printf("%c is a sliding piece\n", set.blacks[i].piece);

		for(j = set.blacks[i].dir_start; j < 8; j += set.blacks[i].dir_incr) {
			rankinc = rankincr(j);
			fileinc = fileincr(j);
			rank = set.blacks[i].ps.rank + rankinc;
			file = set.blacks[i].ps.file + fileinc;
			for(k = 1; k <= slide_smooth(set.blacks[i].dirs[j]); ++k) {
				if (!inrange(rank, file)) {
					printf("Error: %c: %d: %d %c%c\n", set.blacks[i].piece, j, k, file + 'a', rank + '1');
					break;
				}
				moves.brd[rank][file].pc = 'M';
				rank += rankinc;
				file += fileinc;
			}
			/* k == slide_smooth(set.blacks .. ]) */
			/*
			   if (set.blacks[i].dirs[j] & (1 << 3)) {
			   moves.brd[rank][file].pc = 'X';
			   }*/
		}
		display(board, MOVES_MODE);
		display(moves, MOVES_MODE);
	}
}

/* the move given as input to make must be valid
 * the function is not told explicitly the nature of the move
 * it must deduce what kind of move it is based on the squares and pieces involves
 * this requires it to make multiple changes to board when castling is taking place
 * however, as it has a guarentee that the move is valid (note: piece_can_move() has a lot of work to do), it can simply act
 * is there a better way to implement this? perhaps something where the copying work is done by a seperate function, (which it calls)
 * and the metadata is is handled by the function itself.
 * TODO: Better Ideas ?
 * */
void make_move(chessboard *board, chesset *set, move mv) {

	/* this is done only to shut the warning up. there is a guarentee that if castling happens, castle_rook WILL be set to a proper value */
	move castle_rook = mv;

	square from = board->brd[mv.ini.rank][mv.ini.file];
	square to = board->brd[mv.fin.rank][mv.fin.file];

	menial_move(board, set, mv);

	/* move piece on board, update index on board, update piece position */

	/*  A MONUMENT TO MY STUPIDITY: 
	    menial_move(board, set, mv);

	    square from = board->brd[mv.ini.rank][mv.ini.file];
	    square to = board->brd[mv.fin.rank][mv.fin.file];
	    */

	/* castling is a king move, but the rook must also be moved 
	 * Should I instead use a seperate function handle_castle() ? TODO
	 * */
	if (toupper(from.pc) == 'K' && (mv.fin.file == ('g' - 'a') || mv.fin.file == ('c' - 'a'))) {
		/*printf("Castling is Happening\n");*/
		/* TODO TODO TODO: king must be in home square for move to be detected as a castling move, otherwise, king could have been in f1 for all we know */
		castle_rook.ini.rank = castle_rook.fin.rank = mv.ini.rank;
		if (mv.fin.file == ('g' - 'a')) {
			castle_rook.ini.file = 'h' - 'a';
			castle_rook.fin.file = 'f' - 'a';
		}
		else if (mv.fin.file == ('c' - 'a')) {
			castle_rook.ini.file = 'a' - 'a';
			castle_rook.fin.file = 'd' - 'a';
		}

		/* move the rook */
		menial_move(board, set, castle_rook);

		if (isWhite(from.pc)) {
			board->castling &= MASK_WHITE_CASTLE;
		}
		else {
			board->castling &= MASK_BLACK_CASTLE;
		}
	}
	else {
		printf("Not Castling\n");
	}


	/* flip color */
	board->player = (board->player == 'w') ? 'b' : 'w';

	if (isBlack(from.pc)) {
		board->fullmoves += 1;
	}

	if (to.pc) {
		/* restart doomsday clock ? more like boringday clock */
		board->halfmoves = 0;
		/* rearrange chess set */
		kill_piece(board, set, to);
	}

	if (toupper(from.pc) == 'P') {
		/* restart doomsday clock ? more like boringday clock */
		board->halfmoves = 0;
	}
}

void menial_move(chessboard *board, chesset *set, move mv) {
	square from = board->brd[mv.ini.rank][mv.ini.file];
	square blank;
	blank.pc = '\0';
	blank.index = -1;

	board->brd[mv.fin.rank][mv.fin.file] = from;

	board->brd[mv.ini.rank][mv.ini.file] = blank;

	/* update piece positions in set */
	if (isWhite(from.pc)) {
		set->whites[(usint)from.index].ps = mv.fin;
	}
	else {
		set->blacks[(usint)from.index].ps = mv.fin;
	}
}
