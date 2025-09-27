CC ?= cc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -g
LDFLAGS = -lm -lncurses

all: curses

grid.o: grid.c
	$(CC) $(CFLAGS) -c grid.c

cells.o: cells.c
	$(CC) $(CFLAGS) -c cells.c

curses.o: curses.c
	$(CC) $(CFLAGS) -c curses.c

curses: curses.o cells.o grid.o
	$(CC) -o curses curses.o cells.o grid.o $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o curses
