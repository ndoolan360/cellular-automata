#ifndef GOL_H
#define GOL_H

#include "grid.h"
#include <stdbool.h>
#include <stddef.h>

typedef enum { DEAD = 0, ALIVE = 1 } cell_state_t;

typedef struct {
  cell_state_t state;
} cell_data_t;

QtNode *gol_init(size_t size);
void gol_free(QtNode *root);

cell_data_t gol_get_cell_data(QtNode *root, int x, int y);
void gol_toggle_cell_state(QtNode *root, int x, int y);

void gol_step(QtNode **root_ptr);

#endif // GOL_H
