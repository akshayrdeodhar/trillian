#ifndef TRILLIAN_H
#define TRILLIAN_H

#include "board.h"
#include "pieces.h"
#include "moves.h"

typedef struct {
	move mov;
	double score;
}branch;

double value(char pc); 

double color_evaluate(piece *side, int n); 

double position_evaluate(chesset *set); 

move trillian(chessboard *board, chesset *set);

branch greater_trillian(chessboard board, chesset set, unsigned depth);

#endif
