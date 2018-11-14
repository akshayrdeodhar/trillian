FLAGS = -Wall -O2
MATH = -lm
ASAN = #-fsanitize=address #-fsanitize=leak -fsanitize=undefined
DEBUG = #-g

CFLAGS=$(FLAGS) $(ASAN) $(DEBUG)

CC=gcc  $(CFLAGS)

SRC=src

DAT=dat

SOURCE=$(SRC)/main.c\
       $(SRC)/board.c\
       $(SRC)/pieces.c\
       $(SRC)/moves.c\
       $(SRC)/input.c\
       $(SRC)/display.c\
       $(SRC)/vector.c\
       $(SRC)/array.c\
       $(SRC)/zaphod.c\
       $(SRC)/trillian.c\
       $(SRC)/player.c

OBJECT =  $(SOURCE:.c=.o)

BIN=project $(DAT)/*

DIST= $(SRC)/*.c $(SRC)/*.h  $(DAT)/* Makefile README.md LICENSE 

# Makes executable
project: $(OBJECT)
	$(CC) $(OBJECT) $(MATH) -o project

# Compiles to .o
$(OBJECT) : %.o : %.c Makefile 
	$(CC) -c $< -o $@

# Makes a distribution with source
dist: $(DIST)
	tar -czvf trillian42.tar.gz $(DIST)

# Makes a bin distriution
bin: $(BIN)
	tar -czvf trillian42bin.tar.gz $(BIN)
# Removes .o and temp files
clean:
	rm *~ $(SRC)/*.o project
