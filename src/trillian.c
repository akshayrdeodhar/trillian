#include "trillian.h"
#include "zaphod.h"
#include <stdio.h>
#include "moves.h"
#include "display.h"
#include <limits.h>

#define DEBUG_EVAL 0
#define DEBUG_DEEP 0

int value(char pc) {
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


#define VALCOEFF 10
#define KINGCOEFF (-2)
#define CTRLCOEFF (1)
int color_evaluate(piece *side, int n) {
	int i, j;
	piece pc;
	int valsum, ctrlsum, kingsafety, ctrltemp;

	kingsafety = 0;
	pc = side[0];
	for (i = 0; i < 8; i++) {
		if (pc.pin_dir & (1 << i)) {
			kingsafety += 1;
		}
	}
#if DEBUG_EVAL
	printf("Kingsafety: %lf\n", kingsafety);
#endif

	valsum = 0;
	for (i = 1; i < n; i++) {
		pc = side[i];
		valsum += value(pc.piece);
	}
#if DEBUG_EVAL
	printf("Valsum: %lf\n", valsum);
#endif

	ctrlsum = 0;
	for (i = 1; i < n; i++) {
		ctrltemp = 0;
		pc = side[i];
		for (j = pc.dir_start; j <= pc.dir_end; j += pc.dir_incr) {
			ctrltemp += pc.dirs[j & 7];
		}
		ctrlsum += ctrltemp;
	}
#if DEBUG_EVAL
	printf("Ctrlsum: %lf\n", ctrlsum);
#endif

	return VALCOEFF * valsum + CTRLCOEFF * ctrlsum + KINGCOEFF * kingsafety;
}

int position_evaluate(chesset *set) {
	int score, white, black;
	white = color_evaluate(set->whites, set->n_white);
	black = color_evaluate(set->blacks, set->n_black);
	score = white - black;
	return score;
}


#define MINMAX(curr, minmax, color) ((color == 'w') ? ((curr) > (minmax)) : ((curr) < (minmax)))
move trillian(chessboard *board, chesset *set) {
	int i;
	move mv, rook_castle;
	char promoted;
	chessboard sboard;
	chesset sset;
	special_move castle;
	array a;
	ainit(&a);
	array b;
	ainit(&b);
	generate_moves(board, set, &a);
	int score;
	int minmax = ((board->player == 'w') ? INT_MIN : INT_MAX);
	move bestmove;
	int n = alength(&a);
	for (i = 0; i < n; i++) {
		mv = a.arr[i];

		score = 0;
		sboard = (*board);
		sset = (*set);

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
			score = (board->player == 'w' ? INT_MAX : INT_MIN);
		}
		else if (is_draw(&sboard, &sset)) {
		}
#if DEBUG_DEEP
		filled_display(sboard, MOVES_MODE);
		ainit(&b);
		generate_moves(sboard, sset, &b);
		nb = alength(&b);
		printf("For"); print_move(a.arr[i]);
		for (k = 0; k < nb; k++) {
			printf("\t"); print_move(b.arr[k]);
		}
		adestroy(&b);
#endif

		score += position_evaluate(&sset);

		printf("minmax: %d\n", minmax);
		printf("%c%c-%c%c: %d\n", mv.ini.file + 'a', mv.ini.rank + '1', mv.fin.file + 'a', mv.fin.rank + '1', score);
		if (MINMAX(score, minmax, board->player)) {
			minmax = score;
			bestmove = mv;
		}
	}

	return bestmove;
}

branch greater_trillian(chessboard board, chesset set, unsigned depth) {
	int i;
	move mv, rook_castle;
	char promoted;
	branch temp;
	chessboard sboard;
	chesset sset;
	special_move castle;
	int score, minmax;
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
	int score, minmax;
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
	else if (board.player == 'b') {
		return bestblack;
	}
}