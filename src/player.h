#ifndef PLAYER_H
#define PLAYER_H

#define HUMAN 1
#define COMPUTER 2

typedef struct {
	char name[16];
	char type;
	char color;
}player_token;

void player_info_string(char save_string[], player_token pw, player_token pb);
int string_to_players(char save_string[], player_token *ppw, player_token *ppb);
int extract_token(player_token *ptk, char pstring[]);

#endif
