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

void gol_set_cell_state(QtNode *root, int x, int y, cell_state_t state) {
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

static struct {
  int dx;
  int dy;
} OFFSETS[8] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0},
                {1, 0},   {-1, 1}, {0, 1},  {1, 1}};

static bool SURVIVE[9] = {false, false, true, true, false, false, false, false, false};
static bool BIRTH[9] = {false, false, false, true, false, false, false, false, false};

static QtNode *step(QtNode *root) {
  QtNode *new = gol_init(root->size);

  for (int y = 0; y < (int)root->size; y++) {
    for (int x = 0; x < (int)root->size; x++) {
      bool is_alive = gol_get_cell_data(root, x, y).state == ALIVE;
      uint8_t neighbours = 0;
      for (int i = 0; i < 8; i++) {
        cell_data_t neighbour =
            gol_get_cell_data(root, x + OFFSETS[i].dx, y + OFFSETS[i].dy);
        neighbours += neighbour.state;
      }

      if ((BIRTH[neighbours] && !is_alive) ||
          (SURVIVE[neighbours] && is_alive)) {
        gol_set_cell_state(new, x, y, ALIVE);
      }
    }
  }

  return new;
}

void gol_step(QtNode **root_ptr) {
  QtNode *new_root = step(*root_ptr);
  gol_free(*root_ptr);
  *root_ptr = new_root;
}
