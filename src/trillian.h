#ifndef TRILLIAN_H
#define TRILLIAN_H

#include "board.h"
#include "pieces.h"

double value(char pc); 

double color_evaluate(piece *side, int n); 

double position_evaluate(chesset *set); 

move trillian(chessboard *board, chesset *set);

#endif
