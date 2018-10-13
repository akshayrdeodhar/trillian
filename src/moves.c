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
#define DEBUG_CALCULATE (1 << 1)
#define DEBUG_UPDATE (1 << 0)
#define DEBUG_PIN (1 << 2)

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

#define MAXMAG(a, b) (mkpositive(a) > mkpositive(b) ? mkpositive(a) : mkpositive(b))

/* CHECKING Functions: 
 * Validate a move
 * */

int can_attack(piece p, position ps) {
	usint direction;
	usint dist;
	movement sl;
	usint rankinc, fileinc;

	if (!inrange(ps.rank, ps.file)) {
		/* not a valid square */
		return 0;
	}
	sl = find_movement(p.ps, ps);
	direction = find_dir(sl);
	
	rankinc = rankincr(direction);
	fileinc = fileincr(direction);

	if (!((direction >= p.dir_start) && (direction <= p.dir_end) && ((direction - p.dir_start) % p.dir_incr == 0))) {
		/* not in piece's direction */
		return 0;
	}

	if (direction < 8) {
		dist = MAXMAG(sl.drank, sl.dfile);
	}
	else {
		dist = 1;
	}

	if (!dist) {
		return 0;
	}
	
	if (dist > p.dirs[direction & 7]) {
		/* not in calculated range */
		return 0;
	}


	if (toupper(p.piece) == 'P' && fileincr(direction) == 0) {
		/* movement of pawn along rank axis is not attack */
		return 0;
	}

	return 1;
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

/* Movement and Direction Functions
 * find_movement (finds change in rank and file, returns in form of a 2-d vector)
 * find_dir (returns code of direction corresponding to delta_rank and delta_file, DIR_NONE if not a direction of movement of any piece
 * rankincr, fileincr (return change in rank and file per unit movement ALONG direction specified 
 * */
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
	else if ((sl.drank + sl.dfile == 0) && sl.dfile < 0 && sl.drank > 0) {
		dir = 1; /* nw */
	}
	else if (sl.dfile == 0 && sl.drank > 0) {
		dir = 2; /* n */
	}
	else if (sl.drank == sl.dfile && sl.dfile > 0 && sl.drank > 0) {
		dir = 3; /* ne */
	}
	else if (sl.dfile > 0 && sl.drank == 0) {
		dir = 4; /* e */
	}
	else if ((sl.drank + sl.dfile == 0) && sl.dfile > 0 && sl.drank < 0) {
		dir = 5; /* se */
	}
	else if (sl.dfile == 0 && sl.drank < 0) {
		dir = 6; /* s */
	}
	else if (sl.drank == sl.dfile && sl.dfile < 0 && sl.drank < 0) {
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



/* CALCULATION Functions: All possible attacked squares, along with last square in attack range calculated:
 * calculate_all (used only once, at the beginning of the game, to calc all possible moves of all squares
 * calculate_piece (calculate all squares attacked by piece)
 * calculate_direction (calculate all squares attacked by piece in particular direction
 * */
void calculate_all(chesset *set, chessboard ch) {
	usint i;
	for (i = 0; i < set->n_white; ++i) {
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("Index: %d ", i);
		}
		calculate_piece(&(set->whites[i]), ch);
	}

	for (i = 0; i < set->n_black; ++i) {
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("Index: %d ", i);
		}
		calculate_piece(&(set->blacks[i]), ch);
	}
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

		case 'N': case 'K':
			break;

		case 'P':
			/* straight movement */
			if(fileincr(direction) == 0) {
				if (homerow(p->piece) == (rank - rankinc) && !ch.brd[rank][file].pc && !ch.brd[rank + rankinc][file + fileinc].pc) {
					i = 2;
				}
				else if (!ch.brd[rank][file].pc) {
					i = 1;
				}
				else {
					end = ch.brd[rank][file].pc;
					i = 0;
				}
				p->end[direction & 7] = end;
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
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("  %c%c KIL\n", file + 'a', rank + '1');
		}
	}
	else {
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("  %c%c NON\n", file + 'a', rank + '1');
		}
	}

	p->end[direction & 7] = end;
	p->dirs[direction & 7] = (usint)i;
}

