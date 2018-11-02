#include <stdlib.h>
#include "array.h"

#define MAX 128

void ainit(array *a) {
	a->arr = (move *)malloc(MAX * sizeof(move));
	a->size = MAX;
	a->current = 0;
}

void aappend(array *a, move mv) {
	if (a->current == a->size) {
		a->arr = (move *)realloc(a->arr, (a->size + MAX) * sizeof(move));
		a->size += MAX;
	}

	a->arr[a->current] = mv;
	a->current += 1;
}

void adestroy(array *a) {
	if (a->size == 0) {
		return;
	}
	free(a->arr);
	a->current = a->size = 0;
}

int alength(array *a) {
	return a->current;
}
