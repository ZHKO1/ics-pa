#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#define MAXLINE 1024
static char str[MAXLINE] = "";
static const int vsi[] = {0, INT_MAX / 17, INT_MAX, INT_MIN, INT_MIN + 1, UINT_MAX / 17, INT_MAX / 1200, UINT_MAX};
__uint64_t vsu64[] = {0, __UINT64_MAX__, 0x8000000000000000UL, __UINT64_MAX__ / 1200, __UINT64_MAX__ / 7777};
__attribute__((unused)) static const signed char vsc[] = {'a', '!', '>', 'B'};
static const char *vss[] = {"", "Dragonlash Flame", "Shippu Jinrai-kyaku", "Shinryu Reppa"};
static int done = 0;
#ifdef __TEST_SNPRINTF__
  static int snprintf_lens[] = {0, 10, 15, 20, 40};
  static int snprintf_len = 5;
#endif

#define FORMAT(d) ("%" #d)
#define FORMAT_WITH_WIDTH(d, width) ("%" #width #d)

#ifdef __TEST_SNPRINTF__
#define FOR_NONE_FUNC(string) done = snprintf(str, snprintf_len, string);
#define FOR_NONE_FUNC_P(string) printf("    done = snprintf(str, %d, \"" string "\");\n", snprintf_len);
#define FOR_SET_FUNC(set) done = snprintf(str, snprintf_len, s, set[i]);
#define FOR_SET_FUNC_P(set_format, set_type, set) printf("    done = snprintf(str, %d, \"%s\", (%s)" set_format ");\n", snprintf_len, s, #set_type, set[i]);
#else
#define FOR_NONE_FUNC(string) done = sprintf(str, string);
#define FOR_NONE_FUNC_P(string) printf("    done = sprintf(str, \"" string "\");\n");
#define FOR_SET_FUNC(set) done = sprintf(str, s, set[i]);
#define FOR_SET_FUNC_P(set_format, set_type, set) printf("    done = sprintf(str, \"%s\", (%s)" set_format ");\n", s, #set_type, set[i]);
#endif

#define FOR_NONE(string)                                    \
  {                                                         \
    memset(str, 0, MAXLINE);                                \
    FOR_NONE_FUNC(string)                                   \
    printf("  {\n");                                        \
    printf("    memset(str, 0, MAXLINE);\n");               \
    FOR_NONE_FUNC_P(string)                                 \
    printf("    assert(done == %d);\n", done);              \
    printf("    assert(strcmp(str, \"%s\") == 0);\n", str); \
    printf("  }\n");                                        \
  }

#define FOR_SET(string, set_type, set, set_format, format)    \
  {                                                           \
    int i;                                                    \
    int c = sizeof((set)) / sizeof(*(set));                   \
    char *t = format;                                         \
    size_t t_len = strlen(t);                                 \
    for (i = 0; i < c; i++)                                   \
    {                                                         \
      char s[MAXLINE] = string;                               \
      size_t s_len = strlen(s);                               \
      assert(s_len >= t_len);                                 \
      memcpy(s + (i % (s_len - t_len + 1)), t, t_len);        \
      memset(str, 0, MAXLINE);                                \
      FOR_SET_FUNC(set)                                       \
      printf("  {\n");                                        \
      printf("    memset(str, 0, MAXLINE);\n");               \
      FOR_SET_FUNC_P(set_format, set_type, set)               \
      printf("    assert(done == %d);\n", done);              \
      printf("    assert(strcmp(str, \"%s\") == 0);\n", str); \
      printf("  }\n");                                        \
    }                                                         \
  }

#define FOR_SET_ALL(set_type, set, set_format, format_type)      \
  {                                                              \
    FOR_SET_COMMON(set_type, set, set_format, format_type);      \
    FOR_SET_FILEDWIDTH(set_type, set, set_format, format_type);  \
    FOR_SET_ZEROPADDING(set_type, set, set_format, format_type); \
  }

#define FOR_SET_COMMON(set_type, set, set_format, format_type)                    \
  {                                                                               \
    FOR_SET(FORMAT(format_type), set_type, set, set_format, FORMAT(format_type)); \
    FOR_SET("abcdefghi", set_type, set, set_format, FORMAT(format_type));         \
  }

#define FOR_SET_FILEDWIDTH(set_type, set, set_format, format_type)                                              \
  {                                                                                                             \
    FOR_SET(FORMAT_WITH_WIDTH(format_type, 10), set_type, set, set_format, FORMAT_WITH_WIDTH(format_type, 10)); \
    FOR_SET("ABCDEFGHI", set_type, set, set_format, FORMAT_WITH_WIDTH(format_type, 10));                        \
  }

#define FOR_SET_ZEROPADDING(set_type, set, set_format, format_type)                                               \
  {                                                                                                               \
    FOR_SET(FORMAT_WITH_WIDTH(format_type, 010), set_type, set, set_format, FORMAT_WITH_WIDTH(format_type, 010)); \
    FOR_SET("123456789", set_type, set, set_format, FORMAT_WITH_WIDTH(format_type, 010));                         \
  }

int main(void)
{
  printf("#include <amtest.h>\n");
  printf("#include <limits.h>\n");
  printf("#define MAXLINE 1024\n");
  printf("static char str[MAXLINE] = \"\";\n");
  // printf("static int vsi[] = {0, INT_MAX / 17, INT_MAX, INT_MIN, INT_MIN + 1, UINT_MAX / 17, INT_MAX / 17, UINT_MAX};\n");
  printf("static int done = 0;\n");
#ifdef __TEST_SNPRINTF__
  printf("void test_snprintf(void) {\n");
#else
  printf("void test_sprintf(void) {\n");
#endif

#ifdef __TEST_SNPRINTF__
int index = sizeof(snprintf_lens) / sizeof(int);
for(int i = 0; i < index; i++) {
  snprintf_len = snprintf_lens[i];
#endif

  FOR_NONE("");
  FOR_NONE("abcdefgh");

  FOR_SET_ALL(signed int, vsi, "%d", d);
  FOR_SET_ALL(unsigned int, vsi, "%d", o);
  FOR_SET_ALL(unsigned int, vsi, "%d", u);
  FOR_SET_ALL(unsigned int, vsi, "%d", x);
  FOR_SET_ALL(char, vsc, "%d", c);
  FOR_SET_ALL(char *, vss, "\"%s\"", s);

#if defined(__ISA_NATIVE__)
  FOR_SET_ALL(void *, vsu64, "0x%lx", p);
#elif defined(__ISA_RISCV32__)
  FOR_SET_ALL(void *, vsi, "0x%x", p);
#elif
# error unsupported ISA __ISA__
#endif
#ifdef __TEST_SNPRINTF__
}
#endif
  printf("}\n");

  return 0;
}
