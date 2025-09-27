# Cellular Automoata

An implementation of conways game of life.

### TODO

- finish `ruleset parse_rule(char *rule_str)`...
- control frame limit better (curses)
- add alternate view method (raylib?)
- support wrapping cells with input arg
- support resizing grid at runtime
- support infinite grid

### Optimisation thoughts

- step function with access to parent on node to avoid having to call `gol_get_cell_data`
    - could use child [0] on leaf nodes to reference parent? (feels unintuituve though)
- cell_data_t to just be a 1 bit flag, doesn't need to be a struct
