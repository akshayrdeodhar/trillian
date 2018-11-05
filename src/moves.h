#ifndef MOVES_H
#define MOVES_H

#include "board.h"
#include "pieces.h"

int can_attack(piece p, position ps); /* whether position ps is in piece p's ATTACK RANGE */

int vanilla_can_move(piece p, position ps); /* checks whether piece can move to specified position without considering threats to king */

int can_move(chessboard *board, chesset *set, move mv); /* checks whether specified move is legal for given board position */

void calculate_piece(piece *p, chessboard *ch); /* calculate sliding movements in all directions of piece, store them in piece */

void calculate_all(chesset *set, chessboard *ch); /* calculates sliding movements for all sliding pieces on the board */

void calculate_direction(piece *p, chessboard *ch, usint direction); /* calculates sliding movement of piece in 'direction' */

void update_set(chesset *set, chessboard *ch, move latest); /* updates sliding moves based on latest move */

void update_pieces(chessboard *board, chesset *set, move mv); /* if move affects a sliding piece, update it's slide */

void update_piece(chessboard *board, piece *p, move mv); /* if move affects a sliding piece, update it's slide */

void debug_calculation(chesset *set, chessboard *board); /* enumerate stored sliding moves for checking- not to be used in main() */

void debug_piece_calculations(chessboard *moves, piece p); /* enumerate moves of piece 'p' for visual checking- called from debug_calculations */

special_move make_move(chessboard *board, chesset *set, move mv); /* make move, modify state of board, set metadata modify indexes in case of kill*/

void menial_move(chessboard *board, chesset *set, move mv); /* simply move piece on board, set position in piece (called by make_move) */

void check_manually(chesset s); /* check whether knight moves are calculated accurately, DEBUGGING, REMOVE */

void calculate_pins(chesset *set, chessboard *ch, char color); /* calculate pin directions of all pieces in set */

void calculate_threats(chesset *set, char color); /* calculate which neightbouring squares of king are attcked, and checks to king */

void enumpins(chesset *set); /* enumerate pins */

void show_threats(chesset *set, chessboard *board); /* display which squares around king are attacked, check information */

void attack_moves(piece p, chessboard *board); /* show all squares attacked by piece on board */

void attack_bitboard(chesset *set, chessboard *board); /* show all squares attacked by all pieces */

void moves(piece p, chessboard *board); /* show all squares to which piece can move on board */

void moves_bitboard(chesset *set, chessboard *board); /* show all squares to which all pieces in set can move on bitboard */

move rook_move(special_move king_castle); /* return rook move for castle */

move king_castle(special_move castle); /* returns king move for castle */

int can_castle(chessboard *board, chesset *set, special_move castle); /* check whether specified castling is legal */

special_move check_special(square sq, move mv); /* check whether move is castling or promotion, return it's code */
			
void handle_promotion(chessboard *board, chesset *set, move mv, char promoted); /* promote the pawn to chosen piece */

void show_register(usint word); /* show 8 bit register of flags */

int is_checkmate(chessboard *board, chesset *set); /* whether a king is in checkmate */

int is_stalemate(chessboard *board, chesset *set); /* check whether stalemate */

int insufficient_mating_material(chesset *set); /* checks for insufficient mating material */

int is_draw(chessboard *board, chesset *set); /* checks all draw possibilites */

void update_repetition(chessboard *board, move mv);

void show_repetition(chessboard *board);

#endif
