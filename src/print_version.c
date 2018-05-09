#ifdef PRINT_VERSION_TARGET
#include <stdio.h>
#include "version.h"

/* This is a utility that prints the current semantic version string, to be used in make files */

int main(int argc, char **argv) {
  printf("%d.%d.%d\n", REDISML_VERSION_MAJOR, REDISML_VERSION_MINOR,
         REDISML_VERSION_PATCH);
  return 0;
}
#endif
