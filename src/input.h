#ifndef INPUT_H
#define INPUT_H
#include "player.h"

#define MAX 16

typedef enum commandtype {
	move_ins,
	quit_ins,
	save_ins, 
	draw_ins,
	board_ins,
	invalid_ins
}command;

typedef struct {
	command c;
	move mv;
}token;

move extract_move(char string[]);
unsigned readline(char string[], unsigned maxlen, FILE *fp);
char get_promotion(char player);
int get_gamemode(void);
player_token get_player(char col);
token get_command(char line[]);

#endif



