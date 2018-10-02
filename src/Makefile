try: main.o board.o display.o
	gcc main.o board.o display.o -Wall -std=c89 -o try

main.o: main.c board.h defs.h
	gcc -c main.c -Wall -std=c89

board.o: board.c board.h defs.h
	gcc -c board.c -Wall -std=c89

display.o: display.c display.h board.h defs.h
	gcc -c display.c -Wall -std=c89

pieces.o: pieces.c pieces.h board.h defs.h
	gcc -c pieces.c -Wall -std=c89 

clean:
	rm *~ *.o try
