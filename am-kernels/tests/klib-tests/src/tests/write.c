#include <amtest.h>

#define N 32

#define check_char_seq(i) assert(str[(i)] == 'A' + (i))
#define check_char_eq(i, val) assert(str[(i)] == (val));

uint8_t data[N];
char str[N];

void reset()
{
  int i;
  for (i = 0; i < N; i++)
  {
    data[i] = i + 1;
  }
}

int check_out_range(int i)
{
  if (i < 0 || i >= N)
  {
    return 1;
  }
  return 0;
}

int check_overlap(int l1, int r1, int l2, int r2)
{
  assert(l1 <= r1);
  assert(l2 <= r2);
  if (l1 < l2 && r1 < l2)
  {
    return 0;
  }
  if (l2 < l1 && r2 < l1)
  {
    return 0;
  }
  return 1;
}

void check_seq(int l, int r)
{
  int i;
  for (i = l; i <= r; i++)
  {
    assert(data[i] == i + 1);
  }
}

void check_eq(int l, int r, int val)
{
  int i;
  for (i = l; i <= r; i++)
  {
    assert(data[i] == val);
  }
}

void reset_str()
{
  int i;
  for (i = 0; i < N; i++)
  {
    str[i] = 'A' + i;
  }
}

void str_log()
{
  int i;
  for (i = 0; i < N; i++)
  {
    if (str[i])
    {
      printf("%c", str[i]);
    }
    else
    {
      printf("%c", '!');
    }
  }
  printf("\n");
}

void test_memset()
{
  int l, r;
  for (l = 0; l < N; l++)
  {
    for (r = l + 1; r < N; r++)
    {
      reset();
      uint8_t val = (l + r) / 2;
      void *result = memset(data + l, val, r - l + 1);
      assert(result == (void *)(data + l));
      check_seq(0, l - 1);
      check_eq(l, r, val);
      check_seq(r + 1, N - 1);
    }
  }
}

void test_memcpy()
{
  int src, dest, n;
  for (src = 0; src < N; src++)
  {
    for (dest = 0; dest < N; dest++)
    {
      for (n = 1; n < N; n++)
      {
        int src_end = src + n - 1;
        int dest_end = dest + n - 1;
        if (check_out_range(src_end) || check_out_range(dest_end))
        {
          continue;
        }
        if (check_overlap(src, src_end, dest, dest_end))
        {
          continue;
        }
        reset();
        void *result = memcpy(data + dest, data + src, n);
        assert(result == (void *)(data + dest));

        for (int j = 0; j < N; j++)
        {
          if (j < dest || j > dest_end)
          {
            check_seq(j, j);
          }
        }
        for (int j = 0; j < n; j++)
        {
          check_eq(src + j, src + j, src + j + 1);
          check_eq(dest + j, dest + j, src + j + 1);
        }
      }
    }
  }
}

void test_memmove()
{
  int src, dest, n;
  for (src = 0; src < N; src++)
  {
    for (dest = 0; dest < N; dest++)
    {
      for (n = 1; n < N; n++)
      {
        int src_end = src + n - 1;
        int dest_end = dest + n - 1;
        if (check_out_range(src_end) || check_out_range(dest_end))
        {
          continue;
        }
        reset();
        char tmp[n];
        for (int i = 0; i < n; i++)
        {
          tmp[i] = data[src + i];
        }
        void *result = memmove(data + dest, data + src, n);
        assert(result == (void *)(data + dest));
        for (int j = 0; j < N; j++)
        {
          if (j < dest || j > dest_end)
          {
            check_seq(j, j);
          }
        }
        for (int j = 0; j < n; j++)
        {
          check_eq(dest + j, dest + j, tmp[j]);
        }
      }
    }
  }
}

