#ifndef DEFS_H
#define DEFS_H

typedef unsigned char usint; /* for rank and file, slide words */
typedef char ssint; /* for changes in direction */

typedef enum special_move {
	none = 0,
	white_kingside = 1,
	white_queenside = 2,
	black_kingside = 3,
	black_queenside = 4,
	promotion = 5,
}special_move;



#define isWhite(p) (isupper(p)) /* checks whether SAN char corresponds to a white piece */
#define isBlack(p) (islower(p)) /* checks whether SAN char corresponds to a black piece */

#define print_move(mv) (printf("%c%c-%c%c\n", mv.ini.file + 'a', mv.ini.rank + '1', mv.fin.file + 'a', mv.fin.rank + '1'))

#define inrange(x, y) ((x > -1) && (x < 8) && (y > -1) && (y < 8))


#endif
