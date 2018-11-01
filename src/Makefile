FLAGS = -Wall -std=c89 -O2
MATH = -lm
REDIRECT = #2> errors.err
ASAN = #-fsanitize=address
DEBUG = #-g

try: main.o board.o display.o pieces.o moves.o input.o vector.o
	gcc main.o board.o display.o pieces.o moves.o input.o vector.o $(FLAGS) $(MATH) $(ASAN) $(DEBUG) -o try $(REDIRECT)

main.o: main.c board.h pieces.h display.h moves.h vector.h
	gcc -c main.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

board.o: board.c board.h defs.h vector.h
	gcc -c board.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

display.o: display.c display.h board.h defs.h
	gcc -c display.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

pieces.o: pieces.c pieces.h board.h defs.h
	gcc -c pieces.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

moves.o: moves.c moves.h pieces.h board.h defs.h display.h vector.h
	gcc -c moves.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

input.o: input.c
	gcc -c input.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

vector.o: vector.c vector.h defs.h
	gcc -c vector.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

clean:
	rm *~ *.o try
