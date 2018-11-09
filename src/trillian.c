#include "trillian.h"
#include "zaphod.h"
#include <stdio.h>
#include "moves.h"
#include "display.h"
#include <limits.h>
#include "display.h"

#define DEBUG_EVAL 0
#define DEBUG_DEEP 0

double value(char pc) {
	switch(toupper(pc)) {
		case 'Q':
			return 9;
			break;
		case 'R':
			return 5;
			break;
		case 'B': case 'N':
			return 3;
			break;
		case 'P':
			return 1;
			break;
		default:
			return 0;
			break;
	}
	return 0;
}

#define VALCOEFF 15
#define KINGCOEFF (-3)
#define CTRLCOEFF (1)
#define SACOEFF (1)
double color_evaluate(piece *side, int n) {
	int i, j;
	piece pc;
	double valsum, ctrlsum, kingsafety, ctrltemp, supportattack, satemp;

	kingsafety = 0;
	pc = side[0];
	for (i = 0; i < 8; i++) {
		if (pc.pin_dir & (1 << i)) {
			kingsafety += 1;
		}
	}

	valsum = 0;
	ctrlsum = 0;
	supportattack = 0;
	for (i = 1; i < n; i++) {
		ctrltemp = 0;
		satemp = 0;
		pc = side[i];
		valsum += value(pc.piece);

		for (j = pc.dir_start; j <= pc.dir_end; j += pc.dir_incr) {
			ctrltemp += pc.dirs[j & 7];
			satemp += (0.5 + (value(pc.end[j & 7]) - value(pc.piece)) / 16); /* see end of file for equation */
		}
		ctrlsum += ctrltemp;
		supportattack += satemp;
	}

	return VALCOEFF * valsum + CTRLCOEFF * ctrlsum + SACOEFF * supportattack + KINGCOEFF * kingsafety;
}

double position_evaluate(chesset *set) {
	double score, white, black;
	white = color_evaluate(set->whites, set->n_white);
	black = color_evaluate(set->blacks, set->n_black);
	score = white - black;
	return score;
}

#define MINMAX(curr, minmax, color) ((color == 'w') ? ((curr) > (minmax)) : ((curr) < (minmax)))
branch greater_trillian(chessboard board, chesset set, unsigned depth) {
	int i;
	move mv, rook_castle;
	char promoted;
	branch temp;
	chessboard sboard;
	chesset sset;
	special_move castle;
	double score, minmax;
	branch br;

	array a;
	ainit(&a);

	if (depth == 0) {
		score = position_evaluate(&set);
		br.score = score;
		return br;
	}

	minmax = ((board.player == 'w') ? INT_MIN : INT_MAX);
	generate_moves(&board, &set, &a);
		br.score = minmax;
	int n = alength(&a);
	for (i = 0; i < n; i++) {
		mv = a.arr[i];

		sboard = board;
		sset = set;

		castle = make_move(&sboard, &sset, mv);
		if (white_kingside <= castle && black_queenside >= castle) {
			rook_castle = rook_move(castle);
			menial_move(&sboard, &sset, rook_castle);
			update_pieces(&sboard, &sset, rook_castle);
		}
		else if (castle == promotion) {
			/* always promotes to queen, for lack of a better logic */
			promoted = (sboard.player == 'w' ? 'q' : 'Q');
			handle_promotion(&sboard, &sset, mv, promoted);

		}

		/* recalculate */
		update_pieces(&sboard, &sset, mv);
		calculate_pins(&sset, &sboard, 'w');
		calculate_pins(&sset, &sboard, 'b');
		calculate_threats(&sset, sboard.player);

		/* check for end of game */
		if (is_checkmate(&sboard, &sset)) {
			score = (board.player == 'w' ? (INT_MAX - 1) : (INT_MIN + 1)); /* to prevent collisions with 'INF values */
			br.mov = mv;
			br.score = score;
			return br;
		}
		else if (is_draw(&sboard, &sset)) {
			temp.score = 0;
			temp.mov = mv;
		}
		else {
			temp = greater_trillian(sboard, sset, depth - 1);
			temp.mov = mv;
		}

		if (MINMAX(temp.score, br.score, board.player)) {
			br = temp;
		}
	}
	return br;
}


branch smarter_trillian(chessboard board, chesset set, branch bestwhite, branch bestblack, unsigned depth) {
	int i;
	move mv, rook_castle;
	char promoted;
	branch temp;
	chessboard sboard;
	chesset sset;
	special_move castle;
	int score;
	branch br;

	array a;
	ainit(&a);

	if (depth == 0) {
		score = position_evaluate(&set);
		br.score = score;
		return br;
	}

	generate_moves(&board, &set, &a);
	int n = alength(&a);

	for (i = 0; i < n; i++) {
		mv = a.arr[i];

		sboard = board;
		sset = set;

		castle = make_move(&sboard, &sset, mv);
		if (white_kingside <= castle && black_queenside >= castle) {
			rook_castle = rook_move(castle);
			menial_move(&sboard, &sset, rook_castle);
			update_pieces(&sboard, &sset, rook_castle);
		}
		else if (castle == promotion) {
			/* always promotes to queen, for lack of a better logic */
			promoted = (sboard.player == 'w' ? 'q' : 'Q');
			handle_promotion(&sboard, &sset, mv, promoted);

		}

		/* recalculate */
		update_pieces(&sboard, &sset, mv);
		calculate_pins(&sset, &sboard, 'w');
		calculate_pins(&sset, &sboard, 'b');
		calculate_threats(&sset, sboard.player);

		/* check for end of game */
		if (is_checkmate(&sboard, &sset)) {
			score = (board.player == 'w' ? INT_MAX : INT_MIN);
			br.mov = mv;
			br.score = score;
			return br;
		}
		else if (is_draw(&sboard, &sset)) {
			temp.score = 0;
			temp.mov = mv;
		}
		else {
			temp = greater_trillian(sboard, sset, depth - 1);
			temp.mov = mv;
		}

		if (board.player == 'w' && temp.score > bestwhite.score) {
			bestwhite = temp;
			/* black will never pick this */
			if (bestwhite.score >= bestblack.score) {
				return bestwhite;
			}
		}
		else if (board.player == 'b' && temp.score < bestblack.score) {
			bestblack = temp;
			/* white will never pick this */
			if (bestblack.score <= bestwhite.score) {
				return bestblack;
			}
		}
	}
	if (board.player == 'w') {
		return bestwhite;
	}
	else {
		return bestblack;
	}

}

/* support-attack:
 * it is good for a piece of low value to attack or support a piece of hig value:
 * linear equation thatt maps (-8, 8) to (0, 1)
 * y = (0.5) + (x) / 16
 * */
