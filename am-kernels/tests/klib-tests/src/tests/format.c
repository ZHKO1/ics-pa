#include <amtest.h>
#include <limits.h>

void test_sprintf();
void test_snprintf();

void test_format()
{
  test_sprintf();
  printf("sprintf pass\n");
  test_snprintf();
  printf("snprintf pass\n");
}
