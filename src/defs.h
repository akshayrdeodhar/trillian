#ifndef DEFS_H
#define DEFS_H

typedef unsigned char usint; /* for rank and file, slide words */
typedef char ssint; /* for changes in direction */

#define isWhite(p) (isupper(p)) /* checks whether SAN char corresponds to a white piece */
#define isBlack(p) (islower(p)) /* checks whether SAN char corresponds to a black piece */

#endif
