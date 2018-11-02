#include "moves.h"
#ifndef ARRAY_H
#define ARRAY_H

typedef struct {
	move *arr;
	int current;
	int size;
}array;

void ainit(array *a);

void aappend(array *a, move mv);

void adestroy(array *a);

int alength(array *a);

#endif
