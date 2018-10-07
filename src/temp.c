/* menial_moves might have gone very wrong */

/* the move given as input to make must be valid
 * the function is not told explicitly the nature of the move
 * it must deduce what kind of move it is based on the squares and pieces involves
 * this requires it to make multiple changes to board when castling is taking place
 * however, as it has a guarentee that the move is valid (note: piece_can_move() has a lot of work to do), it can simply act
 * is there a better way to implement this? perhaps something where the copying work is done by a seperate function, (which it calls)
 * and the metadata is is handled by the function itself.
 * TODO: Better Ideas ?
 * */
void make_move(chessboard *board, chesset *set, move mv) {

	/* this is done only to shut the warning up. there is a guarentee that if castling happens, castle_rook WILL be set to a proper value */
	move castle_rook = mv;

	/* move piece on board, update index on board, update piece position */
	menial_move(board, set, mv);

	square from = board->brd[mv.ini.rank][mv.ini.file];
	square to = board->brd[mv.fin.rank][mv.fin.file];

	/* castling is a king move, but the rook must also be moved 
	 * Should I instead use a seperate function handle_castle() ? TODO
	 * */
	if (toupper(from.pc) == 'K' && (mv.fin.file == ('g' - 'a') || mv.fin.file == ('c' - 'a'))) {
		printf("Castling is Happening\n");
		castle_rook.ini.rank = castle_rook.fin.rank = mv.ini.rank;
		if (mv.fin.file == ('g' - 'a')) {
			castle_rook.ini.file = 'h' - 'a';
			castle_rook.fin.file = 'f' - 'a';
		}
		else if (mv.fin.file == ('c' - 'a')) {
			castle_rook.ini.file = 'a' - 'a';
			castle_rook.fin.file = 'd' - 'a';
		}
		
		/* move the rook */
		menial_move(board, set, castle_rook);
		
		if (isWhite(from.pc)) {
			board->castling &= MASK_WHITE_CASTLE;
		}
		else {
			board->castling &= MASK_BLACK_CASTLE;
		}
	}
	else {
		printf("Not Castling\n");
	}


	/* flip color */
	board->player = (board->player == 'w') ? 'b' : 'w';

	if (isBlack(from.pc)) {
		board->fullmoves += 1;
	}

	if (to.pc) {
		/* restart doomsday clock ? more like boringday clock */
		board->halfmoves = 0;
		/* rearrange chess set */
		kill_piece(board, set, to);
	}

	if (toupper(from.pc) == 'P') {
		/* restart doomsday clock ? more like boringday clock */
		board->halfmoves = 0;
	}
}

void menial_move(chessboard *board, chesset *set, move mv) {
	square from = board->brd[mv.ini.rank][mv.ini.file];
	square blank;
	blank.pc = '\0';
	blank.index = -1;

	board->brd[mv.fin.rank][mv.fin.file] = from;

	board->brd[mv.ini.rank][mv.ini.file] = blank;
	
	/* update piece positions in set */
	if (isWhite(from.pc)) {
		set->whites[(usint)from.index].ps = mv.fin;
	}
	else {
		set->blacks[(usint)from.index].ps = mv.fin;
	}
}

void make_move(chessboard *board, chesset *set, move mv) {

	square from = board->brd[mv.ini.rank][mv.ini.file];
	square to = board->brd[mv.fin.rank][mv.fin.file];
	square blank;
	blank.pc = '\0';
	blank.index = -1;

	board->brd[mv.fin.rank][mv.fin.file] = from;

	board->brd[mv.ini.rank][mv.ini.file] = blank;

	board->player = (board->player == 'w') ? 'b' : 'w';

	if (isWhite(from.pc)) {
		set->whites[(usint)from.index].ps = mv.fin;
	}
	else {
		board->fullmoves += 1;
		set->blacks[(usint)from.index].ps = mv.fin;
	}

	if (to.pc) {
		board->halfmoves = 0;
		kill_piece(board, set, to);
	}

	if (toupper(from.pc) == 'P') {
		board->halfmoves = 0;
	}
}
