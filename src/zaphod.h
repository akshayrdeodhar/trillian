#ifndef ZAPHOD_H
#define ZAPHOD_H

#define COMP_NAME ("Zaphod")

#include "board.h"
#include "pieces.h"
#include "array.h"

void generate_moves(chessboard *board, chesset *set, array *a);
move zaphod(chessboard *board, chesset *set);

#endif
