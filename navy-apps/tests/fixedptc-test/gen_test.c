#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <math.h>
#include <fixedptc.h>

// 这里最大的问题是fixedptc本身精度不是很高，再加上范围的计算很费脑子，所以测试例子挺难搞的
// 因此这里只测试最简单最基本的例子
// 这里fixedpt_approx_equal宏仅用于乘法和除法，放弃二进制最小两位的精度检查

static const float vsf[] = {
    -190.1233213123,
    -50,
    -7.655234,
    -4.31987,
    -3,
    -2.123019021,
    0,
    1.65421,
    2,
    5.9183,
    8,
    60.123311354,
    10321.123,
};



static const int vsi[] = {
    -10,
    -7,
    -4,
    -1,
    0,
    1,
    2,
    3,
    5,
    12,
};

#define FOR_SET_SINGLE(set)                                                                          \
  {                                                                                                  \
    int i;                                                                                           \
    int c = sizeof((set)) / sizeof(*(set));                                                          \
    for (i = 0; i < c; i++)                                                                          \
    {                                                                                                \
      float s = set[i];                                                                              \
      float s_abs = fabs(s);                                                                         \
      float s_floor = floorf(s);                                                                     \
      float s_ceil = ceilf(s);                                                                       \
      printf("  {\n");                                                                               \
      printf("    fixedpt num = fixedpt_rconst(%f);\n", s);                                          \
      /*printf("    printf(\"================\\n\");\n");                                            \
      printf("    printf(\"num=%%x\\n\", num);\n");                                                  \
      printf("    printf(\"abs=%%x %%x\\n\", fixedpt_abs(num), fixedpt_rconst(%f));\n", s_abs);      \
      printf("    printf(\"floor=%%x %%x\\n\", fixedpt_floor(num),fixedpt_rconst(%f));\n", s_floor); \
      printf("    printf(\"ceil=%%x %%x\\n\", fixedpt_ceil(num),fixedpt_rconst(%f));\n", s_ceil);*/  \
      printf("    assert(fixedpt_abs(num) == fixedpt_rconst(%f));\n", s_abs);                        \
      printf("    assert(fixedpt_floor(num) == fixedpt_rconst(%f));\n", s_floor);                    \
      printf("    assert(fixedpt_ceil(num) == fixedpt_rconst(%f));\n", s_ceil);                      \
      printf("  }\n");                                                                               \
    }                                                                                                \
  }

#define FOR_SET_MUL(set)                                                                 \
  {                                                                                      \
    int i, j;                                                                            \
    int c = sizeof((set)) / sizeof(*(set));                                              \
    for (i = 0; i < c; i++)                                                              \
    {                                                                                    \
      for (j = 0; j < c; j++)                                                            \
      {                                                                                  \
        float s_i = set[i];                                                              \
        float s_j = set[j];                                                              \
        float s_mul = s_i * s_j;                                                         \
        printf("  {\n");                                                                 \
        printf("    fixedpt num_i = fixedpt_rconst(%f);\n", s_i);                        \
        printf("    fixedpt num_j = fixedpt_rconst(%f);\n", s_j);                        \
        printf("    assert(fixedpt_approx_equal(fixedpt_muli(num_i, num_j), fixedpt_rconst(%f)));\n", s_mul); \
        printf("  }\n");                                                                 \
      }                                                                                  \
    }                                                                                    \
  }

#define FOR_SET_MULI(set_f, set_i)                                                          \
  {                                                                                         \
    int i, j;                                                                               \
    int set_f_len = sizeof((set_f)) / sizeof(*(set_f));                                     \
    int set_i_len = sizeof((set_i)) / sizeof(*(set_i));                                     \
    for (i = 0; i < set_f_len; i++)                                                         \
    {                                                                                       \
      for (j = 0; j < set_i_len; j++)                                                       \
      {                                                                                     \
        float s_i = set_f[i];                                                               \
        int s_j = set_i[j];                                                                 \
        float s_mul = s_i * s_j;                                                            \
        printf("  {\n");                                                                    \
        printf("    fixedpt num_i = fixedpt_rconst(%f);\n", s_i);                           \
        printf("    assert(fixedpt_approx_equal(fixedpt_muli(num_i, %d), fixedpt_rconst(%f)));\n", s_j, s_mul); \
        printf("  }\n");                                                                    \
      }                                                                                     \
    }                                                                                       \
  }

#define FOR_SET_DIV(set)                                                                                     \
  {                                                                                                          \
    int i, j;                                                                                                \
    int c = sizeof((set)) / sizeof(*(set));                                                                  \
    for (i = 0; i < c; i++)                                                                                  \
    {                                                                                                        \
      for (j = 0; j < c; j++)                                                                                \
      {                                                                                                      \
        float s_i = set[i];                                                                                  \
        float s_j = set[j];                                                                                  \
        if (s_j == 0)                                                                                        \
        {                                                                                                    \
          continue;                                                                                          \
        }                                                                                                    \
        float s_div = s_i / s_j;                                                                             \
        printf("  {\n");                                                                                     \
        printf("    fixedpt num_i = fixedpt_rconst(%f);\n", s_i);                                            \
        printf("    fixedpt num_j = fixedpt_rconst(%f);\n", s_j);                                            \
        printf("    assert(fixedpt_approx_equal(fixedpt_div(num_i, num_j), fixedpt_rconst(%f)));\n", s_div); \
        printf("  }\n");                                                                                     \
      }                                                                                                      \
    }                                                                                                        \
  }

#define FOR_SET_DIVI(set_f, set_i)                                                                              \
  {                                                                                                             \
    int i, j;                                                                                                   \
    int set_f_len = sizeof((set_f)) / sizeof(*(set_f));                                                         \
    int set_i_len = sizeof((set_i)) / sizeof(*(set_i));                                                         \
    for (i = 0; i < set_f_len; i++)                                                                             \
    {                                                                                                           \
      for (j = 0; j < set_i_len; j++)                                                                           \
      {                                                                                                         \
        float s_i = set_f[i];                                                                                   \
        int s_j = set_i[j];                                                                                     \
        if (s_j == 0)                                                                                           \
        {                                                                                                       \
          continue;                                                                                             \
        }                                                                                                       \
        float s_div = s_i / s_j;                                                                                \
        printf("  {\n");                                                                                        \
        printf("    fixedpt num_i = fixedpt_rconst(%f);\n", s_i);                                               \
        printf("    assert(fixedpt_approx_equal(fixedpt_divi(num_i, %d), fixedpt_rconst(%f)));\n", s_j, s_div); \
        printf("  }\n");                                                                                        \
      }                                                                                                         \
    }                                                                                                           \
  }

int main(void)
{
  printf("#include <stdio.h>\n");
  printf("#include <fixedptc.h>\n");
  printf("#include <assert.h>\n");
  printf("#define fixedpt_approx_equal(num1, num2) ((num1 && 0xFFFFFFFC) == (num2 && 0xFFFFFFFC))\n");
  printf("int main(void) {\n");

  FOR_SET_SINGLE(vsf);

  FOR_SET_MUL(vsf);
  FOR_SET_MULI(vsf, vsi);

  FOR_SET_DIV(vsf);
  FOR_SET_DIVI(vsf, vsi);

  printf("}\n");

  return 0;
}
