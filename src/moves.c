#include <ctype.h>
#include <limits.h>
#include <stdio.h>

#include "moves.h"
#include "board.h"
#include "pieces.h"
#include "defs.h"
#include "display.h"

/* compilation macros */
#define DEBUG_MOVES (0)
#define DEBUG_CALCULATE 2
#define DEBUG_UPDATE 1
#define DEBUG_PIN 4

#define SEPERATE_FUNCTIONS 0

#define ALTERNATE_UPDATE 1
#define ALTERNATE_INCR 1
#define ALTERNATE_VERIFY 1

/* piece movement macros */
#define SLIDE_KILL (1 << 7)

#define isSame(p, q) ((isWhite(p) && isWhite(q)) || (isBlack(p) && isBlack(q))) /* p, q SAN chars */
#define isDifferent(p, q) ((isWhite(p) && isBlack(q)) || (isBlack(p) && isWhite(q))) /* p, q SAN chars */

#define slidingPiece(p) ((toupper(p) == 'R') || (toupper(p) == 'Q') || (toupper(p) == 'B')) /* p SAN char */

/* structure of a direction word: X X X F K D D D
 * 7, 6 don't care
 * 5 -> opponent king
 * 4 -> friendly piece at last square
 * 3 -> kill at last square
 * 2, 1, 0: Represent a number from 0-7 which specifies the range (how many times piece can make one move in same direction
 * So much can be done in 8 bits
 * */
#define slide_attack(word, moves) (word >= (moves & 7)) 
#define slide_move(word, moves) ((word > (moves & 7)) || ((word == (moves & 7)) && !(word & (1 << 4))))
#define slide_smooth(word) (word & 7)

#define oppKing(p) (isWhite(p) ? 'k' : 'K') /* p SAN char */

#define deltarank(p) (isWhite(p) ? 1: -1) /* p SAN char */
#define pawndir(p, dir) (isWhite(p) ? (dir == 1 || dir == 2 || dir || 3) : (dir == 5 || dir == 6 || dir == 7)) /* p SAN char -> pawn, dir directoin (as defined in xincr, yincr */
#define homerow(p) (p == 'P' ? 1 : (p == 'p') ? 6 : SCHAR_MIN)

#define mkpositive(u) (u > 0 ? u : (-u)) /* u is any number */

#define dist(sl) mkpositive((sl.drank != 0 ? sl.drank : sl.dfile)) /* slide_distance in direction. sl is movement (i.e slope-like quantity */

/* direction macros */
#define DIR_NONE 16

#define oppDir(d) ((d + 4) & 7) /* d is direction as defined - CAUTION: Not used for Knight */

#define MASK_BLACK_CASTLE (3 << 0) /* q k Q K  are the 4 castling bits in an unsigned number */
#define MASK_WHITE_CASTLE (3 << 2)


movement find_movement(position ini, position fin) {
	movement sl;
	sl.dfile = fin.file - ini.file;
	sl.drank = fin.rank - ini.rank;
	return sl;
}

usint find_dir(movement sl) {
	usint dir;
	/* Direction codes
	 * 1  2  3
	 * 0  P  4
	 * 7  6  5
	 * */

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
	/* Knight Move codes
	 * .  9  . 10  . 
	 * 8  .  .  . 11
	 * .  .  N  .  .
	 * 15 .  .  . 12
	 * . 14  . 13  .
	 * */
	else if (sl.dfile == -2 && sl.drank == 1) {
		dir = 8;
	}
	else if (sl.dfile == -1 && sl.drank == 2) {
		dir = 9;
	}
	else if (sl.dfile == 1 && sl.drank == 2) {
		dir = 10;
	}
	else if (sl.dfile == 2 && sl.drank == 1) {
		dir = 11;
	}
	else if (sl.dfile == 2 && sl.drank == -1) {
		dir = 12;
	}
	else if (sl.dfile == 1 && sl.drank == -2) {
		dir = 13;
	}
	else if (sl.dfile == -1 && sl.drank == -2) {
		dir = 14;
	}
	else if (sl.dfile == -2 && sl.drank == -1) {
		dir = 15;
	}
	else {
		dir = DIR_NONE;
	}

	return dir;
}

