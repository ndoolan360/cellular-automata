#include "grid.h"
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static QtNode *grid_init_node(int x, int y, size_t size, void *data,
                              size_t data_size) {
  QtNode *root = malloc(sizeof(QtNode));
  assert(root);

  root->size = size;
  root->data_size = data_size;
  root->data = data;
  root->x = x;
  root->y = y;
  for (int i = 0; i < 4; i++) {
    root->children[i] = NULL;
  }

  return root;
}

QtNode *grid_init(size_t size, size_t data_size) {
  size_t size_round_up = 1;
  while (size_round_up < size) {
    size_round_up <<= 1;
  }

  return grid_init_node(0, 0, size_round_up, NULL, data_size);
}

void grid_free_node(QtNode *node) {
  if (node == NULL) {
    return;
  }

  for (int i = 0; i < 4; i++) {
    grid_free_node(node->children[i]);
  }

  if (node->data != NULL) {
    free(node->data);
  }
  if (node != NULL) {
    free(node);
  }
}

static bool coords_in_node(QtNode *node, int x, int y) {
  return (node->x <= x && x < node->x + (int)node->size && node->y <= y &&
          y < node->y + (int)node->size);
}

static int8_t compute_coord_quadrant(QtNode *node, int x, int y) {
  assert(node);
  if (node->size <= 1) {
    return -1;
  }

  size_t new_size = node->size >> 1;
  return (y < node->y + (int)new_size ? 0 : 2) +
         (x < node->x + (int)new_size ? 0 : 1);
}

QtNode *grid_get_cell(QtNode *root, int x, int y) {
  if (!coords_in_node(root, x, y)) {
    return NULL;
  }

  QtNode *node = root;
  while (node != NULL && node->size > 1) {
    int8_t quadrant = compute_coord_quadrant(node, x, y);
    assert(quadrant >= 0);
    node = node->children[quadrant];
  }

  return node;
}

QtNode *grid_put_cell(QtNode *root, int x, int y, void *data) {
  if (!coords_in_node(root, x, y)) {
    return NULL;
  }

  QtNode *node = root;
  while (node->size > 1) {
    int8_t quadrant = compute_coord_quadrant(node, x, y);
    size_t new_size = node->size >> 1;

    if (node->children[quadrant] == NULL) {
      int new_x = node->x + ((quadrant % 2) * new_size);
      int new_y = node->y + ((quadrant >> 1) * new_size);

      void *new_data = NULL;
      if (quadrant < 0) {
        new_data = data;
      }

      node->children[quadrant] =
          grid_init_node(new_x, new_y, new_size, new_data, node->data_size);
    } else if (node->children[quadrant]->data != NULL) {
      // overwrite data
      free(node->children[quadrant]->data);
      node->children[quadrant]->data = data;
    }
    node = node->children[quadrant];
  }

  return node;
}

static bool has_children(QtNode *node) {
  if (node == NULL) {
    return false;
  }

  for (int i = 0; i < 4; i++) {
    if (node->children[i] != NULL) {
      return true;
    }
  }
  return false;
}

static bool grid_remove_cell_but_not_root(QtNode *real_root, QtNode *node,
                                          int x, int y) {
  if (node == NULL || !coords_in_node(node, x, y)) {
    return false;
  }

  int8_t quadrant = compute_coord_quadrant(node, x, y);

  // leaf node with data
  if (quadrant < 0) {
    grid_free_node(node);
    return true;
  }

  // remove child
  if (grid_remove_cell_but_not_root(real_root, node->children[quadrant], x,
                                    y)) {
    node->children[quadrant] = NULL;

    // if now empty, prune the branch (but not the real_root)
    if (!has_children(node) && node != real_root) {
      grid_free_node(node);
      return true;
    }
  }

  return false;
}

bool grid_remove_cell(QtNode *root, int x, int y) {
  return grid_remove_cell_but_not_root(root, root, x, y);
}

void grid_resize(QtNode **root_ptr, size_t new_size) {
  QtNode *root = *root_ptr;

  // shrink
  while ((root->size >> 1) >= new_size && root->size > 2) {
    if (!has_children(root)) {
      root->size >>= 1;
    } else {
      // check if can shrink - don't want to lose cells
      if (has_children(root->children[1]) || has_children(root->children[2]) ||
          has_children(root->children[3])) {
        break;
      }

      // shrink to top left quadrant (child 0)
      *root_ptr = root->children[0];
      root->children[0] = NULL;
      grid_free_node(root);
    }

    root = *root_ptr;
  }

  // grow
  while (root->size < new_size) {
    if (root->size < 2) {
      root->size = 2;
    }

    QtNode *new_root =
        grid_init_node(0, 0, root->size << 1, NULL, root->data_size);
    new_root->children[0] = root;
    *root_ptr = new_root;

    root = *root_ptr;
  }
}