void test_strcpy()
{
  int src, dest, n;
  for (src = 0; src < N; src++)
  {
    for (dest = 0; dest < N; dest++)
    {
      for (n = 1; n < N; n++)
      {
        int src_end = src + n - 1;
        int dest_end = dest + n - 1;
        if (check_out_range(src_end) || check_out_range(dest_end))
        {
          continue;
        }
        if (check_overlap(src, src_end, dest, dest_end))
        {
          continue;
        }
        reset_str();
        str[src_end] = 0;
        void *result = strcpy(str + dest, str + src);
        assert(result == (void *)(str + dest));

        for (int j = 0; j < N; j++)
        {
          if (j == src_end)
          {
            check_char_eq(src_end, 0);
          }
          else if (j < dest || j > dest_end)
          {
            check_char_seq(j);
          }
        }
        int j;
        for (j = 0; j < n - 1; j++)
        {
          check_char_eq(src + j, 'A' + src + j);
          check_char_eq(dest + j, 'A' + src + j);
        }
        check_char_eq(dest + j, 0);
      }
    }
  }
}

void test_strncpy()
{
  // case 1: the length of src is equal to n;
  char src[N] = "0123456789";
  reset_str();
  void *result = strncpy(str, src, 11);
  assert(result == (void *)str);
  for (int i = 0; i < 10; i++)
  {
    assert(src[i] == '0' + i);
  }
  assert(src[10] == 0);
  for (int i = 0; i < 10; i++)
  {
    check_char_eq(i, '0' + i);
  }
  check_char_eq(10, 0);
  for (int i = 11; i < N; i++)
  {
    check_char_seq(i);
  }
  // case 2: the length of src is longer to n;
  reset_str();
  result = strncpy(str, src, 5);
  assert(result == (void *)str);
  for (int i = 0; i < 10; i++)
  {
    assert(src[i] == '0' + i);
  }
  assert(src[10] == 0);
  for (int i = 0; i < 5; i++)
  {
    check_char_eq(i, '0' + i);
  }
  for (int i = 5; i < N; i++)
  {
    check_char_seq(i);
  }

  // case 3: the length of src is less to n;
  reset_str();
  result = strncpy(str, src, 20);
  assert(result == (void *)str);
  for (int i = 0; i < 10; i++)
  {
    assert(src[i] == '0' + i);
  }
  assert(src[10] == 0);
  for (int i = 0; i < 10; i++)
  {
    check_char_eq(i, '0' + i);
  }
  for (int i = 10; i < 20; i++)
  {
    check_char_eq(i, 0);
  }
  for (int i = 20; i < N; i++)
  {
    check_char_seq(i);
  }
}

void test_strcat()
{
  int src, dest;
  for (src = 0; src < N; src++)
  {
    for (dest = 0; dest < N; dest++)
    {
      for (int dest_len = 1; dest_len < N; dest_len++)
      {
        for (int src_len = 1; src_len < N; src_len++)
        {
          int src_end = src + src_len - 1;
          int dest_end = dest + dest_len - 1;
          int dest_new_end = dest + dest_len - 2 + src_len;
          if (check_out_range(src_end) || check_out_range(dest_end) || check_out_range(dest_new_end))
          {
            continue;
          }
          if (check_overlap(src, src_end, dest, dest_new_end))
          {
            continue;
          }
          reset_str();
          str[src_end] = 0;
          str[dest_end] = 0;
          void *result = strcat(str + dest, str + src);
          assert(result == (void *)(str + dest));

          for (int j = 0; j < N; j++)
          {
            if (j == src_end)
            {
              check_char_eq(src_end, 0);
            }
            else if (j < dest || j > dest_new_end)
            {
              check_char_seq(j);
            }
          }
          for (int j = dest; j < dest_end; j++)
          {
            check_char_eq(j, 'A' + j);
          }
          for (int j = 0; j < src_len - 1; j++)
          {
            check_char_eq(dest_end + j, 'A' + src + j);
          }
          check_char_eq(dest_end + src_len - 1, 0);
        }
      }
    }
  }
}

void test_write()
{
  test_memset();
  printf("memset pass\n");
  test_memcpy();
  printf("memcpy pass\n");
  test_memmove();
  printf("memmove pass\n");
  test_strcpy();
  printf("strcpy pass\n");
  test_strncpy();
  printf("strncpy pass\n");
  test_strcat();
  printf("strcat pass\n");
}
