#include <amtest.h>

void test_strlen()
{
  assert(strlen("") == 0);
  assert(strlen("abc") == 3);
  assert(strlen("1234567890") == 10);
  assert(strlen("Albert. When I came to you with those calculations, we thought we might start a chain reaction, that would destroy the entire world. Mm, I remember it well. What of it? I believe we did.") == 186);
}

void test_strcmp()
{
  assert(strcmp("", "") == 0);
  assert(strcmp("a", "") == 97);
  assert(strcmp("", "v") == -118);
  assert(strcmp("abcd", "") == 97);
  assert(strcmp("1234", "1235") == -1);
  assert(strcmp("ABCD", "ABCA") == 3);
  assert(strcmp("ABCDEF", "ABCDEF") == 0);
  assert(strcmp("The Legend of Zelda: Twilight Princess", "The Legend of Zelda: Skyward Sword") == 1);
  assert(strcmp("The Legend of Zelda: Ocarina of Time", "The Legend of Zelda: Wind Waker") == -8);
}

void test_strncmp()
{
  assert(strncmp("", "", 0) == 0);
  assert(strncmp("", "", 10) == 0);
  assert(strncmp("a", "", 10) == 97);
  assert(strncmp("", "v", 10) == -118);
  assert(strncmp("abcd", "", 10) == 97);
  assert(strncmp("1234", "1235", 10) == -1);
  assert(strncmp("1234", "1235", 3) == 0);
  assert(strncmp("ABCD", "ABCA", 4) == 3);
  assert(strncmp("ABCD", "ABCA", 3) == 0);
  assert(strncmp("ABCDEF", "ABCDEF", 10) == 0);
  assert(strncmp("The Legend of Zelda: Ocarina of Time", "The Legend of Zelda: Wind Waker", 10) == 0);
  assert(strncmp("The Legend of Zelda: Ocarina of Time", "The Legend of Zelda: Wind Waker", 21) == 0);
  assert(strncmp("The Legend of Zelda: Ocarina of Time", "The Legend of Zelda: Wind Waker", 22) == -8);
}

void test_memcmp()
{
  uint8_t mem1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  assert(memcmp(mem1 + 0, mem1 + 0, 5) == 0);
  assert(memcmp(mem1 + 2, mem1 + 2, 8) == 0);
  assert(memcmp(mem1 + 0, mem1 + 1, 5) == -1);
  assert(memcmp(mem1 + 2, mem1 + 0, 5) == 2);
  assert(memcmp(mem1 + 1, mem1 + 5, 5) == -4);
  assert(memcmp(mem1 + 6, mem1 + 1, 4) == 5);
  uint8_t mem2[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 22};
  assert(memcmp(mem1, mem2, 10) == -13);
}

void test_read()
{
  test_strlen();
  // printf("strlen pass\n");
  test_strcmp();
  // printf("strcmp pass\n");
  test_strncmp();
  // printf("strncmp pass\n");
  test_memcmp();
  // printf("memcmp pass\n");
}
