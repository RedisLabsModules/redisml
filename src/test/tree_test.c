#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../tree.h"
#include "minunit.h"

MU_TEST(add_node_ok) {
  Tree t = {.root = NULL};
  Node *n = NewNumericalNode("1", 1.0);
  int rc = TreeAdd(&t.root, ".", n);
  mu_check(rc == TREE_OK);
}

MU_TEST(add_node_wrong_path) {
  Tree t = {.root = NULL};
  Node *n = NewNumericalNode("1", 1.0);
  int rc = TreeAdd(&t.root, ".r", n);
  mu_check(rc == TREE_ERR);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(add_node_ok);
  MU_RUN_TEST(add_node_wrong_path);
}

int main(int argc, char **argv) {
  printf("Testing reader\n");
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  exit(EXIT_SUCCESS);
}