ssint fileincr(usint direction) {
	const static ssint file_incr[16] = {-1, -1, 0, 1, 1, 1, 0, -1, -2, -1, 1, 2, 2, 1, -1, -2};

	if (direction < 16) {
		return file_incr[direction];
	}
	else {
		return SCHAR_MIN;
	}

	return SCHAR_MIN;
}

ssint rankincr(usint direction) {
	const static ssint rank_incr[16] = {0, 1, 1, 1, 0, -1, -1, -1, 1, 2, 2, 1, -1, -2, -2, -1};

	if (direction < 16) {
		return rank_incr[direction];
	}
	else {
		return SCHAR_MIN;
	}

	return SCHAR_MIN;
}

/* mostly used for king movements, also called from can_move, as for non-pawn pieces, can_move requires can attack */
int can_attack(piece p, position ps) {
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
			if (direction > p.dir_end) {
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

void calculate_piece(piece *p, chessboard ch){
	usint i;
	if (DEBUG_MOVES & DEBUG_CALCULATE) {
		printf("Piece %c\n", p->piece);
	}
	for (i = p->dir_start; i <= p->dir_end; i += p->dir_incr) {
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf(" Dir: %d\n", i);
		}
		calculate_direction(p, ch, i);
	}
}

void calculate_all(chesset *set, chessboard ch) {
	usint i;
	for (i = 1; i < set->n_white; ++i) {
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("Index: %d ", i);
		}
		calculate_piece(&(set->whites[i]), ch);
	}

	for (i = 1; i < set->n_black; ++i) {
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("Index: %d ", i);
		}
		calculate_piece(&(set->blacks[i]), ch);
	}
}

void calculate_pins(chesset *set, chessboard ch, char color) {
	int i, j;
	int oppdir;

	piece p;
	piece *friend;
	piece *enemy;
	usint rank, file;

	int friendly_count = 0;
	int friendly_index;
	int n;

	square pinning_square;
	piece adversary;

	ssint rankinc, fileinc;
	if (color == 'w') {
		p = set->whites[0];
		friend = set->whites;
		enemy = set->blacks;
		n = set->n_white;
	}
	else if (color == 'b') {
		p = set->blacks[0];
		friend = set->blacks;
		enemy = set->whites;
		n = set->n_black;
	}

	for (i = 0; i < n; ++i) {
		friend[i].pin_dir = DIR_NONE;
	}

	for (i = p.dir_start; i <= p.dir_end; i += p.dir_incr) {
		j = 1;

		friendly_count = 0;

		rankinc = rankincr(i);
		fileinc = fileincr(i);

		rank = p.ps.rank + rankinc;
		file = p.ps.file + fileinc;

		while(inrange(rank, file) && !isDifferent(p.piece, ch.brd[rank][file].pc)) {
			if (ch.brd[rank][file].pc) {
				if (!isSame(ch.brd[rank][file].pc, p.piece)) {
					fprintf(stderr, "Non Friendly and Non Enemy piece\n");
				}
				friendly_count++;
				friendly_index = ch.brd[rank][file].index;
				if (DEBUG_MOVES & DEBUG_PIN) {
					putchar('|');
				}
			}
			if (DEBUG_MOVES & DEBUG_PIN) {
				printf("%c%c ", file + 'a', rank + '1');
			}
			rank += rankinc;
			file += fileinc;
			j++;
		}
		if (DEBUG_MOVES & DEBUG_PIN) {
			printf("\n");
		}
		if (!inrange(rank, file)) {
			continue;
		}
		oppdir = oppDir(i);
		pinning_square = ch.brd[rank][file];
		if (slidingPiece(pinning_square.pc) && friendly_count == 1) {
			adversary = enemy[(int)pinning_square.index]; 
			if (DEBUG_MOVES & DEBUG_PIN) {
				printf("Adversary %c at %c%c\n", adversary.piece, adversary.ps.file + 'a', adversary.ps.rank + '1');
			}
			if ((oppdir % adversary.dir_incr == adversary.dir_start)) {
				friend[friendly_index].pin_dir = i;
			}
		}
	}
}


