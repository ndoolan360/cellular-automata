#include "cells.h"
#include "grid.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

QtNode *cells_init(size_t size) { return grid_init(size, sizeof(cell_data_t)); }

void cells_free(QtNode *root) { grid_free_node(root); }

cell_data_t DEFAULT = {.state = DEAD};

cell_data_t cells_get_cell_data(QtNode *root, int x, int y) {
  QtNode *node = grid_get_cell(root, x, y);
  if (node == NULL || node->data == NULL) {
    return DEFAULT;
  }

  return *(cell_data_t *)node->data;
}

static void cells_set_cell_state(QtNode *root, int x, int y,
                                 cell_state_t state) {
  if (state == DEAD) {
    grid_remove_cell(root, x, y);
    return;
  }

  QtNode *node = grid_put_cell(root, x, y, NULL);
  if (node == NULL) {
    return;
  }

  cell_data_t *data = malloc(sizeof(cell_data_t));
  assert(data);

  data->state = state;
  node->data = data;
}

void cells_toggle_cell_state(QtNode *root, int x, int y) {
  cell_data_t current = cells_get_cell_data(root, x, y);
  cells_set_cell_state(root, x, y, current.state == ALIVE ? DEAD : ALIVE);
}

static struct {
  int dx;
  int dy;
} OFFSETS[8] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0},
                {1, 0},   {-1, 1}, {0, 1},  {1, 1}};

void cells_step(QtNode **root_ptr, ruleset rule) {
  QtNode *root = *root_ptr;
  QtNode *new = cells_init(root->size);

  QtNode **cells = (QtNode **)calloc(32, sizeof(QtNode *));
  size_t cell_count = 0;
  size_t cells_size = 32;
  cells_get_all_alive_cells(root, &cells, &cells_size, &cell_count);

  for (size_t i = 0; i < cell_count; i++) {
    QtNode *node = cells[i];
    uint8_t neighbours = 0;
    for (int i = 0; i < 8; i++) {
      int neighbour_x = node->x + OFFSETS[i].dx;
      int neighbour_y = node->y + OFFSETS[i].dy;

      if (rule.wrap) {
        neighbour_x = (neighbour_x + root->size) % root->size;
        neighbour_y = (neighbour_y + root->size) % root->size;
      }

      cell_data_t neighbour =
          cells_get_cell_data(root, neighbour_x, neighbour_y);
      neighbours += neighbour.state;

      uint8_t neighbour_neighbours = 0;
      for (int j = 0; j < 8; j++) {
        int neighbour_neighbour_x = neighbour_x + OFFSETS[j].dx;
        int neighbour_neighbour_y = neighbour_y + OFFSETS[j].dy;

        if (rule.wrap) {
          neighbour_neighbour_x =
              (neighbour_neighbour_x + root->size) % root->size;
          neighbour_neighbour_y =
              (neighbour_neighbour_y + root->size) % root->size;
        }

        if (neighbour_neighbour_x == node->x &&
            neighbour_neighbour_y == node->y) {
          neighbour_neighbours += 1;
        } else {
          cell_data_t neighbour_neighbour = cells_get_cell_data(
              root, neighbour_neighbour_x, neighbour_neighbour_y);
          neighbour_neighbours += neighbour_neighbour.state;
        }
      }

      if (rule.birth[neighbour_neighbours]) {
        cells_set_cell_state(new, neighbour_x, neighbour_y, ALIVE);
      }
    }

    if (rule.survive[neighbours]) {
      cells_set_cell_state(new, node->x, node->y, ALIVE);
    }
  }

  free(cells);

  cells_free(*root_ptr);
  *root_ptr = new;
}

void cells_get_all_alive_cells(QtNode *node, QtNode ***cells_ptr,
                               size_t *cells_size_ptr, size_t *cell_count_ptr) {
  if (node == NULL) {
    return;
  }

  QtNode **cells = *cells_ptr;
  size_t cells_size = *cells_size_ptr;
  size_t cell_count = *cell_count_ptr;

  // resize cells as needed
  if (cell_count >= cells_size) {
    size_t new_size = cells_size * 2;
    void *tmp = realloc(cells, new_size * sizeof(QtNode *));
    if (tmp == NULL) {
      perror("realloc failed");
      free(cells);
      exit(EXIT_FAILURE);
    }
    (*cells_ptr) = tmp;
    *cells_size_ptr = new_size;
  }

  // add the cell if it's active
  if (node->data != NULL && ((cell_data_t *)node->data)->state == ALIVE) {
    (*cells_ptr)[cell_count] = node;
    *cell_count_ptr += 1;
  }

  for (int i = 0; i < 4; i++) {
    cells_get_all_alive_cells(node->children[i], cells_ptr, cells_size_ptr,
                              cell_count_ptr);
  }
}

ruleset parse_rule(char *rule_str, bool wrap) {
  ruleset out = {.wrap = wrap, .birth = {0}, .survive = {0}};

  size_t i = 0;
  char last_letter;
  while (rule_str[i] != '\0') {
    switch (rule_str[i]) {
    case 'B':
    case 'S':
      last_letter = rule_str[i];
      break;
    case '/':
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
      if (last_letter == 'B') {
        out.birth[rule_str[i] - '0'] = true;
      } else if (last_letter == 'S') {
        out.survive[rule_str[i] - '0'] = true;
      } else {
        perror("unknown prior letter in rule");
        exit(1);
      }
      break;
    default:
      perror("unknown character in rule");
      exit(1);
    }

    i++;
  }

  return out;
}
