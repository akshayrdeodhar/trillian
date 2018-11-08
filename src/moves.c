#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>

#include "moves.h"
#include "board.h"
#include "pieces.h"
#include "defs.h"
#include "display.h"
#include "vector.h"

/* compilation macros */
#define DEBUG_MOVES (0)
#define DEBUG_CALCULATE (1 << 1)
#define DEBUG_UPDATE (1 << 0)
#define DEBUG_PIN (1 << 2)
#define DEBUG_VALID (1 << 3)
#define DEBUG_THREAT (1 << 4)
#define DEBUG_CASTLE (1 << 5)
#define DEBUG_GAMEND (1 << 6)

#define OLD_CASTLE (0)
#define OLD_STALEMATE (0)
#define TRY_ENPASS (0)

#define whitesquare(pos) (((pos.rank) + (pos.file)) & 1)
#define blacksquare(pos) (!whitesquare(pos))
#define same_color_square(pos1, pos2) ((whitesquare(pos1) && whitesquare(pos2)) || (blacksquare(pos1) && blacksquare(pos2)))

#define EPSILON 1e-6

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
#define pawndir(p, dir) (isWhite(p) ? ((dir) == 1 || (dir) == 2 || (dir) == 3) : ((dir) == 5 || (dir) == 6 || (dir) == 7)) /* p SAN char -> pawn, dir directoin (as defined in xincr, yincr */
#define homerow(p) (p == 'P' ? 1 : (p == 'p') ? 6 : SCHAR_MIN)


#define euclidian(sl) (sqrt(sl.drank * sl.drank + sl.dfile * sl.dfile))


/* direction macros */
#define oppDir(d) ((d + 4) & 7) /* d is direction as defined - CAUTION: Not used for Knight */

#define MASK_BLACK_CASTLE (3 << 0) /* q k Q K  are the 4 castling bits in an unsigned number */
#define MASK_WHITE_CASTLE (3 << 2)


#define posequal(pos1, pos2) (pos1.rank == pos2.rank && pos1.file == pos2.file)
#define movesequal(mv1, mv2) (posequal(mv1.ini, mv2.ini) && posequal(mv1.fin, mv2.fin))

/* PROMOTION Function:
 * promotes pawn in final rank to chosen piece
 * recalulates moves of that piece
 * */

void handle_promotion(chessboard *board, chesset *set, move mv, char promoted) {
	square final;
	final = board->brd[mv.fin.rank][mv.ini.file];
	if (final.pc == 'P') {
		set_piece(&(set->whites[(usint)final.index]), promoted, mv.fin.rank, mv.fin.file);
		calculate_piece(&(set->whites[(usint)final.index]), board);
	}
	else if (final.pc == 'p') {
		set_piece(&(set->blacks[(usint)final.index]), promoted, mv.fin.rank, mv.fin.file);
		calculate_piece(&(set->blacks[(usint)final.index]), board);
	}
	board->brd[mv.fin.rank][mv.fin.file].pc = promoted;
}

/* CASTLING FUNCTIONS:
 * castle_move() checks whether move is castling, and returns typed
 * can_castle() checks whether particular castling move is allowed
 * */

/* the 4 castle moves. also used for validation */
const static move king_castle_moves[] = {
	{{0, 4}, {0, 6}},
	{{0, 4}, {0, 2}},
	{{7, 4}, {7, 6}}, 
	{{7, 4}, {7, 2}}
};

const static move rook_castle_moves[] = {
	{{0, 7}, {0, 5}},
	{{0, 0}, {0, 3}},
	{{7, 7}, {7, 5}}, 
	{{7, 0}, {7, 3}},
	{{8, 8}, {8, 8}} /* for invalid input */
};

move king_castle(special_move castle) {
	return king_castle_moves[castle - 1];
}



special_move check_special(square sq, move mv) {
	move temp;
	char pc = toupper(sq.pc);
	int i;

	if (!(pc == 'K' || pc == 'P')) {
		return none;
	}

	if (pc == 'K') {
		for (i = white_kingside; i <= black_queenside; ++i) {
			/* if move is one of the 4 possible castling moves */
			temp = king_castle_moves[i - 1];
			if (movesequal(mv, temp)) {
				/* return castling code */
				return i;
			}
		}
	}
	else if (pc == 'P') {
		if (mv.fin.rank == 7 || mv.fin.rank == 0) {
			/*promotion */
			return promotion;
		}
	}

	/* not a castling move */
	return none;
}


