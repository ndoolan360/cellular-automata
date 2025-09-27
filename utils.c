#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int parse_int_arg(char flag, int min, int max) {
  char *endp = NULL;
  long l = -1;
  if (!optarg || ((l = strtol(optarg, &endp, 10)), (endp && *endp))) {
    fprintf(stderr, "invalid %c option %s - expecting a number\n", flag,
            optarg ? optarg : "<NULL>");
    exit(EXIT_FAILURE);
  };
  if (l < min) {
    fprintf(stderr, "invalid %c option: '%s' - must be greater than '%d'\n", flag,
            optarg ? optarg : "<NULL>", min);
    exit(EXIT_FAILURE);
  }
  if (l > max) {
    fprintf(stderr, "invalid %c option: '%s' - must be less than '%d'\n", flag,
            optarg ? optarg : "<NULL>", max);
    exit(EXIT_FAILURE);
  }
  return (int)l;
}
