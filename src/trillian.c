#include "trillian.h"
#include "zaphod.h"

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

double color_evaluate(pieces *side, int n) {
	int i;
	piece pc;
	double valsum, controlsum, ctrltemp;
	for (i = 1; i < n; i++) {
		pc = side[i].piece;
		ctrltemp = 0;
		valsum += value(pc.piece);
		for (j = 0; j < 8; j++) {
			ctrltemp += pc.dirs[j];
		}
	}

}

double position_evaluate(chesset *set) {
	double score;
	score = color_evaluate(set->whites, set->n_white) - color_evaluate(set->blacks, set->n_black);
	return score;
}
