#include <amtest.h>

void test_write();
void test_read();
void test_format();

int main(const char *args)
{
  test_write();
  test_read();
  test_format();
  printf("all pass\n");
  return 0;
}