void calculate_direction(piece *p, chessboard ch, usint direction) {
	usint rank, file, i;
	ssint rankinc, fileinc;

	rankinc = rankincr(direction);
	fileinc = fileincr(direction);

	rank = p->ps.rank + rankinc;
	file = p->ps.file + fileinc;

	i = 1;
	char end = '\0';
	switch(toupper(p->piece)) {
		case 'Q': case 'R': case 'B':
			/* slide to first obstacle */
			while(inrange(rank, file) && !ch.brd[rank][file].pc) {
				++i;
				rank += rankinc;
				file += fileinc;
			}
			break;

		case 'N':
			break;

		case 'P':
			/* straight movement */
			if(fileincr(direction) == 0) {
				if (homerow(p->piece) == (rank - rankinc) && !ch.brd[rank][file].pc && !ch.brd[rank + rankinc][file + fileinc].pc) {
					i = 2;
				}
				else if (!ch.brd[rank][file].pc) {
					printf("Pwn: at %c%c %c\n", p->ps.file + 'a', p->ps.rank + '1', ch.brd[rank][file].pc);
					i = 1;
				}
				else {
					i = 0;
				}
				p->dirs[direction & 7] = i;
				return;
			}
			break;

		default:
			break;
	}

	if (!inrange(rank, file)) {
		i--;
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("  %c%c OOB\n", file + 'a', rank + '1');
		}
	}
	else if (oppKing(p->piece) == ch.brd[rank][file].pc) {
		/*i |= (1 << 5);*/
		end = ch.brd[rank][file].pc;
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("  %c%c CAP\n", file + 'a', rank + '1');
		}
	}
	else if (isSame(p->piece, ch.brd[rank][file].pc)) {
		/*i |= (1 << 4);*/
		end = ch.brd[rank][file].pc;
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("  %c%c FRI\n", file + 'a', rank + '1');
		}
	}
	else if (isDifferent(p->piece, ch.brd[rank][file].pc)) {
		/*i |= (1 << 3);*/
		end = ch.brd[rank][file].pc;
	}

	p->end[direction & 7] = end;
	p->dirs[direction & 7] = (usint)i;
}

void verify_calculation(chesset set, chessboard board) {
	chessboard moves;
	int i; 
	for (i = 0; i < set.n_white; ++i) {
		moves = board;

		printf("At index %d: %c\n", i, set.whites[i].piece);
		verify_piece_calculations(&moves, set.whites[i]);

		display(board, MOVES_MODE);
		display(moves, MOVES_MODE);
	}
	for (i = 0; i < set.n_black; ++i) {
		moves = board;

		printf("At index %d: %c\n", i, set.blacks[i].piece);
		verify_piece_calculations(&moves, set.blacks[i]);

		display(board, MOVES_MODE);
		display(moves, MOVES_MODE);
	}
}

void verify_piece_calculations(chessboard *moves, piece p) {
	int j, k;
	ssint rankinc, fileinc;
	usint rank, file;

	if (toupper(p.piece) == 'K') {
		return;
	}

	for(j = p.dir_start; j <= p.dir_end; j += p.dir_incr) {

		rankinc = rankincr(j);
		fileinc = fileincr(j);
		rank = p.ps.rank + rankinc;
		file = p.ps.file + fileinc;

		k = 1;

		if (toupper(p.piece) == 'N') {
			printf("%d\n", p.dirs[j & 7] & 7);
		}

		while (k < (p.dirs[j & 7] & 7)) {
			if (!inrange(rank, file)) {
				printf("Units: %d\n", p.dirs[j & 7] & 7);
				printf("Error: %c at %c%c : d = %d in direction %d at %c%c\n", p.piece, p.ps.file + 'a', p.ps.rank + '1', k & 7, j , file + 'a', rank + '1');
				break;
			}
			moves->brd[rank][file].pc = 'M';
			rank += rankinc;
			file += fileinc;
			k++;
		}

		if (p.dirs[j & 7]  && isSame(p.piece, p.end[j & 7])/*& (1 << 3)*/) {
			moves->brd[rank][file].pc = 'S';
		}
		else if (p.dirs[j & 7] && oppKing(p.piece) == p.end[j & 7]/*& (1 << 4)*/) {
			moves->brd[rank][file].pc = 'C';
		}
		else if(p.dirs[j & 7] && isDifferent(p.piece, p.end[j & 7])/*& (1 << 5)*/) {
			moves->brd[rank][file].pc = 'X';
		}
		else if (inrange(rank, file)) {
			moves->brd[rank][file].pc = 'M';
		}
	}
}

