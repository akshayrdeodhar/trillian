#ifndef TRILLIAN_H
#define TRILLIAN_H

#include "board.h"
#include "pieces.h"

double value(char pc); 

double color_evaluate(pieces *side, int n); 

double position_evaluate(chesset *set); 

#endif
