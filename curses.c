#include "gol.h"
#include "grid.h"
#include <assert.h>
#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

static void finish(int sig) {
  printf("\033[?1003l\n"); // Disable mouse movement events
  endwin();
  exit(sig);
}

int main(void) {
  signal(SIGINT, finish);

  initscr();
  keypad(stdscr, TRUE);
  nonl();
  noecho();
  cbreak();
  timeout(0);
  curs_set(0);

  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  printf("\033[?1003h\n"); // Report mouse movement events

  QtNode *root = gol_init(32);
  bool show_debug = false;
  MEVENT event = { 0 };

  for (;;) {
    int c = getch();
    switch (c) {
    case 'q':
    case 27: // [ESC]
      goto quit;
    case 'd':
      show_debug = !show_debug;
      break;
    case 'n':
      gol_step(&root);
      break;
    case KEY_MOUSE:
      if (getmouse(&event) == OK) {
        if (event.bstate & (BUTTON1_PRESSED | BUTTON1_CLICKED)) {
          gol_toggle_cell_state(root, event.x, event.y);
        }
      }
    }

    // draw frame
    clear();

    int min = -1;
    int max = root->size;
    chtype border_char = '.';
    for (int y = 0; y <= max; ++y) {
      mvaddch(y, min, border_char);
      mvaddch(y, max, border_char);
    }
    for (int x = 0; x <= max; ++x) {
      mvaddch(min, x, border_char);
      mvaddch(max, x, border_char);
    }

    for (int y = 0; y < (int)root->size; ++y) {
      for (int x = 0; x < (int)root->size; ++x) {
        cell_data_t data = gol_get_cell_data(root, x, y);
        if (data.state == ALIVE) {
          mvaddch(y, x, ACS_CKBOARD);
        }
      }
    }

    mvprintw(0, 1, "q: quit | n: next");
    if (show_debug) {
      int x = event.x;
      int y = event.y;
      cell_data_t cell = gol_get_cell_data(root, x, y);
      mvprintw(
          1, 1,
          "grid: %zux%zu | mouse: %d,%d (0x%lx) | cell: %d",
          root->size, root->size, x, y, event.bstate, cell.state);
    }
    refresh();

  }

  quit:
  gol_free(root);
  finish(0);
}
