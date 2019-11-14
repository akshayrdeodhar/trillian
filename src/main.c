/*  chess
    Copyright (C) 2018  Akshay

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/





/* The main game loop which calls everything */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>

#include "config.h"
#include "board.h"
#include "pieces.h"
#include "display.h"
#include "moves.h"
#include "input.h"
#include "zaphod.h"
#include "trillian.h"
#include "limits.h"


/* the directory where you wish to put savegame files */
#define CONF_SAVES "./"

/* file to be displayed as title screen- location wrt location of executable OR absolute location */
#define CONF_TITLE DATADIR"/trillian/titlescreen.txt"



#ifndef CONF_DEFAULT
#define DEFAULT_PATH "./dat/default.fen"
#else
#define DEFAULT_PATH CONF_DEFAULT
#endif 

#ifndef CONF_TITLE
#define DEFAULT_TITLE "./dat/titlescreen.txt"
#else
#define DEFAULT_TITLE CONF_TITLE
#endif

#ifdef CONF_SAVES
#define SAVE_DIR CONF_SAVES
#else
#define SAVE_DIR "./save/"
#endif

#ifdef CONF_SPECS
#define SPECS CONF_SPECS
#else
#define SPECS 1
#endif

#ifdef CONF_DEPTH
#define DEPTH CONF_DEPTH
#else
#define DEPTH 4
#endif

#define movesequal(mv1, mv2) (posequal(mv1.ini, mv2.ini) && posequal(mv1.fin, mv2.fin))

