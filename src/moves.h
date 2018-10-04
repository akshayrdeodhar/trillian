#ifndef MOVES_H
#define MOVES_H

#include "board.h"
#include "pieces.h"

typedef struct {
	usint dx, dy;
}movement;

/*
   typedef struct {
   piece white[16];
   usint n_white;
   piece black[16];
   usint n_black;
   }pieces;
   */

typedef struct {
	square ini;
	square fin;
}move;

movement find_movement(move mv);

usint find_dir(movement sl); 

ssint xincr(usint direction);

ssint yincr(usint direction);

int same_team(piece p, piece q);

int can_move(piece p, square sq, chessboard ch);

usint slide_distance(usint direction);

int can_slide(piece p, square sq);

int knight_move(piece q, square sq);

int pawn_move(piece q, square sq);

void calculate(piece *p, chessboard ch);

void update_slide(piece *p, chessboard ch, usint direction);

#endif
