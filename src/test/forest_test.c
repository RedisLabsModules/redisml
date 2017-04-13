#include <stdio.h>
#include <stdlib.h>

#include "../forest.h"
#include "minunit.h"

MU_TEST(add_node_ok) {
    Forest_Tree t = {.root = NULL};
    __forest_Node *n = Forest_NewNumericalNode("1", 1.0);
    int rc = Forest_TreeAdd(&t.root, ".", n);
    mu_check(rc == FOREST_OK);
}

MU_TEST(add_node_wrong_path) {
    Forest_Tree t = {.root = NULL};
    __forest_Node *n = Forest_NewNumericalNode("1", 1.0);
    int rc = Forest_TreeAdd(&t.root, ".r", n);
    mu_check(rc == FOREST_ERR);
}

MU_TEST_SUITE(test_suite) {
    MU_RUN_TEST(add_node_ok);
    MU_RUN_TEST(add_node_wrong_path);
}

int main(int argc, char **argv) {
    printf("Testing forest\n");
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    exit(EXIT_SUCCESS);
}
