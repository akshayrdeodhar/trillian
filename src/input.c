#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "moves.h"
#include "defs.h"

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
