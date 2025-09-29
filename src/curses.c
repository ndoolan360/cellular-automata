#include "cells.h"
#include "grid.h"
#include <curses.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static void finish(int sig) {
  printf("\033[?1003l\n"); // Disable mouse movement events
  endwin();
  exit(sig);
}

static int parse_int_arg(char flag, int min, int max) {
  char *endp = NULL;
  long l = -1;
  if (!optarg || ((l = strtol(optarg, &endp, 10)), (endp && *endp))) {
    fprintf(stderr, "invalid %c option %s - expecting a number\n", flag,
            optarg ? optarg : "<NULL>");
    exit(EXIT_FAILURE);
  };
  if (l < min) {
    fprintf(stderr, "invalid %c option: '%s' - must be greater than '%d'\n",
            flag, optarg ? optarg : "<NULL>", min);
    exit(EXIT_FAILURE);
  }
  if (l > max) {
    fprintf(stderr, "invalid %c option: '%s' - must be less than '%d'\n", flag,
            optarg ? optarg : "<NULL>", max);
    exit(EXIT_FAILURE);
  }
  return (int)l;
}

static bool parse_args(int argc, char **argv, bool *debug, int *fps,
                       size_t *grid_size, ruleset *rule) {
  char *rule_str = "B3/S23";
  bool wrap = false;
  bool infinite = false;

  int opt;
  while ((opt = getopt(argc, argv, "df:g:ir:w")) != -1) {
    switch (opt) {
    case 'd':
      *debug = true;
      break;
    case 'f':
      *fps = parse_int_arg('f', INT_MIN, INT_MAX);
      break;
    case 'g':
      *grid_size = parse_int_arg('g', 1, INT_MAX);
      break;
    case 'r':
      rule_str = optarg;
    case 'i':
      infinite = true;
      break;
    case 'w':
      wrap = true;
      break;
    default:
      fprintf(stderr, "Usage: %s [d f:fps g:grid-size i r:rule w]\n", argv[0]);
      return false;
    }
  }

  *rule = parse_rule(rule_str, wrap, infinite);

  return true;
}

int main(int argc, char **argv) {
  signal(SIGINT, finish);

  bool show_debug = false;
  int target_fps = 60;
  size_t grid_size = 64;
  ruleset ruleset;
  if (!parse_args(argc, argv, &show_debug, &target_fps, &grid_size, &ruleset)) {
    finish(1);
  }

  initscr();
  keypad(stdscr, TRUE);
  nonl();
  noecho();
  cbreak();
  timeout(0);
  curs_set(0);

  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  printf("\033[?1003h\n"); // Report mouse movement events

  QtNode *root = cells_init(grid_size);
  QtNode **cells = (QtNode **)calloc(grid_size, sizeof(QtNode *));

  int camera_x = 0, camera_y = 0;
  MEVENT event = {0};

  bool running = false;
  struct timespec last_frame = {0};
  for (;;) {
    struct timespec start = {0};
    clock_gettime(CLOCK_MONOTONIC, &start);

    size_t pan_dist = 5;
    int c = getch();
    switch (c) {
    case 'q':
    case 27: // [ESC]
      goto quit;
    case 'd':
      show_debug = !show_debug;
      break;
    case ' ':
      running = !running;
      break;
    case '<':
      target_fps--;
      break;
    case '>':
      target_fps++;
      break;
    case 'n':
      cells_step(&root, ruleset);
      break;
    case KEY_UP:
      camera_y -= pan_dist;
      break;
    case KEY_DOWN:
      camera_y += pan_dist;
      break;
    case KEY_LEFT:
      camera_x -= pan_dist;
      break;
    case KEY_RIGHT:
      camera_x += pan_dist;
      break;
    case KEY_MOUSE:
      if (getmouse(&event) == OK) {
        if (event.bstate & (BUTTON1_PRESSED | BUTTON1_CLICKED)) {
          cells_toggle_cell_state(root, event.x + camera_x, event.y + camera_y);
        }
      }
      break;
    }

    if (c == KEY_RESIZE || c == KEY_UP || c == KEY_DOWN ||
                             c == KEY_LEFT || c == KEY_RIGHT) {
    // auto resize to screen
      int max_dim = (LINES + camera_y > COLS + camera_x) ? LINES + camera_y
                                                         : COLS + camera_x;
      grid_resize(&root, max_dim);
    }

    float elapsed = start.tv_sec - last_frame.tv_sec +
                    ((start.tv_nsec - last_frame.tv_nsec) / 1e9);

    // draw frame
    if (target_fps == 0 || elapsed >= (1.0 / target_fps)) {
      if (running) {
        cells_step(&root, ruleset);
      }

      clear();

      int min = -1;
      int max = root->size;
      chtype border_char = '.';
      for (int y = min; y <= max; y++) {
        mvaddch(y - camera_y, min - camera_x, border_char);
        mvaddch(y - camera_y, max - camera_x, border_char);
      }
      for (int x = min; x <= max; x++) {
        mvaddch(min - camera_y, x - camera_x, border_char);
        mvaddch(max - camera_y, x - camera_x, border_char);
      }

      size_t cell_count = 0;
      cells_get_all_alive_cells(root, &cells, &grid_size, &cell_count);

      for (size_t i = 0; i < cell_count; i++) {
        QtNode *node = cells[i];
        mvaddch(node->y - camera_y, node->x - camera_x, ACS_CKBOARD);
      }

      mvprintw(0, 1,
               "q: quit | n: next | arrow-keys: pan | space: %s",
               running ? "pause" : "run");
      if (show_debug) {
        cell_data_t cell =
            cells_get_cell_data(root, event.x + camera_x, event.y + camera_y);
        mvprintw(
            2, 1,
            "grid: %zux%zu | screen: %dx%d | mouse: %d,%d (0x%lx) | cell: %d",
            root->size, root->size, LINES + camera_y, COLS + camera_x,
            event.x + camera_x, event.y + camera_y, event.bstate, cell.state);
        mvprintw(
            3, 1,
            "frametime: %.6f | target-fps: %d | '<': dec fps | '>': inc fps",
            elapsed, target_fps);
      }
      refresh();

      clock_gettime(CLOCK_MONOTONIC, &last_frame);
    }
  }

quit:
  free(cells);
  cells_free(root);
  finish(0);
}
