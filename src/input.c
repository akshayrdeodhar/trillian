#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "moves.h"
#include "defs.h"
#include "input.h"
#include "player.h"

unsigned readline(char string[], unsigned maxlen, FILE *fp) {
	int i = 0;
	char c;

	while(((c = fgetc(fp)) != EOF) && (c != '\n') && (i < maxlen)) {
		string[i++] = c;
	}
	string[i] = '\0';
	while(c != '\n' && c != EOF) {
		c = fgetc(fp);
	}
	
	return i;
}

/* always run is_move before extract_move */
move extract_move(char string[]) {
	move mv;

	usint r1, r2, f1, f2;
	f1 = string[0] - 'a';
	r1 = string[1] - '1';
	f2 = string[3] - 'a';
	r2 = string[4] - '1';

	mv.ini.rank = r1;
	mv.ini.file = f1;
	mv.fin.rank = r2;
	mv.fin.file = f2;

	return mv;
}

char get_promotion(char player) {
	printf("Today is your lucky day!\n");
	char promote_to = '\0';
	char valid = 0;
	char line[8];
	int n;
	while(!valid) {
		printf("Promote to ? (Q, R, B, N, q, r, b, n)\n");
		n = readline(line, 8, stdin);
		if (n > 1) {
			continue;
		}
		promote_to = line[0];
		switch(toupper(promote_to)) {
			case 'Q': case 'R': case 'N': case 'B':
				valid = 1;
				break;
			default:
				printf("Cannot promote to %c\n", promote_to);
				break;
		}
	}
	return (player == 'w' ? toupper(promote_to) : tolower(promote_to));
}

/* prompts user to choose game mode 
 * TODO: Place a cheat-code for 'testing mode'
 * */
int get_gamemode(void) {
	char line[16];
	char no;
	int pole = 0;
	int n;
	printf("Hello gentlemen: All your base are belong to us!\n You can:\nPlay chess with a superior being (Press 1)\nPlay chess with a lower being of your species (Press 2)\n There is no option 3.\n");
	n = readline(line, 16, stdin);
	no = line[0];
	while((n > 1 || (no != 'C' && no != 'c' && no != '1' && no != '2')) && pole < 5) {
		printf("Thou shalt choose 1 or 2\nChoose:");
		n = readline(line, 16, stdin);
		no = line[0];
		pole++;
	}

	if (no == '1' || no == '2') {
		return no - '0';
	}

	if (no == 'c') {
		/* c for cheat code- testing mode */
		return 9;
	}
	else if (no == 'C') {
		return 6;
	}

	printf("Thou art indeed a ReBeL\n");

	return 0;
}

/* creates a player token for player with a fake, old english manner. 
 * if col is NUL, prompts for color. else chooses color automatically based on col
 * TODO: hardening against rebellious users
 * */
player_token get_player(char col) {
	player_token tk;
	char line[8];
	int n, pole = 0;
	char cl;
	printf("What art thou called? (Thy name shalt not exceed 15 characters)\n");
	n = readline(tk.name, 16, stdin);
	printf("%d\n", n);
	while(n > 15 && pole < 5) {
		printf("THY NAME SHALT NOT EXCEED FIFTEEN CHARACTERS\n");
		n = readline(tk.name, 16, stdin);
		pole++;
	}
	tk.type = HUMAN;

	if (col) {
		printf("%s, thou shalt not choose your color\n", tk.name);
		tk.color = (col == 'w') ? 'b' : 'w';
		return tk;
	}

	printf("What color dost thou choose?\n w - white\tb - black\n(Psst- white gets to play first)\n");
	n = readline(line, 8, stdin);
	cl = line[0];
	pole = 0;
	while((n > 1 || (cl != 'w' && cl != 'b')) && pole < 5) {
		printf("Thou shalt choose w or b, thou descendent of the ape\n");
		n = readline(line, 8, stdin);
		cl = line[0];
		pole++;
	}

	if (cl != 'w' && cl != 'b') {
		printf("I grow weary of this.\n");
		tk.color = '\0';
		return tk;
	}

	printf("Very well, %s\n", tk.name);
	tk.color = cl;

	return tk;
}

int is_move(char line[]) {
	usint r1, r2, f1, f2;

	if (strlen(line) != 5 || line[2] != '-') {
		/* definitely not in format fr-fr */
		return 0;
	}

	f1 = line[0] - 'a';
	r1 = line[1] - '1';
	f2 = line[3] - 'a';
	r2 = line[4] - '1';
	if (!(inrange(f1, r1) && inrange(f2, r2))) {
		return 0;
	}

	return 1;
}

token get_command(char line[]) {
	token tk;
	tk.c = invalid_ins;
	if (!(strcmp(line, "quit"))) {
		tk.c = quit_ins;
	}
	else if (!strcmp(line, "save")) {
		tk.c = save_ins;
	}
	else if (!strcmp(line, "draw")) {
		tk.c = draw_ins;
	}
	else if (!strcmp(line, "board")) {
		tk.c = board_ins;
	}
	else if (is_move(line)) {
		tk.c = move_ins;
		tk.mv = extract_move(line);
	}
	else {
		tk.c = invalid_ins;
	}

	return tk;
}