int main(int argc, char *argv[]) {

	/* strings for files and commands */
	char string[128];
	char path[256];
	char container[512];
	char command[32];
	char error[320];
	int code, code_dash = 0; /* getting status codes from user input functions */

	/* for savegames */
	FILE *fp = NULL;
	FILE *fs = NULL;
	DIR *dirp = NULL;

	chessboard board; /* the board ! */
	chesset set; /* the pieces */

	token ins; /* user instrunction */
#if SPECS
	struct timeval performance1, performance2; /* for trillian's performance measurement */
	double timereq;
#endif

	int players; /* no. of players */
	player_token pw, pb, pt; /* white, black, and temp */
	char color; /* to specify side from which board is to be printed */

	branch min; /* for alpha-beta pruning */
	min.score = -1e10;
	branch max;
	max.score = 1e10;

	move mv, rook_castle; /* the move from the user, for special case of castling */
	special_move castle; /* check and do corresponding rook movement for castling */
	char promoted = '\0'; /* to recieve promotion */

	branch otherai;

	if (argc > 2) {
		fprintf(stderr, "usage: ./chess [<file.fen> or <file.sv>]\n");
		return EINVAL;
	}

	if (argc == 1) {
		fp = fopen(DEFAULT_PATH, "r");
		if (fp == NULL) {
			sprintf(error, "'%s'", DEFAULT_PATH);
			perror(error);
			return errno;
		}
	}
	else {
		fp = fopen(argv[1], "r");
		if(fp == NULL) {
			sprintf(error, "'%s'", argv[1]);
			perror(error);
			return errno;
		}
	}

	code = readline(string, 128, fp);
	if (fenstring_to_board(&board, string) == -1) {
		fprintf(stderr, "corrupt FEN string\n");
		return 1;
	}

	code = readline(string, 128, fp);
	if (code) {
		code_dash = string_to_players(string, &pw, &pb);
	}
	fclose(fp);

	/* show title screen art */
	fp = fopen(DEFAULT_TITLE, "r");
	if (fp == NULL) {
		sprintf(error, "'%s'", DEFAULT_TITLE);
		perror(error);
		return errno;
	}
	char c;
	while((c = getc(fp)) != EOF) fputc(c, stderr);
	fclose(fp);


	if ((!code) || (code_dash == -1)) {
		/* file does not contain player info */
		players = get_gamemode();
		if (!players) {
			fprintf(stderr, "Begone, Cretin\n");
			return 0;
		}

		if (players != 6 && players != 9) {
			pw = get_player('\0');
			if (!pw.color) {
				fprintf(stderr, "Your services are no longer required\n");
				return 0;
			}
			if (players == 2) {
				pb = get_player(pw.color);
				if (!pb.color) {
					fprintf(stderr, "Your services are no longer required\n");
					return 0;
				}
			}
			else {
				pb.type = COMPUTER;
				strcpy(pb.name, COMP_NAME);
				pb.color = (pw.color == 'w') ? 'b' : 'w';
			}

			/* pw should be the player who plays white */
			if (pw.color == 'b') {
				pt = pb;
				pb = pw;
				pw = pt;
			}
		}
		else {
			/* backdoor */
			strcpy(pw.name, "Walter");
			pw.type = (players == 6 ? HUMAN : COMPUTER);
			pw.color = 'w';
			strcpy(pb.name, "Jesse");
			pb.type = pw.type == HUMAN ? COMPUTER : HUMAN;
			pb.color = 'b';
		}
	}

	interface_board_set(&board, &set);

	calculate_all(&set, &board);
	calculate_pins(&set, &board, 'w');
	calculate_pins(&set, &board, 'b');
	calculate_threats(&set, board.player);

	if (is_checkmate(&board, &set) || is_draw(&board, &set)) {
		fprintf(stderr, "Invalid game file, game has already ended\n");
	}

	pt = (board.player == 'w' ? pw : pb);
	/* if computer playing always keep perspective from human side */
	color = (pt.type == COMPUTER) ? ((pt.color == 'w') ? 'b' : 'w') : pt.color;
	filled_display(&board, MOVES_MODE, color);

	while(1) {
		/* zaphod generates moves */

		/* get command from user */
		pt = (board.player == 'w') ? pw : pb;
		if (pt.type == HUMAN) {
			printf("Your turn, %s\nCommand:", pt.name);
			readline(command, 32, stdin);
			ins = get_command(command);
			switch(ins.c) {
				case move_ins:
					mv = ins.mv;
					if (!can_move(&board, &set, mv)) {
						fprintf(stderr, "Invalid Move: %s\n", pt.name);
						continue;
					}
					break;
				case quit_ins:
					return 0;
					break;
				case save_ins:
					dirp = opendir(SAVE_DIR);
					if (dirp == NULL) {
						sprintf(error, "'%s'", SAVE_DIR);
						perror(error);
						fprintf(stderr, "continuing game\n");
						continue;
					}
					if ((code = get_save(path, dirp)) == -1) {
						fprintf(stderr, "YOU are unable to provide a file name\nContinuing game");
						closedir(dirp);
						continue;
					}
					closedir(dirp);

					strcpy(container, SAVE_DIR);
					strcat(container, path);
					strcat(container, ".sv");
					fprintf(stderr, "Saving to '%s' ..\n", container);
					fs = fopen(container, "w");
					if (fs == NULL) {
						sprintf(error, "'%s'", path);
						perror(error);
						fprintf(stderr, "continuing game\n");
						continue;
					}
					board_to_fenstring(string, &board);
					fprintf(fs, "%s\n", string);
					player_info_string(string, pw, pb);
					fprintf(fs, "%s\n", string);
					fclose(fs);
					continue;
					break;
				case draw_ins:
					continue;
					break;
				case board_ins:
					filled_display(&board, MOVES_MODE, board.player);
					continue;
					break;
				case help_ins:
					fprintf(stderr, "Overview of Commands:\nmove: [a-h][1-8]-[a-h][1-8] (initial and final squares of move)\nsave: save current position of board to file\nboard: Re-Draw board\nquit: exits (without saying)\nhelp: this instruction! (I am a strange loop)\n");
					continue;
					break;
				case invalid_ins:
					fprintf(stderr, "'%s' is not a recognised instruction or move\n", command);
					continue;
					break;
				default:
					continue;
					break;
			}
		}
		else {
#if SPECS
			gettimeofday(&performance1, NULL);
#endif
			otherai = trillian(board, set, min, max, DEPTH);
#if SPECS
			gettimeofday(&performance2, NULL);
			timereq = (performance2.tv_sec + performance2.tv_usec * 1e-6) - (performance1.tv_sec + performance1.tv_usec * 1e-6);
#endif

			mv = otherai.mov;
			if (!can_move(&board, &set, mv)) {
				fprintf(stderr, "Invalid Move, %s\n", pt.name);
				continue;
			}
		}

		castle = make_move(&board, &set, mv);

		/* handle special moves */
		if (white_kingside <= castle && black_queenside >= castle) {
			rook_castle = rook_move(castle);
			menial_move(&board, &set, rook_castle);
			update_pieces(&board, &set, rook_castle);
		}
		else if (castle == promotion) {
			if (pt.type == HUMAN) {
				promoted = get_promotion(board.player == 'b' ? 'w' : 'b');
			}
			else if (pt.type == COMPUTER) {
				/* always promotes to queen, for lack of a better logic */
				promoted = (board.player == 'w' ? 'q' : 'Q');
			}
			handle_promotion(&board, &set, mv, promoted);

		}

		/* recalculate */
		update_pieces(&board, &set, mv);
		calculate_pins(&set, &board, 'w');
		calculate_pins(&set, &board, 'b');
		calculate_threats(&set, board.player);

		/* show board */
		pt = (board.player == 'w' ? pw : pb);
		/* if computer playing always keep perspective from human side */
		color = (pt.type == COMPUTER) ? ((pt.color == 'w') ? 'b' : 'w') : pt.color;
		filled_display(&board, MOVES_MODE, color);

		/* show previous move for clarity */
		pt = (board.player == 'w') ? pb : pw;
		fprintf(stderr, "%s played  ", pt.name); print_move(mv);
#if SPECS
		if (pt.type == COMPUTER) {
			fprintf(stderr, "Time Taken: %lf sec\n", timereq);
		}
		printf("Position Evaluation: %lf\n", position_evaluate(&set, board.fullmoves));
#endif
		/* check for end of game */

		if (is_checkmate(&board, &set)) {
			fprintf(stderr, "Checkmate!\n");
			fprintf(stderr, "%s Wins!\n", board.player == 'w' ? pb.name : pw.name);
			break;
		}
		else if (is_draw(&board, &set)) {
			fprintf(stderr, "Draw!\n");
			break;
		}


	}
	return 0;
}
