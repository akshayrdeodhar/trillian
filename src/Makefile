FLAGS = -Wall -O2 #-Wextra #-std=c89
MATH = -lm
REDIRECT = #2> errors.err
ASAN = #-fsanitize=address #-fsanitize=leak -fsanitize=undefined
DEBUG = #-g

try: main.o board.o display.o pieces.o moves.o input.o vector.o zaphod.o array.o player.o trillian.o
	gcc main.o board.o display.o pieces.o moves.o input.o vector.o zaphod.o array.o player.o trillian.o $(FLAGS) $(MATH) $(ASAN) $(DEBUG) -o try $(REDIRECT)

main.o: main.c board.h pieces.h display.h moves.h vector.h zaphod.h trillian.h player.h array.h config.h
	gcc -c main.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

board.o: board.c board.h defs.h vector.h
	gcc -c board.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

display.o: display.c display.h board.h defs.h chessescape.h config.h
	gcc -c display.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

pieces.o: pieces.c pieces.h board.h defs.h
	gcc -c pieces.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

moves.o: moves.c moves.h pieces.h board.h defs.h display.h vector.h
	gcc -c moves.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

input.o: input.c input.h player.h
	gcc -c input.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

vector.o: vector.c vector.h defs.h
	gcc -c vector.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

array.o: array.c array.h vector.h
	gcc -c array.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

player.o: player.c player.h
	gcc -c player.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

zaphod.o: zaphod.c zaphod.h array.h moves.h pieces.h vector.h
	gcc -c zaphod.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

trillian.o: trillian.c zaphod.h trillian.h moves.h pieces.h vector.h array.h
	gcc -c trillian.c $(FLAGS) $(ASAN) $(DEBUG) $(REDIRECT)

clean:
	rm *~ *.o try

purify:
	rm *~
