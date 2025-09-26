#include "gol.h"
#include "grid.h"
#include <assert.h>
#include <curses.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
  initscr();
  nonl();
  noecho();
  cbreak();
  timeout(0);
  curs_set(0);

  QtNode *root = gol_init(32);

  // oscillator
  gol_set_cell_state(root, 2, 2, ALIVE);
  gol_set_cell_state(root, 3, 2, ALIVE);
  gol_set_cell_state(root, 4, 2, ALIVE);

  // block
  gol_set_cell_state(root, 2, 6, ALIVE);
  gol_set_cell_state(root, 2, 7, ALIVE);
  gol_set_cell_state(root, 3, 6, ALIVE);
  gol_set_cell_state(root, 3, 7, ALIVE);

  // glider
  gol_set_cell_state(root, 7, 4, ALIVE);
  gol_set_cell_state(root, 8, 4, ALIVE);
  gol_set_cell_state(root, 9, 4, ALIVE);
  gol_set_cell_state(root, 9, 3, ALIVE);
  gol_set_cell_state(root, 8, 2, ALIVE);

  for (;;) {
    int c = getch();
    switch (c) {
    case 'q':
    case 27: // [ESC]
      goto quit;
    case 'n':
      gol_step(&root);
      break;
    }

    // draw frame
    clear();

    for (int y = 0; y < (int)root->size; ++y) {
      for (int x = 0; x < (int)root->size; ++x) {
        cell_data_t data = gol_get_cell_data(root, x, y);
        if (data.state == ALIVE) {
          mvaddch(y, x, ACS_CKBOARD);
        }
      }
    }

    mvprintw(0, 1, "q: quit | n: next");
    refresh();

  }

  quit:
  gol_free(root);
  return 0;
}
