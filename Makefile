CC ?= cc
CFLAGS = -Wall -Wextra -Werror -std=c99 -pedantic -g
LDFLAGS = -lm -lncurses

all: curses

grid.o: grid.c
	$(CC) $(CFLAGS) -c grid.c

gol.o: gol.c
	$(CC) $(CFLAGS) -c gol.c

curses.o: curses.c
	$(CC) $(CFLAGS) -c curses.c

curses: curses.o gol.o grid.o
	$(CC) -o curses curses.o gol.o grid.o $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o curses
