#ifndef INPUT_H
#define INPUT_H
#include "player.h"

#define MAX 16

enum commandtype {
	move_ins,
	game_ins,
	filename_ins
};

typedef enum commandtype command;

typedef struct {
	command c;
	move mv;
	usint instruction;
	char string[MAX];
}token;

move extract_move(char string[]);
unsigned readline(char string[], unsigned maxlen);
char get_promotion(char player);
int get_gamemode(void);
player_token get_player(char col);

#endif