/* KING FUNCTIONS:
 * calculate_pins (sets pin_dir of all pieces which are pinned for current board state
 * enumpins (displays list of pinned pieces (debugging?)
 * */




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
		pinning_square = ch.brd[rank][file];
		if (slidingPiece(pinning_square.pc) && friendly_count == 1) {
			oppdir = oppDir(i);
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




/* UPDATE Functions:
 * When a move is made, only those pieces in whose range the affected squares are, have their moves recalculated, in those specific directions
 * update_pieces (updates attack moves and end_piece data for all affected pieces (simply a caller for update_piece()
 * update_piece() checks which directions of piece are affected, recalculates them 
 * */

void update_pieces(chessboard board, chesset *set, move mv) {
	usint i;
	square sq;
	
	sq = board_position(board, mv.fin);
	if (isWhite(sq.pc)) {
		printf(" Recalculating %c at %c%c\n", set->whites[(usint)sq.index].piece, mv.fin.file + 'a', mv.fin.rank + '1');
		calculate_piece(&(set->whites[(usint)sq.index]), board);
	}
	else {
		printf(" Recalculating %c at %c%c\n", set->blacks[(usint)sq.index].piece, mv.fin.file + 'a', mv.fin.rank + '1');
		calculate_piece(&(set->blacks[(usint)sq.index]), board);
	}

	/* if a sliding piece has moved, recalculate it ?*/

	for (i = 0; i < set->n_white; ++i) {
		update_piece(board, &(set->whites[i]), mv);		
	}
	for (i = 0; i < set->n_black; ++i) {
		update_piece(board, &(set->blacks[i]), mv);		
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

	printf(" %c%c to %c%c: dir = %d\n", pc.ps.file + 'a', pc.ps.rank + '1', mv.ini.file + 'a', mv.ini.rank + '1', dir1);
	printf(" %c%c to %c%c: dir = %d\n", pc.ps.file + 'a', pc.ps.rank + '1', mv.fin.file + 'a', mv.fin.rank + '1', dir2);

	if (DEBUG_MOVES & DEBUG_UPDATE) {
		printf(" Updating %c: %d %d -> %d %d %d\n", pc.piece, dir1, dir2, dir_start, dir_incr, dir_end);
	}

	if (((dir1 - dir_start) % dir_incr == 0) && dir1 >= dir_start && dir1 <= dir_end) {
		if (DEBUG_MOVES & DEBUG_UPDATE) {
			printf(" %d is in %d %d %d\n", dir1, dir_start, dir_incr, dir_end);
		}

		calculate_direction(p, board, dir1);
	}

	if (((dir2 - dir_start) % dir_incr == 0) && dir2 >= dir_start && dir2 <= dir_end) {
		if (DEBUG_MOVES & DEBUG_UPDATE) {
			printf(" %d is in %d %d %d\n", dir1, dir_start, dir_incr, dir_end);
		}
		calculate_direction(p, board, dir2);
	}
}



/* MOVEMENT-ACTION Functions
 * make_move() carries out move, updating killed board and pieces, also meta actions for castling and enpass (in future :))
 * menial_move() does simply the copying and erasing of data, does not know of nature of move
 * */

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


/* DEBUGGING Functions
 * Mostly for programmatically checking another function
 * */

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

void debug_calculation(chesset set, chessboard board) {
	chessboard moves;
	int i; 
	for (i = 0; i < set.n_white; ++i) {
		moves = board;

		printf("At index %d: %c\n", i, set.whites[i].piece);
		debug_piece_calculations(&moves, set.whites[i]);

		display(board, MOVES_MODE);
		display(moves, MOVES_MODE);
	}
	for (i = 0; i < set.n_black; ++i) {
		moves = board;

		printf("At index %d: %c\n", i, set.blacks[i].piece);
		debug_piece_calculations(&moves, set.blacks[i]);

		display(board, MOVES_MODE);
		display(moves, MOVES_MODE);
	}
}

void debug_piece_calculations(chessboard *moves, piece p) {
	int j, k;
	ssint rankinc, fileinc;
	usint rank, file;

	for(j = p.dir_start; j <= p.dir_end; j += p.dir_incr) {

		rankinc = rankincr(j);
		fileinc = fileincr(j);
		rank = p.ps.rank + rankinc;
		file = p.ps.file + fileinc;

		k = 1;

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

		if (p.piece == 'P' || p.piece == 'p') {
			printf("%c%c -> %c\n", file + 'a', rank + '1', p.end[j & 7]);
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

void attack_moves(piece p, chessboard board) {
	int file, rank;
	chessboard attacks;

	position ps;

	attacks = board;
	for (file = 0; file < 8; ++file) {
		for (rank = 0; rank < 8; ++rank) {
			ps.rank  = rank;
			ps.file = file;
			if (can_attack(p, ps)) {
				attacks.brd[rank][file].pc = 'X';
			}
		}
	}

	display(board, MOVES_MODE);
	printf("Attacks of %c at %c%c\n", p.piece, p.ps.file + 'a', p.ps.rank + '1');
	display(attacks, MOVES_MODE);
}

void attack_bitboard(chesset set, chessboard board) {
	int i;
	for (i = 0; i < set.n_white; ++i) {
		attack_moves(set.whites[i], board);
	}

	for (i = 0; i < set.n_black; ++i) {
		attack_moves(set.blacks[i], board);
	}
}

