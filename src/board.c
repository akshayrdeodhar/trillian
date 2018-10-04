#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"

#define FEN_TOKENS 6
#define valid_enpass_square(square_string) ((square_string[0] >= 'a' && square_string[0] <= 'z') && (square_string[1] == '3' || square_string[1] == '6'))

/* get next token from the 'fen' string
 * read current token and skip a space
 * more than one space-> format improper
 * Returns number of characters read
 * Returns -1 if more than one space
 * */
int get_fentok(char tok[], char str[], int reset) {
	static unsigned index = 0;
	static unsigned token_no = 0;
	int i = 0;

	if (reset) {
		index = 0;
		token_no = 0;
	}

	while(str[index] != '\0' && str[index] != ' ') {
		tok[i++] = str[index++];
	}
	tok[i] = '\0';
	/* assert: str[index] == '\0' OR str[index] == ' ' */
	/* note: a valid 'fen' string has 6 tokens in all */
	if (str[index] == ' ') {
		index++;
	}

	if (str[index] == '\0' && token_no != 5) {
		return 0;
	}

	token_no++;
	return i;
}

int safe_atoi(char string[]) {
	int n = 0;
	int i = 0;
	while(isdigit(string[i])) {
		n += string[i] - '0';
		++i;
	}

	if (string[i] != '\0') {
		return -1;
	}

	return n;
}

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
	int length;
	int i, j = 0;
	int n;
	int number = 0;

	char *current = (char *)malloc(sizeof(char) * (strlen(fenstring) + 1));

	length = get_fentok(current, fenstring, 1);

	if (!length) {
		return 0;
	}

	for (j = 0; j < length; ++j) {
		switch(current[j]) {
			case '/':
				if (file != 8) {
					fprintf(stderr, "invalid fen string: invalid number of squares in row %d\n", rank + 1);
					return 0;
				}
				--rank;
				file = 0;
				break;


			case 'K': case 'Q': case 'R': 
			case 'N': case 'B': case 'P':
			case 'k': case 'q': case 'r': 
			case 'n': case 'b': case 'p':
				board->brd[rank][file].pc = current[j];
				board->brd[rank][file].index = NONE;
				file++;
				break;

			case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8':
				n = current[j] - '0';
				if (n == 0 || n == 9) {
					fprintf(stderr, "invalid fen string: invalid number of blank squares in row %d\n", rank + 1);
					return 0;
				}
				for (i = 0; i < n; ++i) {
					board->brd[rank][file].pc = (char)0;
					board->brd[rank][file].index = NONE;
					file++;
				}
				break;

			default:
				fprintf(stderr, "invalid fen string: character not recognised\n");
				/* not in notation, exit */
				return 0;
				break;
		}
	}
	if (rank != 0) {
		fprintf(stderr, "invalid fen string: invalid number of rows (%d)\n", rank + 1);
		return 0;
	}

	length = get_fentok(current, fenstring, 0);

	if (!length) {
		return 0;
	}

	if (current[0] == 'w' || current[i] == '0') {
		board->player = current[i];
	}
	else {
		return 0;
	}

	length = get_fentok(current, fenstring, 0);

	if (!length) {
		return 0;
	}

	for (j = 0; j < length; ++j) {
		switch(current[j]) {
			case '-':
				if (board->castling != 0 || current[j] != ' ') {
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
	}
	if (j > 4) {
		return 0;
	}

	length = get_fentok(current, fenstring, 0);

	if ((length == 2 && valid_enpass_square(current)) || (length == 1 && current[0] == '-')) {
		if (length == 2) {
			board->enpass_target.file = current[0] - 'a';
			board->enpass_target.rank = current[1] - '1';
		}
	}
	else {
		return 0;
	}


	length = get_fentok(current, fenstring, 0);

	if (!length) {
		return 0;
	}

	if ((number = safe_atoi(current)) > -1) {
		board->halfmoves = (usint)number;
	}
	else {
		return 0;
	}


	length = get_fentok(current, fenstring, 0);

	if (!length) {
		return 0;
	}

	if ((number = safe_atoi(current)) > -1) {
		board->fullmoves = (usint)number;
	}
	else {
		return 0;
	}

	free(current);
	return 1;
}


square board_position(chessboard board, position p) {
	return board.brd[p.rank][p.file];
}




