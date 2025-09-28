CC ?= cc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -g
LDFLAGS = -lm -lncurses
SRC = src

all: cell_curses

grid.o: $(SRC)/grid.c
	$(CC) $(CFLAGS) -c $(SRC)/grid.c

cells.o: $(SRC)/cells.c
	$(CC) $(CFLAGS) -c $(SRC)/cells.c

curses.o: $(SRC)/curses.c
	$(CC) $(CFLAGS) -c $(SRC)/curses.c

cell_curses: curses.o cells.o grid.o
	$(CC) -o cell_curses curses.o cells.o grid.o $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o cell_curses
