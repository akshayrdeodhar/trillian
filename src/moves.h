#ifndef MOVES_H
#define MOVES_H

#include "board.h"
#include "pieces.h"

typedef struct {
	usint dfile, drank;
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
	position ini;
	position fin;
}move;

movement find_movement(move mv);

usint find_dir(movement sl); 

ssint fileincr(usint direction);

ssint rankincr(usint direction);

int same_team(piece p, piece q);

int can_move(piece p, position sq, chessboard ch);

usint slide_distance(usint direction);

int can_slide(piece p, position sq);

int knight_move(piece q, position sq);

int pawn_move(piece q, position sq);

void calculate_piece(piece *p, chessboard ch);

void calculate_all(chesset *set, chessboard ch);

void update_slide(piece *p, chessboard ch, usint direction);

void verify_calculation(chesset set, chessboard board);

#endif
