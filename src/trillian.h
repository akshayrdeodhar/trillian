#ifndef TRILLIAN_H
#define TRILLIAN_H

#include "board.h"
#include "pieces.h"
#include "moves.h"

typedef struct {
	move mov;
	int score;
}branch;

int value(char pc); 

int color_evaluate(piece *side, int n); 

int position_evaluate(chesset *set); 

move trillian(chessboard *board, chesset *set);

branch greater_trillian(chessboard board, chesset set, unsigned depth);

branch smarter_trillian(chessboard board, chesset set, branch bestwhite, branch bestblack, unsigned depth);

#endif
