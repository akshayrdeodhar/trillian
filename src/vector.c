/* position, displacement and direction */

#include "vector.h"
#include <limits.h>

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
	static const ssint file_incr[16] = {-1, -1, 0, 1, 1, 1, 0, -1, -2, -1, 1, 2, 2, 1, -1, -2};

	if (direction < 16) {
		return file_incr[direction];
	}
	else {
		return SCHAR_MIN;
	}

	return SCHAR_MIN;
}

ssint rankincr(usint direction) {
	static const ssint rank_incr[16] = {0, 1, 1, 1, 0, -1, -1, -1, 1, 2, 2, 1, -1, -2, -2, -1};

	if (direction < 16) {
		return rank_incr[direction];
	}
	else {
		return SCHAR_MIN;
	}

	return SCHAR_MIN;
}



