/* The main game loop which calls everything */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include "board.h"
#include "pieces.h"
#include "display.h"
#include "moves.h"
#include "input.h"
#include "zaphod.h"
#include "trillian.h"
#include "limits.h"

#define DEFAULT_PATH "../dat/default.fen"
#define SAVE_DIR "../save/"
#define DEBUG (0)
#define DEBUG_INTERFACE 1
#define DEBUG_CALCULATE 2
#define DEBUG_THREAT 4
#define DEBUG_END 8
#define DEBUG_FEN 16
#define MAIN_LOOP 1
#define ENUM_MOVES 0
#define DEPTH 4

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

	struct timeval performance1, performance2; /* for trillian's performance measurement */
	double timereq;

	int players; /* no. of players */
	player_token pw, pb, pt; /* white, black, and temp */
	char color; /* to specify side from which board is to be printed */

	branch min; /* for alpha-beta pruning */
	min.score = INT_MIN;
	branch max;
	max.score = INT_MAX;

	move mv, rook_castle; /* the move from the user, for special case of castling */
	special_move castle; /* check and do corresponding rook movement for castling */
	char promoted = '\0'; /* to recieve promotion */

	branch otherai;

#if ENUM_MOVES
	array a;
	int n, i, movecount;
#endif

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
	else if (argc == 2) {
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
	fp = fopen("../dat/titlescreen.txt", "r");
	char c;
	while((c = getc(fp)) != EOF) putchar(c);
	fclose(fp);


	if ((!code) || (code_dash == -1)) {
		/* file does not contain player info */
		players = get_gamemode();
		if (!players) {
			printf("Begone, Cretin\n");
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
		fprintf(stdin, "Invalid game file, game has already ended\n");
	}

	if (DEBUG & DEBUG_THREAT) {
		show_threats(&set, &board);
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
			fprintf(stderr, "Your turn, %s\nCommand:", pt.name);
			readline(command, 32, stdin);
			ins = get_command(command);
			switch(ins.c) {
				case move_ins:
					mv = ins.mv;
					if (!can_move(&board, &set, mv)) {
						printf("Invalid Move: %s\n", pt.name);
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
					printf("Overview of Commands:\nmove: [a-h][1-8]-[a-h][1-8] (initial and final squares of move)\nsave: save current position of board to file\nboard: Re-Draw board\nquit: exits (without saying)\nhelp: this instruction! (I am a strange loop)\n");
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
#if ENUM_MOVES
			ainit(&a);
			printf("Possible Moves:\n");
			generate_moves(&board, &set, &a);
			movecount = alength(&a);
			for (i = 0; i < movecount; i++) {
				print_move(a.arr[i]);
			}
			adestroy(&a);
#endif
			gettimeofday(&performance1, NULL);
			otherai = distributed_trillian(board, set, min, max, DEPTH);
			gettimeofday(&performance2, NULL);
			timereq = (performance2.tv_sec + performance2.tv_usec * 1e-6) - (performance1.tv_sec + performance1.tv_usec * 1e-6);

			mv = otherai.mov;
			if (!can_move(&board, &set, mv)) {
				printf("Invalid Move, %s\n", pt.name);
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

		if (DEBUG & DEBUG_THREAT) {
			show_threats(&set, &board);
		}

#if (DEBUG & DEBUG_FEN) 
		show_register(board.castling);
		display(&board, READ_MODE);
		board_to_fenstring(string, &board);
		printf("%s\n", string);
#endif

		/* show board */
		pt = (board.player == 'w' ? pw : pb);
		/* if computer playing always keep perspective from human side */
		color = (pt.type == COMPUTER) ? ((pt.color == 'w') ? 'b' : 'w') : pt.color;
		filled_display(&board, MOVES_MODE, color);

		/* show previous move for clarity */
		pt = (board.player == 'w') ? pb : pw;
		fprintf(stderr, "%s played  ", pt.name); print_move(mv);
		if (pt.type == COMPUTER) {
			printf("Time Taken: %lf sec\n", timereq);
		}
		printf("Position Evaluation: %lf\n", position_evaluate(&set, board.fullmoves));
	}
	return 0;
}
