#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "board.h"
#include "pieces.h"
#include "display.h"
#include "moves.h"
#include "input.h"
#include "zaphod.h"

#define DEFAULT_PATH "../dat/default.fen"
#define DEBUG (0)
#define DEBUG_INTERFACE 1
#define DEBUG_CALCULATE 2
#define DEBUG_THREAT 4
#define DEBUG_END 8
#define DEBUG_FEN 16

#define MAIN_LOOP 1
#define BIG_DISPLAY 1

int main(int argc, char *argv[]) {

	char command[32];
	move mv, rook_castle;
	int players;
	player_token pw, pb, pt;
	token ins;
	int n, movecount, i;
	n = movecount = 0;

	if (argc > 2) {
		fprintf(stderr, "usage: ./chess <file.fen>\n");
		return EINVAL;
	}

	FILE *fp = NULL;

	if (argc == 1) {
		fp = fopen(DEFAULT_PATH, "r");
		if (fp == NULL) {
			perror("invalid default file");
			return errno;
		}
	}
	else if (argc == 2) {
		fp = fopen(argv[1], "r");
		if(fp == NULL) {
			perror("invalid filename");
			return errno;
		}
	}

	array a;
	ainit(&a);
	char string[128];
	readline(string, 128, fp);
	fclose(fp);
	n = strlen(string);
	if (string[n - 1] == '\n') {
		string[n - 1] = '\0';
	}
	chessboard board;
	chesset set;
	special_move castle;

	if (fenstring_to_board(&board, string)) {
		printf("Impossible:\n");
#if BIG_DISPLAY
		filled_display(&board);
#else
		display(&board, READ_MODE);
#endif
	}
	else {
#if BIG_DISPLAY
		filled_display(&board);
#else
		display(&board, READ_MODE);
#endif
		return 1;
	}
	string[0] = '\0';

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
		strcpy(pw.name, "Walter");
		pw.type = (players == 6 ? HUMAN : COMPUTER);
		pw.color = 'w';
		strcpy(pb.name, "Jesse");
		pb.type = pw.type == HUMAN ? COMPUTER : HUMAN;
		pb.color = 'b';
	}



	interface_board_set(&board, &set);

#if (DEBUG & DEBUG_INTERFACE)
	show_set(set); 
	verify_interface(&board, &set);
#endif

	calculate_all(&set, &board);

#if (DEBUG & DEBUG_CALCULATE)
	debug_calculation(&set, &board);
#endif
	calculate_pins(&set, &board, 'w');
	calculate_pins(&set, &board, 'b');

	calculate_threats(&set, board.player);

	if (is_checkmate(&board, &set) || is_stalemate(&board, &set)) {
		fprintf(stderr, "Invalid game file, game has already ended\n");
	}

	if (DEBUG & DEBUG_THREAT) {
		show_threats(&set, &board);
	}
#if 0
	calculate_threats(&set, 'b');

	if (DEBUG & DEBUG_THREAT) {
		show_threats(set, board);
	}
#endif

#if MAIN_LOOP
	while(1) {
		/* zaphod generates moves */
#if 1
		ainit(&a);
		printf("Possible Moves:\n");
		generate_moves(&board, &set, &a);
		movecount = alength(&a);
		for (i = 0; i < movecount; i++) {
			print_move(a.arr[i]);
		}
		adestroy(&a);
#endif


		/* get command from user */
		pt = (board.player == 'w') ? pw : pb;
		if (pt.type == HUMAN) {
			fprintf(stderr, "Your turn, %s\nCommand:", pt.name);
			readline(command, 32, stdin);
			ins = get_command(command);
			switch(ins.c) {
				case move_ins:
					mv = ins.mv;
					break;
				case quit_ins:
					return 0;
					break;
				case save_ins:
					continue;
					break;
				case draw_ins:
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
			print_move(mv);
		}
		else {
			mv = zaphod(&board, &set);
		}
		if (!can_move(&board, &set, mv)) {
			fprintf(stderr, "Invalid Move, %s\n", pt.name);
			continue;
		}
		castle = make_move(&board, &set, mv);

		/* handle special moves */
		if (white_kingside <= castle && black_queenside >= castle) {
			rook_castle = rook_move(castle);
			menial_move(&board, &set, rook_castle);
			update_pieces(&board, &set, rook_castle);
		}
		else if (castle == promotion) {
			char promoted = get_promotion(board.player == 'b' ? 'w' : 'b');
			handle_promotion(&board, &set, mv, promoted);

		}

		/* recalculate */
		update_pieces(&board, &set, mv);
		if (DEBUG & DEBUG_END) {
		show_repetition(&board);
		}
		calculate_pins(&set, &board, 'w');
		calculate_pins(&set, &board, 'b');
		calculate_threats(&set, board.player);

		/* check for end of game */
		if (is_checkmate(&board, &set)) {
			printf("Checkmate!\n");
			printf("%s Wins!\n", board.player == 'w' ? pb.name : pw.name);
			break;
		}
		else if (is_draw(&board, &set)) {
			printf("Draw!\n");
			break;
		}

		if (DEBUG & DEBUG_THREAT) {
			show_threats(&set, &board);
		}

		board_to_fenstring(string, &board);
		if (DEBUG & DEBUG_FEN) {
			printf("Current .FEN string: %s\n", string);
		}

		/* show board */
#if BIG_DISPLAY == 1
		filled_display(&board);
#else
		display(&board, MOVES_MODE);
#endif
		printf("%s\n", string);

		/* show previous move for clarity */
		pt = (board.player == 'w') ? pb : pw;
		printf("%s played  ", pt.name); print_move(mv);
	}
#endif
	return 0;
}
