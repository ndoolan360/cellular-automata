#ifndef CELLS_H
#define CELLS_H

#include "grid.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum { DEAD = 0, ALIVE = 1 } cell_state_t;

typedef struct {
  cell_state_t state;
} cell_data_t;

typedef struct {
  bool wrap;
  bool birth[9];
  bool survive[9];
} ruleset;

QtNode *cells_init(size_t size);
void cells_free(QtNode *root);

cell_data_t cells_get_cell_data(QtNode *root, int x, int y);
void cells_toggle_cell_state(QtNode *root, int x, int y);

void cells_step(QtNode **root_ptr, ruleset rule);
void cells_get_all_alive_cells(QtNode *root, QtNode ***cells_ptr, size_t *cells_size_ptr, size_t *cell_count_ptr);

ruleset parse_rule(char *rule_str, bool wrap);

#endif // CELLS_H