void check_manually(chesset s) {
	int i, j;
	for (i = 0; i < s.n_white; ++i) {
		if (s.whites[i].piece == 'N') {
			printf("Knight\n");
			for (j = 0; j < 8; ++j) {
				printf("Danger index = %d direction = %d\n d = %d\n", i, j, s.whites[i].dirs[j] & 7);
			}
		}
	}

	for (i = 0; i < s.n_black; ++i) {
		if (s.blacks[i].piece == 'n') {
			printf("Knight\n");
			for (j = 0; j < 8; ++j) {
				printf("Danger index = %d direction = %d d = %d\n", i, j, s.blacks[i].dirs[j] & 7);
			}
		}
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

void update_pieces(chessboard board, chesset *set, move mv) {
	usint i;
	square sq;

	/* if a sliding piece has moved, recalculate it ?*/

	for (i = 0; i < set->n_white; ++i) {
		update_piece(board, &(set->whites[i]), mv);		
	}
	for (i = 0; i < set->n_black; ++i) {
		update_piece(board, &(set->blacks[i]), mv);		
	}

	sq = board_position(board, mv.fin);
	if (isWhite(sq.pc)) {
		calculate_piece(&(set->whites[(usint)sq.index]), board);
	}
	else {
		calculate_piece(&(set->blacks[(usint)sq.index]), board);
	}

}

void update_piece(chessboard board, piece *p, move mv) {
	movement sl1, sl2;
	usint dir1, dir2;
	usint dir_start, dir_incr, dir_end;
	piece pc;

	pc = *p;

	sl1 = find_movement(pc.ps, mv.ini);
	sl2 = find_movement(pc.ps, mv.fin);

	dir_start = pc.dir_start;
	dir_incr = pc.dir_incr;
	dir_end = pc.dir_end;

	dir1 = find_dir(sl1);
	dir2 = find_dir(sl2);

	if (DEBUG_MOVES & DEBUG_UPDATE) {
		printf("%d %d -> %d %d %d\n", dir1, dir2, dir_start, dir_incr, dir_end);
	}

	if (((dir1 - dir_start) % dir_incr == 0) && dir1 >= dir_start && dir1 <= dir_end) {
		if (DEBUG_MOVES & DEBUG_UPDATE) {
			printf("Updating %c at %c%c in direction %d\n", pc.piece, pc.ps.file + 'a', pc.ps.rank + '1', dir1);
		}

		calculate_direction(p, board, dir1);
	}

	if (((dir2 - dir_start) % dir_incr == 0) && dir2 >= dir_start && dir2 <= dir_end) {
		if (DEBUG_MOVES & DEBUG_UPDATE) {
			printf("Updating %c at %c%c in direction %d\n", pc.piece, pc.ps.file + 'a', pc.ps.rank + '1', dir1);
		}
		calculate_direction(p, board, dir2);
	}
}

void enumpins(chesset set) {
	int i;
	for (i = 0; i < set.n_white; ++i) {
		if (set.whites[i].pin_dir != DIR_NONE) {
			printf("PIN: Piece at %c%c pinned in direction %d\n", set.whites[i].ps.file + 'a', set.whites[i].ps.rank + '1', set.whites[i].pin_dir);
		}
	}
	for (i = 0; i < set.n_black; ++i) {
		if (set.blacks[i].pin_dir != DIR_NONE) {
			printf("PIN Piece at %c%c pinned in direction %d\n", set.blacks[i].ps.file + 'a', set.blacks[i].ps.rank + '1', set.blacks[i].pin_dir);
		}
	}
}
