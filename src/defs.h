#ifndef DEFS_H
#define DEFS_H
#include <ctype.h>

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


#define mkpositive(u) ((u) > 0 ? (u) : (-(u))) /* u is any number */
#define MAXMAG(a, b) (mkpositive(a) > mkpositive(b) ? mkpositive(a) : mkpositive(b))
#define isWhite(p) (isupper(p)) /* checks whether SAN char corresponds to a white piece */
#define isBlack(p) (islower(p)) /* checks whether SAN char corresponds to a black piece */
#define print_move(mv) (printf("%c%c-%c%c\n", mv.ini.file + 'a', mv.ini.rank + '1', mv.fin.file + 'a', mv.fin.rank + '1'))
#define inrange(x, y) ((x >= 0) && (x < 8) && (y >= 0) && (y < 8))
#define isSame(p, q) ((isWhite(p) && isWhite(q)) || (isBlack(p) && isBlack(q))) /* p, q SAN chars */
#define distance(sl, dir) (((dir) < 8) ? MAXMAG(sl.drank, sl.dfile) : 1)
#define posequal(pos1, pos2) (pos1.rank == pos2.rank && pos1.file == pos2.file)

#endif
