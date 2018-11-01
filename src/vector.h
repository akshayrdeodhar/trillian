#ifndef VECTOR_H
#define VECTOR_H

#include "defs.h"

#define DIR_NONE 16

typedef struct {
	usint rank, file;
}position;

typedef struct {
	ssint dfile, drank;
}movement;

typedef struct {
	position ini;
	position fin;
}move;

movement find_movement(position ini, position fin); /* finds change in position */

usint find_dir(movement sl); /* finds sliding direction from 'slope' */

ssint fileincr(usint direction); /* returns change in 'file' for unit movement in 'direction' */

ssint rankincr(usint direction); /* return change 'rank' for unit movement in direction */

#endif
