#include "gol.h"
#include "grid.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

QtNode *gol_init(size_t size) { return grid_init(size, sizeof(cell_data_t)); }

void gol_free(QtNode *root) { grid_free_node(root); }

cell_data_t DEFAULT = {.state = DEAD};

cell_data_t gol_get_cell_data(QtNode *root, int x, int y) {
  QtNode *node = grid_get_cell(root, x, y);
  if (node == NULL || node->data == NULL) {
    return DEFAULT;
  }

  return *(cell_data_t *)node->data;
}

static void gol_set_cell_state(QtNode *root, int x, int y, cell_state_t state) {
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

void gol_toggle_cell_state(QtNode *root, int x, int y) {
  cell_data_t current = gol_get_cell_data(root, x, y);
  gol_set_cell_state(root, x, y, current.state == ALIVE ? DEAD : ALIVE);
}

static struct {
  int dx;
  int dy;
} OFFSETS[8] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0},
                {1, 0},   {-1, 1}, {0, 1},  {1, 1}};

void gol_step(QtNode **root_ptr, ruleset rule) {
  QtNode *root = *root_ptr;
  QtNode *new = gol_init(root->size);

  QtNode **cells = (QtNode **)calloc(32, sizeof(QtNode *));
  size_t cell_count = 0;
  size_t cells_size = 32;
  gol_get_all_alive_cells(root, &cells, &cells_size, &cell_count);

  for (size_t i = 0; i < cell_count; i++) {
    QtNode *node = cells[i];
    uint8_t neighbours = 0;
    for (int i = 0; i < 8; i++) {
      int neighbour_x = node->x + OFFSETS[i].dx;
      int neighbour_y = node->y + OFFSETS[i].dy;
      cell_data_t neighbour = gol_get_cell_data(root, neighbour_x, neighbour_y);
      neighbours += neighbour.state;

      uint8_t neighbour_neighbours = 0;
      for (int j = 0; j < 8; j++) {
        int neighbour_neighbour_x = neighbour_x + OFFSETS[j].dx;
        int neighbour_neighbour_y = neighbour_y + OFFSETS[j].dy;
        if (neighbour_neighbour_x == node->x &&
            neighbour_neighbour_y == node->y) {
          neighbour_neighbours += 1;
        } else {
          cell_data_t neighbour_neighbour = gol_get_cell_data(
              root, neighbour_neighbour_x, neighbour_neighbour_y);
          neighbour_neighbours += neighbour_neighbour.state;
        }
      }

      if (rule.birth[neighbour_neighbours]) {
        gol_set_cell_state(new, neighbour_x, neighbour_y, ALIVE);
      }
    }

    if (rule.survive[neighbours]) {
      gol_set_cell_state(new, node->x, node->y, ALIVE);
    }
  }

  free(cells);

  gol_free(*root_ptr);
  *root_ptr = new;
}

void gol_get_all_alive_cells(QtNode *node, QtNode ***cells_ptr,
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
    gol_get_all_alive_cells(node->children[i], cells_ptr, cells_size_ptr,
                            cell_count_ptr);
  }
}

ruleset parse_rule(char *_) {
  // TODO: implement properly

  // B3/S23
  return (ruleset){
      .birth = {false, false, false, true, false, false, false, false, false},
      .survive = {false, false, true, true, false, false, false, false, false},
  };
}
