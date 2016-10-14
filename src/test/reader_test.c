#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../reader.h"
#include "minunit.h"

MU_TEST(input_ok) {
  Field *irFields = NULL;
  InputRow testIR = {0, irFields};
  int ret = MakeInputRow("1:0.2,3:4,5:6,7:8", &testIR);
  mu_check(ret == 0);
}

MU_TEST(missing_val) {
  Field *irFields = NULL;
  InputRow testIR = {0, irFields};
  int ret = MakeInputRow("1:0.2,3:4,5:6,7:", &testIR);
  mu_check(ret == 1);
}

MU_TEST(all_wrong) {
  Field *irFields = NULL;
  InputRow testIR = {0, irFields};
  int ret = MakeInputRow("1", &testIR);
  mu_check(ret == 1);
}

MU_TEST_SUITE(test_suite) {
  MU_RUN_TEST(input_ok);
  MU_RUN_TEST(missing_val);
  MU_RUN_TEST(all_wrong);
}

int main(int argc, char **argv) {
  printf("Testing reader\n");
  MU_RUN_SUITE(test_suite);
  MU_REPORT();
  exit(EXIT_SUCCESS);
}
