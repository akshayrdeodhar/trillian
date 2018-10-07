#include <stdio.h>
#include <string.h>
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
