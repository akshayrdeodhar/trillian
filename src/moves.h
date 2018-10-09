#ifndef MOVES_H
#define MOVES_H

#include "board.h"
#include "pieces.h"

typedef struct {
	ssint dfile, drank;
}movement;

typedef struct {
	position ini;
	position fin;
}move;

movement find_movement(position ini, position fin); /* finds change in position */

usint find_dir(movement sl); /* finds sliding direction from 'slope' */

ssint fileincr(usint direction); /* returns change in 'file' for unit movement in 'direction' */

ssint rankincr(usint direction); /* return change 'rank' for unit movement in direction */

int can_move(piece p, position sq, chessboard ch); /* problem: I don't know whether it is white to play or black to play */

usint slide_distance(usint direction); /* ??? */

int can_slide(piece p, position sq); /* can piece 'p' slide to position sq */

int knight_move(piece q, position sq); /* can knight 'q' move to position sq */

int pawn_move(piece q, position sq); /* can pawn 'q' move to position sq : needs enpass_target*/

void calculate_piece(piece *p, chessboard ch); /* calculate sliding movements in all directions of piece, store them in piece */

void calculate_all(chesset *set, chessboard ch); /* calculates sliding movements for all sliding pieces on the board */

void update_slide(piece *p, chessboard ch, usint direction); /* calculates sliding movement of piece in 'direction' */

void update_set(chesset *set, chessboard ch, move latest); /* updates sliding moves based on latest move */

void update_sliding_pieces(chessboard board, chesset *set, move mv); /* if move affects a sliding piece, update it's slide */

void update_sliding_piece(chessboard board, piece *p, move mv); /* if move affects a sliding piece, update it's slide */

void verify_calculation(chesset set, chessboard board); /* enumerate stored sliding moves for checking- not to be used in main() */

void make_move(chessboard *board, chesset *set, move mv); /* make move, modify state of board, set metadata modify indexes in case of kill*/

void menial_move(chessboard *board, chesset *set, move mv); /* simply move piece on board, set position in piece (called by make_move) */

#endif
