#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "board.h"
#include "pieces.h"
#include "display.h"
#include "moves.h"
#include "input.h"

#define DEFAULT_PATH "../dat/default.fen"
#define DEBUG (DEBUG_THREAT | DEBUG_END)
#define DEBUG_INTERFACE 1
#define DEBUG_CALCULATE 2
#define DEBUG_THREAT 4
#define DEBUG_END 8

#define MAIN_LOOP 1

int main(int argc, char *argv[]) {

	char command[32];
	move mv, rook_castle;

	if (argc > 2) {
		fprintf(stderr, "usage: ./chess <file.fen>\n");
		return EINVAL;
	}

	FILE *fp;

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


	int n;
	char string[128];
	fgets(string, 128, fp);
	n = strlen(string);
	if (string[n - 1] == '\n') {
		string[n - 1] = '\0';
	}
	chessboard board;
	chesset set;
	castle_move castle;

	if (fenstring_to_board(&board, string)) {
		printf("Impossible:\n");
		display(board, READ_MODE);
	}
	else {
		display(board, READ_MODE);
		return 1;
	}

	interface_board_set(&board, &set);

#if (DEBUG & DEBUG_INTERFACE)
	show_set(set); 
	verify_interface(board, set);
#endif

	calculate_all(&set, board);

#if (DEBUG & DEBUG_CALCULATE)
	debug_calculation(set, board);
#endif
	calculate_pins(&set, board, 'w');
	calculate_pins(&set, board, 'b');

	calculate_threats(&set, board.player);

	if (DEBUG & DEBUG_THREAT) {
		show_threats(set, board);
	}
#if 0
	calculate_threats(&set, 'b');

	if (DEBUG & DEBUG_THREAT) {
		show_threats(set, board);
	}
#endif

#if MAIN_LOOP
	while(1) {

		fprintf(stderr, "COMMAND:");
		readline(command, 32);

		if (!(strcmp(command, "quit"))) {
			if (DEBUG & DEBUG_END) {
				enumpins(set);
				moves_bitboard(set, board);
			}
			return 0;
		}

		mv = extract_move(command);
		print_move(mv);
		if (!can_move(board, set, mv)) {
			fprintf(stderr, "Invalid Move\n");
			continue;
		}

		castle = make_move(&board, &set, mv);


		if (castle) {
			rook_castle = rook_move(castle);
			menial_move(&board, &set, rook_castle);
			update_pieces(board, &set, rook_castle);
		}
		
		update_pieces(board, &set, mv);

		if (castle) {
			moves_bitboard(set, board);
		}

		/*verify_interface(board, set);*/
		display(board, READ_MODE);


		calculate_pins(&set, board, 'w');
		calculate_pins(&set, board, 'b');

		calculate_threats(&set, board.player);

		if (DEBUG & DEBUG_THREAT) {
			show_threats(set, board);
		}
#if 0
		calculate_threats(&set, 'b');

		if (DEBUG & DEBUG_THREAT) {
			show_threats(set, board);
		}
#endif
	}
#endif
	return 0;
}