/* changes castling availibility based on king and rook movement */
void update_castling(chessboard *board, move mv) {
	special_move castle;
	for (castle = white_kingside; castle <= black_queenside; castle++) {
		/* for each possible castling move
		 * NOTE: piece can never move to king king home position unless king has already moved from there */
		if ((board->castling & (1 << (castle))) && (posequal(mv.ini, king_castle_moves[castle - 1].ini) || posequal(mv.ini, rook_castle_moves[castle - 1].ini) || posequal(mv.fin, rook_castle_moves[castle - 1].ini))) {
			/* if that move is available and something is moving from the king initial square, or something is moving from or to the rook initial square, that particular move*/
			if (DEBUG_MOVES & DEBUG_CASTLE) {
				printf("Disabled Castle: %d\n", castle);
				show_register(board->castling);
				show_register(~(1 << (castle)));
			}
			board->castling &= (~(1 << (castle))); /* that castling is disabled */
			if (DEBUG_MOVES & DEBUG_CASTLE) {
				show_register(board->castling);
			}
		}
	}
}

int can_castle(chessboard *board, chesset *set, special_move castle) {
	if (!(board->castling & (1 << (castle)))) {
		if (DEBUG_MOVES & DEBUG_CASTLE) {
			/* if that castling move is not available */
			printf("Checking: %d\n", castle);
			show_register(((usint)castle) << 1);
			show_register(board->castling);
			printf("Not Available\n");
		}
		return 0;
	}
	usint dir;
	ssint fileinc;
	char king_color;
	piece *enemy;
	int n, i;
	position slide_square;
	movement sl;

	move king_move, rook_move;

	king_move = king_castle_moves[castle - 1];
	sl = find_movement(king_move.ini, king_move.fin);
	dir = find_dir(sl);
	fileinc = fileincr(dir);

	king_color = board->brd[king_move.ini.rank][king_move.ini.file].pc;
	if (king_color == 'K') {
		enemy = set->blacks;
		n = set->n_black;
	}
	else if (king_color == 'k') {
		enemy = set->whites;
		n = set->n_white;
	}
	else {
		fprintf(stderr, "Attempting to castle non-king piece\n");
		return 0;
	}

	if (set->threat_count) {
		/* king in check */
		return 0;
	}

	rook_move = rook_castle_moves[castle - 1];
	char rook_color = board->brd[rook_move.ini.rank][rook_move.ini.file].pc;
	if (toupper(rook_color) != 'R' || !isSame(king_color, rook_color)) {
		if (DEBUG_MOVES & DEBUG_CASTLE) {
			printf("No Rook\n");
		}
		return 0;
	}

	slide_square.rank = king_move.ini.rank;

	for (slide_square.file = king_move.ini.file + fileinc; slide_square.file != (rook_move.ini.file); slide_square.file += fileinc) {
		if (board->brd[slide_square.rank][slide_square.file].pc) {
			return 0;
		}
	}

	for (slide_square.file = king_move.ini.file + fileinc; slide_square.file != (king_move.fin.file + fileinc); slide_square.file += fileinc) {
		for (i = 0; i < n; ++i) {
			if (can_attack(enemy[i], slide_square)) {
				if (DEBUG_MOVES & DEBUG_CASTLE) {
					printf("%c%c attacked by %c\n", slide_square.file + 'a', slide_square.rank + '1', enemy[i].piece);
				}
				return 0;
			}
		}
	}
	return 1;
}

move rook_move(special_move king_castle) {
#define INVALID_CASTLE 4
	if (!king_castle) {
		return rook_castle_moves[INVALID_CASTLE];
	}
	return rook_castle_moves[king_castle - 1];
}


/* CHECKING Functions: 
 * Validate a move
 * */


