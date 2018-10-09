FLAGS = -Wall -std=c89 -fsanitize=address

try: main.o board.o display.o pieces.o moves.o input.o
	gcc main.o board.o display.o pieces.o moves.o input.o $(FLAGS) -o try

main.o: main.c board.h pieces.h display.h moves.h
	gcc -c main.c $(FLAGS)

board.o: board.c board.h defs.h
	gcc -c board.c $(FLAGS)

display.o: display.c display.h board.h defs.h 
	gcc -c display.c $(FLAGS)

pieces.o: pieces.c pieces.h board.h defs.h 
	gcc -c pieces.c $(FLAGS) 

moves.o: moves.c moves.h pieces.h board.h defs.h display.h
	gcc -c moves.c $(FLAGS)

input.o: input.c
	gcc -c input.c $(FLAGS)

clean:
	rm *~ *.o try
