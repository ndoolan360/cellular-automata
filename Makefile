CC ?= cc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -g
LDFLAGS = -lm -lncurses
SRC = src

all: cellular-automata

grid.o: $(SRC)/grid.c
	$(CC) $(CFLAGS) -c $(SRC)/grid.c

cells.o: $(SRC)/cells.c
	$(CC) $(CFLAGS) -c $(SRC)/cells.c

curses.o: $(SRC)/curses.c
	$(CC) $(CFLAGS) -c $(SRC)/curses.c

cellular-automata: curses.o cells.o grid.o
	$(CC) -o cellular-automata curses.o cells.o grid.o $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o cellular-automata