int can_attack(piece p, position ps) {
	usint direction;
	usint dist;
	movement sl;

	if (!inrange(ps.rank, ps.file)) {
		/* not a valid square */
		return 0;
	}
	sl = find_movement(p.ps, ps);
	direction = find_dir(sl);

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
	dist = distance(sl, direction);

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

/* checks whether piece movement to position is a legal move
 * does not consider whether king is in check 
 * */
int vanilla_can_move(piece p, position ps) {
	usint direction;
	usint dist;
	movement sl;

	if (!inrange(ps.rank, ps.file)) {
		/* not a valid square */
		return 0;
	}
	sl = find_movement(p.ps, ps);
	direction = find_dir(sl);

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

	if (!dist || dist > p.dirs[direction & 7]) {
		/* not in calculated range or friendly file */
		return 0;
	}
	else if (dist == p.dirs[direction & 7] && ((isSame(p.piece, p.end[direction & 7])) || oppKing(p.piece) == p.end[direction & 7])) {
		/* friendly or enemy king */
		return 0;
	}

	if (toupper(p.piece) == 'P' && sl.dfile && !p.end[direction]) {
		/* diagonal movement of pawn when there is no piece to be attacked */
		return 0;
	}

	if (toupper(p.piece) == 'K') {
		/* king moving into attacked square */
		if (DEBUG_MOVES & DEBUG_THREAT) {
			show_register(p.pin_dir);
		}
		if (p.pin_dir & (1 << direction)) {
			return 0;
		}
	}
	else if ((p.pin_dir != DIR_NONE) && direction != p.pin_dir && oppDir(direction) != p.pin_dir) {
		/* pinned piece moving along non-pinnned direction */
		return 0;
	}

	return 1;
}

int can_move(chessboard *board, chesset *set, move mv) {
	piece p, king;
	special_move castle;
	square sq;

	sq = board->brd[mv.ini.rank][mv.ini.file];

	if (isWhite(sq.pc) && board->player == 'w') {
		p = set->whites[(usint)sq.index];
		king = set->whites[0];
	}
	else if (isBlack(sq.pc) && board->player == 'b') {
		p = set->blacks[(usint)sq.index];
		king = set->blacks[0];
	}
	else {
		/* no piece or trying to control opponents piece */
		fprintf(stderr, "Trying to control opponent's piece\n");
		return 0;
	}

	if (set->threat_to == board->player && set->threat_count && toupper(p.piece) != 'K') {
		/* king in check and another piece moving */
		if (set->threat_count == 1) {
			movement king_to_piece = find_movement(king.ps, mv.fin);
			movement piece_to_threat = find_movement(mv.fin, set->threat_source);
			movement king_to_threat = find_movement(king.ps, set->threat_source);
			if (DEBUG_THREAT & DEBUG_MOVES) {
			printf("king_to_piece: %lf\tpiece_to_threat: %lf\tking_to_threat: %lf\n", euclidian(king_to_piece), euclidian(piece_to_threat), euclidian(king_to_threat));
			}
			if (fabs(euclidian(king_to_piece) + euclidian(piece_to_threat) - euclidian(king_to_threat)) > EPSILON) {
				/* if the king, the piece and the threat are not in a straight line :) :) :) */
				fprintf(stderr, "King in check\n");
				return 0;
			}
		}
		else {
			/* multiple checks and king not moving */
			fprintf(stderr, "King in multiple checks\n");
			return 0;
		}
	}

	castle = check_special(board->brd[mv.ini.rank][mv.ini.file], mv);
	if (castle >= white_kingside && castle <= black_queenside) {
		return can_castle(board, set, castle);
	}

	return vanilla_can_move(p, mv.fin);
}



/* CALCULATION Functions: All possible attacked squares, along with last square in attack range calculated:
 * calculate_all (used only once, at the beginning of the game, to calc all possible moves of all squares
 * calculate_piece (calculate all squares attacked by piece)
 * calculate_direction (calculate all squares attacked by piece in particular direction
 * */
void calculate_all(chesset *set, chessboard *ch) {
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


void calculate_piece(piece *p, chessboard *ch){
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

void calculate_direction(piece *p, chessboard *ch, usint direction) {
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
			while(inrange(rank, file) && !ch->brd[rank][file].pc) {
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
				if (homerow(p->piece) == (rank - rankinc) && !ch->brd[rank][file].pc && !ch->brd[rank + rankinc][file + fileinc].pc) {
					i = 2;
				}
				else if (!ch->brd[rank][file].pc) {
					i = 1;
				}
				else {
					end = ch->brd[rank][file].pc;
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
	end = ch->brd[rank][file].pc;
#if 0
	else if (oppKing(p->piece) == ch->brd[rank][file].pc) {
		/*i |= (1 << 5);*/
		end = ch->brd[rank][file].pc;
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("  %c%c CAP\n", file + 'a', rank + '1');
		}
	}
	else if (isSame(p->piece, ch->brd[rank][file].pc)) {
		/*i |= (1 << 4);*/
		end = ch->brd[rank][file].pc;
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("  %c%c FRI\n", file + 'a', rank + '1');
		}
	}
	else if (isDifferent(p->piece, ch->brd[rank][file].pc)) {
		/*i |= (1 << 3);*/
		end = ch->brd[rank][file].pc;
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("  %c%c KIL\n", file + 'a', rank + '1');
		}
	}
	else {
		if (DEBUG_MOVES & DEBUG_CALCULATE) {
			printf("  %c%c NON\n", file + 'a', rank + '1');
		}
	}

	if (inrange(ch->enpass_target.rank, ch->enpass_target.file)) {
		if (TRY_ENPASS) {
			if (toupper(p->piece) == 'P' && ch->enpass_target.rank == rank && ch->enpass_target.file == file) {
				end = isWhite(p->piece) ? 'p' : 'P';
			}
		}
	}
#endif

	p->end[direction & 7] = end;
	p->dirs[direction & 7] = (usint)i;
}

/* KING FUNCTIONS:
 * calculate_pins (sets pin_dir of all pieces which are pinned for current board state
 * enumpins (displays list of pinned pieces (debugging?)
 * */


void calculate_threats(chesset *set, char color) {
	int n;
	piece *enemy;
	piece *king;
	int i, j;
	movement sl;
	usint direction, dist;
	position neighbour;

	set->threat_count = 0;


	if (color == 'w') {
		enemy = set->blacks;
		king = &(set->whites[0]);
		n = set->n_black;
		set->threat_to = 'w';
	}
	else if (color == 'b') {
		enemy = set->whites;
		king = &(set->blacks[0]);
		n = set->n_white;
		set->threat_to = 'b';
	}
	else {
		return;
	}

	king->pin_dir = 0; /* reset threat word */

	/* checks */
	for (i = 0; i < n; ++i) {
		if (can_attack(enemy[i], king->ps)) {
			set->threat_count += 1; /* count the number of pieces that are checking the king */
			set->threat_source = enemy[i].ps; /* threat index will be overwritten in case of multiple checks, but then it is not useful anyway*/
			/* if a sliding piece is checking the king,
			 * the position in the direction opposite to the pin direction is also out of bounds
			 * */
			if (slidingPiece(enemy[i].piece)) {
				sl = find_movement(king->ps, enemy[i].ps);
				direction = find_dir(sl);
				dist = distance(sl, direction);
				if (DEBUG_MOVES & DEBUG_THREAT) {
					printf("Direction %d %d\n", direction, oppDir(direction));
				}
				if (direction < 8) {
					if (dist != 1) {
						/* if attacking piece adjecant to king, opposite dir is dir of piece- do not conisder */
						king->pin_dir |= 1 << direction;
					}
					king->pin_dir |= 1 << oppDir(direction);
				}
			}
		}
	}

	/* threats to neighbouring squares */
	for (i = king->dir_start; i <= king->dir_end; i += king->dir_incr) {
		for (j = 0; j < n; ++j) {
			neighbour.rank = king->ps.rank + rankincr(i);
			neighbour.file = king->ps.file + fileincr(i);

			if (can_attack(enemy[j], neighbour)) {
				king->pin_dir |= (1 << i); /* set the dir-th bit */
				/*break;*/ /* it takes only one enemy attack to block a square */
			}
		}
	}
	if (DEBUG_MOVES & DEBUG_THREAT) {
		printf("%x\n", king->pin_dir);
	}
}


void calculate_pins(chesset *set, chessboard *ch, char color) {
	int i, j;
	int oppdir;

	piece p;
	/* just to shut up errors */
	p.ps.rank = p.ps.file = 8; 
	p.dir_start = p.dir_end = p.dir_incr = 8;
	p.piece = '\0';
	/* end of shutup_errors */
	piece *friend = NULL;
	piece *enemy = NULL;
	usint rank, file;

	int friendly_count = 0;
	int friendly_index;
	int n = 0;

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

	for (i = 1; i < n; ++i) {
		friend[i].pin_dir = DIR_NONE;
	}

	for (i = p.dir_start; i <= p.dir_end; i += p.dir_incr) {
		j = 1;

		friendly_count = 0;

		rankinc = rankincr(i);
		fileinc = fileincr(i);

		rank = p.ps.rank + rankinc;
		file = p.ps.file + fileinc;

		while(inrange(rank, file) && !isDifferent(p.piece, ch->brd[rank][file].pc)) {
			if (ch->brd[rank][file].pc) {
				if (!isSame(ch->brd[rank][file].pc, p.piece)) {
					fprintf(stderr, "Non Friendly and Non Enemy piece\n");
				}
				friendly_count++;
				friendly_index = ch->brd[rank][file].index;
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
		pinning_square = ch->brd[rank][file];
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

void update_pieces(chessboard *board, chesset *set, move mv) {
	usint i;
	square sq;

	sq = board_position(board, mv.fin);
	if (isWhite(sq.pc)) {
		if (DEBUG_UPDATE & DEBUG_MOVES) {
			printf(" Recalculating %c at %c%c\n", set->whites[(usint)sq.index].piece, mv.fin.file + 'a', mv.fin.rank + '1');
		}
		calculate_piece(&(set->whites[(usint)sq.index]), board);
	}
	else {
		if (DEBUG_UPDATE & DEBUG_MOVES) {
			printf(" Recalculating %c at %c%c\n", set->blacks[(usint)sq.index].piece, mv.fin.file + 'a', mv.fin.rank + '1');
		}
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

void update_piece(chessboard *board, piece *p, move mv) {
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
special_move make_move(chessboard *board, chesset *set, move mv) {

	/* this is done only to shut the warning up. there is a guarentee that if castling happens, castle_rook WILL be set to a proper value */
	special_move castle;

	square from = board->brd[mv.ini.rank][mv.ini.file];
	square to = board->brd[mv.fin.rank][mv.fin.file];

	castle = check_special(from, mv);

	update_repetition(board, mv); /* for draw by repetition- requires final square intact. */

	menial_move(board, set, mv);

	/* move piece on board, update index on board, update piece position */

	position temp;
	temp.rank = temp.file = 8;
	if (toupper(from.pc) == 'P' && mkpositive(mv.ini.rank - mv.fin.rank) == 2) {
		temp.rank = (mv.ini.rank + mv.fin.rank) / 2;
		temp.file = mv.ini.file;
	}
	board->enpass_target = temp;

	/* update castling word */
	update_castling(board, mv);

	/* flip color */
	board->player = (board->player == 'w') ? 'b' : 'w';

	if (isBlack(from.pc)) {
		board->fullmoves += 1;
		board->halfmoves += 1;
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

	return castle;
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

void debug_calculation(chesset *set, chessboard *board) {
	chessboard moves;
	int i; 
	for (i = 0; i < set->n_white; ++i) {
		moves = *board;

		printf("At index %d: %c\n", i, set->whites[i].piece);
		debug_piece_calculations(&moves, set->whites[i]);

		display(board, MOVES_MODE);
		display(&moves, MOVES_MODE);
	}
	for (i = 0; i < set->n_black; ++i) {
		moves = *board;

		printf("At index %d: %c\n", i, set->blacks[i].piece);
		debug_piece_calculations(&moves, set->blacks[i]);

		display(board, MOVES_MODE);
		display(&moves, MOVES_MODE);
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

void enumpins(chesset *set) {
	int i;
	for (i = 0; i < set->n_white; ++i) {
		if (set->whites[i].pin_dir != DIR_NONE) {
			printf("PIN: Piece at %c%c pinned in direction %d\n", set->whites[i].ps.file + 'a', set->whites[i].ps.rank + '1', set->whites[i].pin_dir);
		}
	}
	for (i = 0; i < set->n_black; ++i) {
		if (set->blacks[i].pin_dir != DIR_NONE) {
			printf("PIN Piece at %c%c pinned in direction %d\n", set->blacks[i].ps.file + 'a', set->blacks[i].ps.rank + '1', set->blacks[i].pin_dir);
		}
	}
}

void attack_moves(piece p, chessboard *board) {
	int file, rank;
	chessboard attacks;

	position ps;

	attacks = *board;
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
	display(&attacks, MOVES_MODE);
}

void attack_bitboard(chesset *set, chessboard *board) {
	int i;
	for (i = 0; i < set->n_white; ++i) {
		attack_moves(set->whites[i], board);
	}

	for (i = 0; i < set->n_black; ++i) {
		attack_moves(set->blacks[i], board);
	}
}

void show_threats(chesset *set, chessboard *board) {
	int i;
	piece king;
	/* shut up errors */
	king.dir_start = king.dir_end = king.dir_incr = 8;
	king.ps.rank = king.ps.file = 8;
	king.pin_dir = 0;
	/* end of shutup errors */

	chessboard threats = *board;

	if (set->threat_to == 'w') {
		king = set->whites[0];
	}
	else if (set->threat_to == 'b') {
		king = set->blacks[0];
	}

	for (i = king.dir_start; i <= king.dir_end; i += king.dir_incr) {
		if (king.pin_dir & (1 << i)) {
			if (inrange(king.ps.rank + rankincr(i), king.ps.file + fileincr(i))) {
				threats.brd[king.ps.rank + rankincr(i)][king.ps.file + fileincr(i)].pc = 'B';
			}
		}
	}

	if (DEBUG_MOVES & DEBUG_THREAT) {
		printf("Threat To: %s\n", set->threat_to == 'w' ? "White" : "Black");
		printf("Checks: %d\n", set->threat_count);
	}

	if (set->threat_count == 1) {
		printf("Single check from %c%c\n", set->threat_source.file + 'a', set->threat_source.rank + '1');
		threats.brd[set->threat_source.rank][set->threat_source.file].pc = 'C';
	}

	if (DEBUG_THREAT & DEBUG_MOVES) {
		display(board, MOVES_MODE);
		display(&threats, MOVES_MODE);
	}
}

void moves(piece p, chessboard *board) {
	int file, rank;
	chessboard moves;

	position ps;

	moves = *board;
	for (file = 0; file < 8; ++file) {
		for (rank = 0; rank < 8; ++rank) {
			ps.rank  = rank;
			ps.file = file;
			if (vanilla_can_move(p, ps)) {
				moves.brd[rank][file].pc = 'M';
			}
		}
	}

	display(board, MOVES_MODE);
	printf("Moves of %c at %c%c\n", p.piece, p.ps.file + 'a', p.ps.rank + '1');
	display(&moves, MOVES_MODE);
}

void moves_bitboard(chesset *set, chessboard *board) {
	int i;
	for (i = 0; i < set->n_white; ++i) {
		moves(set->whites[i], board);
	}

	for (i = 0; i < set->n_black; ++i) {
		moves(set->blacks[i], board);
	}
}

void show_register(usint word) {
	int i;
	for (i = 7; i > -1; --i) {
		if (word & (1 << i)) {
			putchar('1');
		}
		else {
			putchar('0');
		}
	}
	putchar('\n');
}

/* End-of-Game functions:
 * Detect draw or checkmate
 * */

/* is_checkmate(board, set)
 * the direction array around the king is set, as is the pin_dir jugaad which records the attacked squares
 * so all that is left is to check moves of the remaining pieces 
 * */

int is_checkmate(chessboard *board, chesset *set) {
	piece king;
	piece *friendlies;
	int n, i, j;
	position save;
	movement sl;
	usint direction, dist;
	ssint rankinc, fileinc;

	if (!(set->threat_count)) {
		/* not in check */
		return 0;
	}

	/* chose player's pieces */
	if (board->player == 'w') {
		king = set->whites[0];
		friendlies = set->whites;
		n = set->n_white;
	}
	else if (board->player == 'b') {
		king = set->blacks[0];
		friendlies = set->blacks;
		n = set->n_black;
	}
	else {
		/* invalid */
		return 0;
	}

	if (DEBUG_GAMEND & DEBUG_MOVES) {
		printf("PIN_DIR:");
		show_register(king.pin_dir);
	}

	for (i = king.dir_start; i <= king.dir_end; i += king.dir_incr) {
		if ((!(king.pin_dir & (1 << i))) && king.dirs[i & 7] && (!isSame(king.piece, king.end[i & 7]))) {
			if (DEBUG_GAMEND & DEBUG_MOVES) {
				printf("King can move in direction %d, not checkmate\n", i);
			}
			/* not attacked, not unavailable, and not friendly -> escape available */
			return 0;
		}
	}

	if (set->threat_count == 1) {
		sl = find_movement(king.ps, set->threat_source);
		direction = find_dir(sl);
		rankinc = rankincr(direction);
		fileinc = fileincr(direction);
		save.rank = king.ps.rank + rankinc;
		save.file = king.ps.file + fileinc;
		dist = distance(sl, direction);
		for (i = 1; i <= dist; i++, save.rank += rankinc, save.file += fileinc) {
			/* for each 'natural' square in line joining king and source of check */
			for (j = 0; j < n; j++) {
				/* for each friendly piece */
				if (vanilla_can_move(friendlies[j], save)) {
					/* if can block or kill */
					return 0;
				}
			}
		}
	}

	return 1;
}

/* should be called after board position and threats have been evaluated (i.e at end of move)
 * */
int is_stalemate(chessboard *board, chesset *set) {
	piece *player = NULL;
	piece pc;
	int n = 0;
	int i, j, k;
	ssint rankinc, fileinc;
	position where;

	if (set->threat_count) {
		if (DEBUG_MOVES & DEBUG_GAMEND) {
			printf("King in check, not stale\n");
		}
		/* king in check, cannot be stalemate */
		return 0;
	}

	/* chose player's pieces */
	if (board->player == 'w') {
		player = set->whites;
		n = set->n_white;
	}
	else if (board->player == 'b') {
		player = set->blacks;
		n = set->n_black;
	}

	/* look for piece moves */
	for (i = 1; i < n; i++) {
		pc = player[i];
		for (j = pc.dir_start; j <= pc.dir_end; j += pc.dir_incr) {
			where = pc.ps;
			rankinc = rankincr(j);
			fileinc = fileincr(j);
			where.rank += rankinc;
			where.file += fileinc;
			for (k = 1; k <= pc.dirs[j & 7]; k++, where.rank += rankinc, where.file += fileinc) {
				if (vanilla_can_move(pc, where)) {
					if (DEBUG_MOVES & DEBUG_GAMEND) {
						printf("%c at %c%c can move to %c%c, not stalemate\n", pc.piece, pc.ps.file + 'a', pc.ps.rank + '1', where.file + 'a', where.rank + '1');
					}
					return 0;
				}
			}
		}
	}

	/* look for king moves */
	pc = player[0];
	for (j = pc.dir_start; j <= pc.dir_end; j++) {
		if ((!(pc.pin_dir & (1 << (j)))) && pc.dirs[j & 7] == 1 && !isSame(pc.piece, pc.end[j & 7])) {
			return 0;
		}
	}
	/* castling not considered specially because for castling to work, king should be able to move one space in that direction */

	/* no moves possible: stalemate */
	return 1;
}

int is_draw(chessboard *board, chesset *set) {
	return ((board->white_reps == 6) && (board->black_reps == 6)) || insufficient_mating_material(set) || is_stalemate(board, set);
}

/* call before move is made */
void update_repetition(chessboard *board, move mv) {
	square from = board->brd[mv.ini.rank][mv.ini.file];
	square to = board->brd[mv.fin.rank][mv.fin.file];
	move *pmv = NULL;
	move temp;
	usint *count = NULL;
	/* called before colors are flipped */
	if (board->player == 'w') {
		pmv = &(board->whiterep);
		count = &(board->white_reps);
	}
	else if (board->player == 'b') {
		pmv = &(board->blackrep);
		count = &(board->black_reps);
	}

	/* opposite of last move */
	temp.fin = (*pmv).ini;
	temp.ini = (*pmv).fin;

	if (movesequal(temp, mv)) {
		/* opposite of last move made */
		*count += 1;
	}
	else {
		/* new move made */
		*count = 1;
	}
	if ((to.pc) || (toupper(from.pc) == 'P')) {
		/* if capture, make final position 'out of the board'. this way, opposite of this move cannot exist */
		pmv->fin.rank = pmv->fin.file = 8;
	}
	else {
		*pmv = mv;
	}
}

int insufficient_mating_material(chesset *set) {
	char p1;

	if ((set->n_white + set->n_black) == 2) {
		/* two kings */
		return 1;
	}

	if ((set->n_white + set->n_black) == 3) {
		/*king and N or B vs king */
		p1 = (set->n_white == 2) ? set->whites[1].piece : set->blacks[1].piece;
		p1 = toupper(p1);
		if (p1 == 'B' || p1 == 'N') {
			return 1;
		}
	}

	if ((set->n_white == 2) && (set->n_black == 2) && (set->whites[1].piece == toupper(set->blacks[1].piece)) && (same_color_square(set->whites[1].ps, set->blacks[1].ps))) {
		/* kings and same colored bishops */
		return 1;
	}

	return 0;
}

void show_repetition(chessboard *board) {
	printf("White: %d\t", board->white_reps); print_move(board->whiterep);
	printf("Black: %d\t", board->black_reps); print_move(board->blackrep);
}
