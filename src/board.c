/* board state, FEN functions */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"
#include "pieces.h"

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
	int kwf, kbf;

	/* king flag. board must have both kings */
	kwf = kbf = 0;

	char *current = (char *)malloc(sizeof(char) * (strlen(fenstring) + 1));

	length = get_fentok(current, fenstring, 1);

	if (!length) {
		goto handle;
	}

	for (j = 0; j < length; ++j) {
		switch(current[j]) {
			case '/':
				if (file != 8) {
					fprintf(stderr, "invalid fen string: invalid number of squares in row %d\n", rank + 1);
					goto handle;
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
				if (current[j] == 'K') {
					kwf = 1;
				}
				else if (current[j] == 'k') {
					kbf = 1;
				}
				break;

			case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8':
				n = current[j] - '0';
				if (n == 0 || n == 9) {
					fprintf(stderr, "invalid fen string: invalid number of blank squares in row %d\n", rank + 1);
					goto handle;
				}
				for (i = 0; i < n; ++i) {
					board->brd[rank][file].pc = (char)0;
					board->brd[rank][file].index = NONE;
					file++;
				}
				break;

			default:
				fprintf(stderr, "invalid fen string: character '%c' not recognised\n", current[j]);
				/* not in notation, exit */
				goto handle;
				break;
		}
	}
	if (!(kwf && kbf)) {
		fprintf(stderr, "invalid fen string: board must have both white and black king\n");
		goto handle;
	}
	if (rank != 0) {
		fprintf(stderr, "invalid fen string: invalid number of rows (%d)\n", rank + 1);
		goto handle;
	}

	length = get_fentok(current, fenstring, 0);

	if (!length) {
		fprintf(stderr, "Too less tokens\n");
		goto handle;
	}

	if ((current[0] == 'w' || current[0] == 'b') && length == 1) {
		board->player = current[0];
	}
	else {
		goto handle;
		fprintf(stderr, "Too less tokens\n");
	}

	length = get_fentok(current, fenstring, 0);

	if (!length) {
		goto handle;
	}

	for (j = 0; j < length; ++j) {
		switch(current[j]) {
			case '-':
				if (j == 0 && length == 1) {
					board->castling = 0;
				}
				else {
					fprintf(stderr, "invalid castling token\n");
					goto handle;
				}
				break;

			case 'K':
				board->castling |= (1 << white_kingside);
				break;
			case 'k':
				board->castling |= (1 << black_kingside);
				break;
			case 'Q':
				board->castling |= (1 << white_queenside);
				break;
			case 'q':
				board->castling |= (1 << black_queenside);
				break;
			default:
				goto handle;
				break;
		}
	}
	if (j > 4) {
		goto handle;
	}

	length = get_fentok(current, fenstring, 0);

	if ((length == 2 && valid_enpass_square(current)) || (length == 1 && current[0] == '-')) {
		if (length == 2) {
			board->enpass_target.file = current[0] - 'a';
			board->enpass_target.rank = current[1] - '1';
		}
		else {
			board->enpass_target.file = board->enpass_target.rank = NONE_CO_ORD;
		}
	}
	else {
		goto handle;
	}


	length = get_fentok(current, fenstring, 0);

	if (!length) {
		goto handle;
	}

	if ((number = safe_atoi(current)) > -1) {
		board->halfmoves = (usint)number;
	}
	else {
		goto handle;
	}


	length = get_fentok(current, fenstring, 0);

	if (!length) {
		goto handle;
	}

	if ((number = safe_atoi(current)) > -1) {
		board->fullmoves = (usint)number;
	}
	else {
		goto handle;
	}

	free(current);
	return 0;

handle:
	free(current);
	return -1;
}


square board_position(chessboard *board, position p) {
	return board->brd[p.rank][p.file];
}

void board_to_fenstring(char fenstring[], chessboard *board) {
	int i, j;
	usint index = 0;
	usint state;
	usint count;
	char temp[16];
	temp[0] = '\0';
	for (i = 7; i >= 0; --i) {
		state = count = 0;
		for (j = 0; j < 8; ++j) {
			if (board->brd[i][j].pc) {
				if (state) {
					fenstring[index++] = count + '0';
					count = state = 0;
				}
				fenstring[index++] = board->brd[i][j].pc;
			}
			else {
				state = 1;
				count++;
			}
		}
		if (state) {
			fenstring[index++] = count + '0';
		}
		if (i) {
			fenstring[index++] = '/';
		}
	}
	fenstring[index++] = ' ';
	fenstring[index++] = board->player;
	fenstring[index++] = ' ';

	if (board->castling & (1 << (white_queenside))) {
		fenstring[index++] = 'Q';
	}
	if (board->castling & (1 << (black_queenside))) {
		fenstring[index++] = 'q';
	}
	if (board->castling & (1 << (white_queenside))) {
		fenstring[index++] = 'K';
	}
	if (board->castling & (1 << black_kingside)) {
		fenstring[index++] = 'k';
	}

	if (fenstring[index - 1] == ' ') {
		fenstring[index++] = '-';			
	}

	fenstring[index++] = ' ';

	if (board->enpass_target.rank < 8 && board->enpass_target.file < 8) {
		fenstring[index++] = board->enpass_target.file + 'a';
		fenstring[index++] = board->enpass_target.rank + '1';
	}
	else {
		fenstring[index++] = '-';
	}

	fenstring[index++] = ' ';

	fenstring[index] = '\0';

	sprintf(temp, "%d", board->halfmoves);
	strcat(fenstring, temp);
	index += strlen(temp);
	fenstring[index++] = ' ';
	fenstring[index] = '\0';

	temp[0] = '\0';
	sprintf(temp, "%d", board->fullmoves);
	strcat(fenstring, temp);
}
