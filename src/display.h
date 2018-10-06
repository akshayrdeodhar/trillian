#ifndef DISPLAY_H
#define DISPLAY_h

#include "board.h"

#define READ_MODE 101
#define MOVES_MODE 202

void display(chessboard board, int mode); /* shows board on stdout. choice of whether to show metadata */

#endif

