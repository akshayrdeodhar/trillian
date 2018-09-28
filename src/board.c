#include "board.h"
#include <stdio.h>
#include <ctype.h>

/* Specification: http://www.thechessdrum.net/PGN_Reference.txt
 *
 * Field 1:
 * while I do not encounter ' '
 * if /, and 8 squares covered, go to next rank
 * else, improper string, return error
 * if digit, fill those number of squares with 0
 * if alphabet, and among valid ones, fill corresponding piece in board
 *
 * if 8 ranks not covered, error
 *
 * skip space
 *
 * Field 2:
 * w white, b black
 *
 * Field 3:
 * read at max 4 characters. if order doomed or anything, error
 *
 * Field 4:
 * [abcdefgh][36]
 *
 * Field 5:
 * integer
 *
 * Field 6:
 * integer
 * */
int fenstring_to_board(chessboard *board, char fenstring[]) {
	usint rank = 7;
	usint file = 0;
	int i = 0;
	int j;
	int n;
	int number = 0;

	printf("%s\n", fenstring);

	while(fenstring[i] != '\0' && fenstring[i] != ' ') {
		switch(fenstring[i]) {
			case '/':
				printf("Slash File: %d Rank: %d\n", file, rank);
				if (file != 8) {
					printf("Leroy Brown\n");
					return 0;
				}
				--rank;
				file = 0;
				break;


			case 'K': case 'Q': case 'R': 
			case 'N': case 'B': case 'P':
			case 'k': case 'q': case 'r': 
			case 'n': case 'b': case 'p':
				printf("Alphabet: %c\n", fenstring[i]);
				board->brd[rank][file++] = fenstring[i];
				break;

			case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8':
				printf("Digit %c\n", fenstring[i]);
				n = fenstring[i] - '0';
				if (n == 0 || n == 9) {
					printf("Leroy Brown\n");
					return 0;
				}
				for (j = 0; j < n; ++j) {
					board->brd[rank][file++] = (char)0;
				}
				break;

			default:
				printf("Leroy Brown\n");
				/* not in notation, exit */
				return 0;
				break;
		}
		i++;
	}
	if (rank != 0 || fenstring[i] == '\0') {
		printf("Leroy Brown\n");
		return 0;
	}
	i++;

	printf("I survived\n");

	if (fenstring[i] == 'w' || fenstring[i] == 'b') {
		board->player = fenstring[i];
	}
	else {
		return 0;
	}

	i++;
	j = 0;
	while(fenstring[i] != '\0' && fenstring[i] != ' ') {
		switch(fenstring[i]) {
			case '-':
				i++;
				if (board->castling != 0 || fenstring[i] != ' ') {
					return 0;
				}
				board->castling = 0;
				break;

			case 'K':
				board->castling |= (1 << 1);
				break;
			case 'k':
				board->castling |= (1 << 3);
				break;
			case 'Q':
				board->castling |= (1 << 0);
				break;
			case 'q':
				board->castling |= (1 << 2);
				break;
			default:
				return 0;
				break;
		}
		i++;
		++j;
	}
	if (j > 4) {
		return 0;
	}
	i++;

	if (fenstring[i] != '-' || !(fenstring[i] <= 'h' && fenstring[i] >= 'a')) {
		return 0;
	}
	board->enpass_target.y = fenstring[i] - 'a';
	
	i++;
	if (!(fenstring[i] >= '1' && fenstring[i] <= '8')) {
		return 0;
	}
	board->enpass_target.x = fenstring[i] - '0';
	
	i++;
	if (fenstring[i] != ' ') {
		return 0;
	}
	
	i++;

	j = 0;
	while(fenstring[i] != '\0' && fenstring[i] != ' ') {
		if (isdigit(fenstring[i])) {
			number = number * 10 + fenstring[i] - '0';
		}
		else {
			return 0;
		}
		i++;
		++j;
	}
	if (fenstring[i] == '\0' || j == 0) {
		return 0;
	}
	board->halfmoves = (usint)number;
	i++;

	j = 0;
	while(fenstring[i] != '\0' && fenstring[i] != ' ') {
		if (isdigit(fenstring[i])) {
			number = number * 10 + fenstring[i] - '0';
		}
		else {
			return 0;
		}
		i++;
		++j;
	}
	if (j == 0) {
		return 0;
	}
	board->fullmoves = (usint)number;

	return 1;
}
