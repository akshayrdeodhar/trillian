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

int can_attack(piece p, position ps); /* whether position ps is in piece p's ATTACK RANGE */

int can_move(chessboard board, chesset set, move mv);

void calculate_piece(piece *p, chessboard ch); /* calculate sliding movements in all directions of piece, store them in piece */

void calculate_all(chesset *set, chessboard ch); /* calculates sliding movements for all sliding pieces on the board */

void calculate_direction(piece *p, chessboard ch, usint direction); /* calculates sliding movement of piece in 'direction' */

void update_set(chesset *set, chessboard ch, move latest); /* updates sliding moves based on latest move */

void update_pieces(chessboard board, chesset *set, move mv); /* if move affects a sliding piece, update it's slide */

void update_piece(chessboard board, piece *p, move mv); /* if move affects a sliding piece, update it's slide */

void debug_calculation(chesset set, chessboard board); /* enumerate stored sliding moves for checking- not to be used in main() */

void debug_piece_calculations(chessboard *moves, piece p); /* enumerate moves of piece 'p' for visual checking- called from debug_calculations */

castle_move make_move(chessboard *board, chesset *set, move mv); /* make move, modify state of board, set metadata modify indexes in case of kill*/

void menial_move(chessboard *board, chesset *set, move mv); /* simply move piece on board, set position in piece (called by make_move) */

void check_manually(chesset s);

void calculate_pins(chesset *set, chessboard ch, char color);

void calculate_threats(chesset *set, char color);

void enumpins(chesset set);

void show_threats(chesset set, chessboard board);

void attack_moves(piece p, chessboard board);

void attack_bitboard(chesset set, chessboard board);

void moves(piece p, chessboard board);

void moves_bitboard(chesset set, chessboard board);

move rook_move(castle_move king_castle);

int can_castle(chessboard board, chesset set, castle_move castle);

castle_move check_castling(square sq, move mv);

void show_register(usint word);

int is_checkmate(chessboard board, chesset set);

#endif
