#include <stdlib.h>
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
	usint direction, dist;
	ssint rankinc, fileinc;
	movement sl;
	position save, where;
	move mv;
	chessboard show;

	zn = 0;

	if (board->player == 'w')  {
		zaphod = set->whites;
		zn = set->n_white;
	}
	else if (board->player == 'b') {
		zaphod = set->blacks;
		zn = set->n_black;
	}
	else {
		zaphod = NULL;
		zn = 0;
	}

	/* calculate king moves first, as routine for this does not change based on whether king is in check */
	show = *board; /* all show to starboard */
	pc = zaphod[0]; 
	mv.ini = pc.ps;
	show_register(pc.pin_dir);
	for (i = pc.dir_start; i <= pc.dir_end; i++) {
		if ((!(pc.pin_dir & (1 << i))) && (!isSame(pc.piece, pc.end[i & 7]))) {
			mv.fin.rank = pc.ps.rank + rankincr(i);
			mv.fin.file = pc.ps.file + fileincr(i);
			if (inrange(mv.fin.rank, mv.fin.file)) {
				show.brd[mv.fin.rank][mv.fin.file].pc = 'M';
				aappend(a, mv);
			}
		}
	}
	display(show, MOVES_MODE);

	/* single check, calculate possible blocking or evasion moves */
	if (set->threat_count == 1) {
		king = zaphod[0];
		sl = find_movement(king.ps, set->threat_source);
		direction = find_dir(sl);
		rankinc = rankincr(direction);
		fileinc = fileincr(direction);
		save.rank = king.ps.rank + rankinc;
		save.file = king.ps.file + fileinc;
		dist = distance(sl, direction);
		for (i = 1; i < zn; i++) {
			show = *board;
			/* for each friendly piece */
			pc = zaphod[i];
			mv.ini = pc.ps;
			for (j = 1; j <= dist; j++, save.rank += rankinc, save.file += fileinc) {
			/* for each 'natural' square in line joining king and source of check */
				if (vanilla_can_move(zaphod[j], save)) {
					mv.fin = save;
					show.brd[mv.fin.rank][mv.fin.file].pc = 'M';
					aappend(a, mv);
					/* if can block or kill */
				}
			}
			display(show, MOVES_MODE);
		}
		/* no moves other than these remove check */
		return;
	}
	
	
	for (i = 1; i < zn; i++) {
		/* for each piece */
		show = *board;
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
					show.brd[where.rank][where.file].pc = 'M';
				}
			}
		}
		display(show, MOVES_MODE);
	}
}

