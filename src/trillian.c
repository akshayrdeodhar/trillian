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

/* support-attack:
 * it is good for a piece of low value to attack or support a piece of hig value:
 * linear equation thatt maps (-8, 8) to (0, 1)
 * y = (0.5) + (x) / 16
 * */

branch maximise(chessboard board, chesset set, branch alphawhite, branch betablack, unsigned depth) {
	int i;
	move mv, rook_castle;
	char promoted;
	branch temp;
	chessboard sboard;
	chesset sset;
	special_move castle;
	int score;

	array a;
	ainit(&a);

	if (depth == 0) {
		score = position_evaluate(&set);
		temp.score = score;
		return temp;
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
			score = (board.player == 'w' ? 1e9 : -1e9);
			temp.mov = mv;
			temp.score = score;
			return temp;
		}
		else if (is_draw(&sboard, &sset)) {
			temp.score = 0;
			temp.mov = mv;
			return temp;
		}
		else {
			temp = minimise(sboard, sset, alphawhite, betablack, depth - 1);
			temp.mov = mv;
		}

		if (temp.score > alphawhite.score) {
			alphawhite = temp;
		}

		if (alphawhite.score >= betablack.score) {
			/* in this subtree, afinal >= acurr
			 * black has a subtree with betablack < acurr
			 * so why will black descend?
			 * */
			return alphawhite;
		}
	}
	return alphawhite;
}

branch minimise(chessboard board, chesset set, branch alphawhite, branch betablack, unsigned depth) {
	int i;
	move mv, rook_castle;
	char promoted;
	branch temp;
	chessboard sboard;
	chesset sset;
	special_move castle;
	int score;

	array a;
	ainit(&a);

	if (depth == 0) {
		score = position_evaluate(&set);
		temp.score = score;
		return temp;
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
			score = (board.player == 'w' ? 1e9 : -1e9);
			temp.mov = mv;
			temp.score = score;
			return temp;
		}
		else if (is_draw(&sboard, &sset)) {
			temp.score = 0;
			temp.mov = mv;
			return temp;
		}
		else {
			temp = maximise(sboard, sset, alphawhite, betablack, depth - 1);
			temp.mov = mv;
		}

		if (temp.score < betablack.score) {
			betablack = temp;
		}

		if (betablack.score <= alphawhite.score) {
			/* betablack with comprehensive anylisis
			 * bfinal <= bcurr
			 * But white has a alphawhite >= bcurr
			 * So why descend?
			 * In the end, bfinal < alphawhite, so white won't chose this anyway!
			 * */
			return betablack;
		}
	}
	return betablack;
}

branch distributed_trillian(chessboard board, chesset set, branch alphawhite, branch betablack, unsigned depth) {
	if (board.player == 'w') {
		return maximise(board, set, alphawhite, betablack, depth);
	}
	else {
		return minimise(board, set, alphawhite, betablack, depth);
	}
}
