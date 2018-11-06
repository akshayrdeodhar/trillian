#include "player.h"
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

/* format
 * <t> <c> <name>
 * t = h or c
 * c = w or b
 * name
 * */
void player_info_string(char save_string[], player_token pw, player_token pb) {
	char string[20];
	sprintf(string, "%d %s", pw.type ,pw.name);
	strcpy(save_string, string);
	strcat(save_string, "\t");
	sprintf(string, "%d %s", pb.type, pb.name);
	strcat(save_string, string);
}

/* because save files are created by program, no modification is required */

int string_to_players(char save_string[], player_token *ppw, player_token *ppb) {
	printf("%s\n", save_string);
	char *tokw, *tokb;
	int code1, code2;
	tokw = strtok(save_string, "\t");
	tokb = strtok(NULL, "\t");
	printf("%s\n", tokw);
	code1 = extract_token(ppw, tokw);
	ppw->color = 'w';
	printf("%s\n", tokb);
	code2 = extract_token(ppb, tokb);
	ppb->color = 'b';

	if ((code1 + code2) < 0) {
		return -1;
	}
	else {
		return 0;
	}

	return -1;
}

int extract_token(player_token *ptk, char pstring[]) {
	char *t1, *t2;
	t1 = strtok(pstring, " ");
	t2 = strtok(NULL, " ");

	if (strlen(t1) != 1) {
		return -1;
	}
	ptk->type = atoi(t1);
	strcpy(ptk->name, t2);

	return 0;
}
