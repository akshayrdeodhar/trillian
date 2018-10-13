FLAGS = -Wall -std=c89 -fsanitize=address #-g
REDIRECT = 2> errors.err

try: main.o board.o display.o pieces.o moves.o input.o
	gcc main.o board.o display.o pieces.o moves.o input.o $(FLAGS) -o try $(REDIRECT)

main.o: main.c board.h pieces.h display.h moves.h
	gcc -c main.c $(FLAGS) $(REDIRECT)

board.o: board.c board.h defs.h
	gcc -c board.c $(FLAGS) $(REDIRECT)

display.o: display.c display.h board.h defs.h 
	gcc -c display.c $(FLAGS) $(REDIRECT)

pieces.o: pieces.c pieces.h board.h defs.h 
	gcc -c pieces.c $(FLAGS) $(REDIRECT)

moves.o: moves.c moves.h pieces.h board.h defs.h display.h
	gcc -c moves.c $(FLAGS) $(REDIRECT)

input.o: input.c
	gcc -c input.c $(FLAGS) $(REDIRECT)

clean:
	rm *~ *.o try
