#ifndef GRID_H
#define GRID_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// 64 bytes
typedef struct QtNode {
  int x, y;
  size_t size;
  void *data;
  size_t data_size;
  struct QtNode *children[4];
} QtNode;

QtNode *grid_init(size_t size, size_t data_size);
void grid_free_node(QtNode *node);

QtNode *grid_get_cell(QtNode *root, int x, int y);
QtNode *grid_put_cell(QtNode *root, int x, int y, void *data);
bool grid_remove_cell(QtNode *root, int x, int y);

void grid_resize(QtNode **root_ptr, size_t new_size);

#endif // GRID_H
