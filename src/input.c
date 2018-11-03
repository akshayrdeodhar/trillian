#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "moves.h"
#include "defs.h"
#include "input.h"
#include "player.h"

unsigned readline(char string[], unsigned maxlen) {
	int i = 0;
	char c;

	while(((c = getchar()) != EOF) && (c != '\n') && (i < maxlen - 1)) {
		string[i++] = c;
	}
	string[i] = '\0';
	
	return i;
}

move extract_move(char string[]) {
	move mv;
	mv.ini.rank = mv.fin.rank = mv.ini.file = mv.fin.file = 8;
	int len = strlen(string);
	if (len != 5 || string[2] != '-') {
		return mv;
	}
	
	usint r1, r2, f1, f2;
	f1 = string[0] - 'a';
	r1 = string[1] - '1';
	f2 = string[3] - 'a';
	r2 = string[4] - '1';
	if (!(inrange(f1, r1) && inrange(f2, r2))) {
		return mv;
	}

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
	while(!valid) {
		printf("Promote to ? (Q, R, B, N, q, r, b, n)\n");
		promote_to = getchar();
		getchar();
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
	char no, next;
	int pole = 0;
	printf("Hello gentlemen: All your base are belong to us!\n You can:\nPlay chess with a superior being (Press 1)\nPlay chess with a lower being of your species (Press 2)\n There is no option 3.\n");
	no = getchar();
	next = getchar();
	while(no != 'c' && no != '1' && no != '2' && pole < 5) {
		if (next != '\n');
		putchar('\n');
		printf("Thou shalt choose 1 or 2\nChoose:");
		no = getchar();
		next = getchar();
		pole++;
	}

	if (no == '1' || no == '2') {
		return no - '0';
	}

	if (no == 'c') {
		/* c for cheat code- testing mode */
		return 42;
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
	int n, pole = 0;
	char cl, next;
	printf("What art thou called? (Thy name shalt not exceed 15 characters)\n");
	n = readline(tk.name, 15);
	while(n > 15) {
		putchar('\n');
		printf("THY NAME SHALT NOT EXCEED FIFTEEN CHARACTERS\n");
		n = readline(tk.name, 15);
	}
	tk.type = HUMAN;

	if (col) {
		printf("%s, thou shalt not choose your color\n", tk.name);
		tk.color = (col == 'w') ? 'b' : 'w';
		return tk;
	}

	printf("What color dost thou choose?\n w - white\tb - black\n(Psst- white gets to play first)\n");
	cl = getchar();
	next = getchar();

	printf("\n%c\n", cl);
	while(cl != 'w' && cl != 'b' && pole < 5) {
		if (next != '\n');
		putchar('\n');
		printf("Thou shalt choose w or b, thou descendent of the ape\n");
		cl = getchar();
		next = getchar();
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

