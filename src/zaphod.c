#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "zaphod.h"
#include "moves.h"
#include "board.h"
#include "pieces.h"
#include "defs.h"
#include "array.h"
#include "vector.h"
#include "display.h"

#define DEBUG_BOARD 1

void generate_moves(chessboard *board, chesset *set, array *a) {
	piece *zaphod = NULL;
	piece pc, king;
	int zn, i, j, k;
	special_move castle_k;
	usint direction, dist;
	ssint rankinc, fileinc;
	movement sl;
	position save, where;
	move mv;

	zn = 0;

	if (board->player == 'w')  {
		zaphod = set->whites;
		zn = set->n_white;
		castle_k = white_kingside;
	}
	else if (board->player == 'b') {
		zaphod = set->blacks;
		zn = set->n_black;
		castle_k = black_kingside;
	}
	else {
		zaphod = NULL;
		zn = 0;
	}

	/* calculate king moves first, as routine for this does not change based on whether king is in check */
	pc = zaphod[0]; 
	mv.ini = pc.ps;
	show_register(pc.pin_dir);
	for (i = pc.dir_start; i <= pc.dir_end; i++) {
		if ((!(pc.pin_dir & (1 << i))) && (!isSame(pc.piece, pc.end[i & 7]))) {
			mv.fin.rank = pc.ps.rank + rankincr(i);
			mv.fin.file = pc.ps.file + fileincr(i);
			if (inrange(mv.fin.rank, mv.fin.file)) {
				aappend(a, mv);
			}
		}
	}

	/* single check, calculate possible blocking or evasion moves */
	if (set->threat_count == 1) {
		king = zaphod[0];
		printf("Threat from %c%c\n", set->threat_source.file + 'a', set->threat_source.rank + '1');
		sl = find_movement(king.ps, set->threat_source);
		direction = find_dir(sl);
		rankinc = rankincr(direction);
		fileinc = fileincr(direction);
		save.rank = king.ps.rank + rankinc;
		save.file = king.ps.file + fileinc;
		dist = distance(sl, direction);
		for (i = 1; i <= dist; i++, save.rank += rankinc, save.file += fileinc) {
			/* for each 'natural' square in line joining king and source of check */
			for (j = 0; j < zn; j++) {
				/* for each friendly piece */
				pc = zaphod[j];
				mv.ini = pc.ps;
				if (vanilla_can_move(pc, save)) {
					mv.fin = save;
					print_move(mv);
					aappend(a, mv);
					/* if can block or kill */
				}
			}
		}
		/* no moves other than these remove check */
		return;
	}

	if (can_castle(board, set, castle_k)) {
		mv = king_castle(castle_k);
		print_move(mv);
		aappend(a, mv);
	}
	if (can_castle(board, set, castle_k + 1)) {
		mv = king_castle(castle_k + 1);
		print_move(mv);
		aappend(a, mv);
	}
	
	
	for (i = 1; i < zn; i++) {
		/* for each piece */
		pc = zaphod[i];
		for (j = pc.dir_start; j <= pc.dir_end; j += pc.dir_incr) {
			/* for each direction in which piece moves */
			mv.ini = pc.ps;
			where = pc.ps;
			rankinc = rankincr(j);
			fileinc = fileincr(j);
			where.rank += rankinc;
			where.file += fileinc;
			for (k = 1; k <= pc.dirs[j & 7]; k++, where.rank += rankinc, where.file += fileinc) {
				/* for each movement in direction */
				if (vanilla_can_move(pc, where)) {
					mv.fin = where;
					aappend(a, mv);
				}
			}
		}
	}
}


/* takes board and set with all calculations done, returns a random move, beeblebrox style */
move zaphod(chessboard *board, chesset *set) {
	struct timeval random_time;
	gettimeofday(&random_time, NULL);
	srand(random_time.tv_sec);
	array a;
	int x;
	ainit(&a);
	generate_moves(board, set, &a);
	int n = alength(&a);
	for (x = 0; x < n; x++) {
		print_move(a.arr[x]);
	}
	int i = rand() % n;
	move mv = a.arr[i];
	adestroy(&a);
	return mv;
}

