#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define MAXLINE 1024
char printf_str[MAXLINE];

int printf(const char *fmt, ...) {
  va_list arg;
  va_start (arg, fmt);
  int done = vsprintf(printf_str, fmt, arg);
  va_end (arg);
  size_t i = 0;
  char ch;
  while((ch = printf_str[i])){
    putch(ch);
    i++;
  }
  return done;
}

static void reverse(char *str) {
  for (char *left = str, *right = str + strlen(str) - 1; left < right; left++, right--){
    char tmp = *left;
    *left = *right;
    *right = tmp;
  }
}

static char *itoa_abs(int num, char *str) {
  if(num == 0){
    *str = '0';
    return str;
  }
  int base = 10;
  char *bit = str;
  int num_rest = num;
  while( num_rest ) {
    *bit = abs(num_rest % base) + '0';
    bit++;
    num_rest = num_rest / base;
  }
  *bit = '\0';
  reverse(str);
  return str;
}

void check_fmt_flag_character(char **fmt_p_p, bool *is_zero_padded) {
  char *fmt_p = *fmt_p_p;
  if(*fmt_p == '0'){
    *is_zero_padded = true;
    (*fmt_p_p)++;
  }
}

void check_fmt_field_width(char **fmt_p_p, int *field_width) {
  char *fmt_p = *fmt_p_p;
  int num = 0;
  while (*fmt_p >= '0' && *fmt_p <= '9'){
    num = num * 10 + *fmt_p - '0';
    fmt_p ++;
  }
  *field_width = num;
  *fmt_p_p = fmt_p;
}

void check_fmt_conversion_specifier(char **fmt_p_p, char *conversion_specifier) {
  char *fmt_p = *fmt_p_p;
  switch (*fmt_p) {
  case 's':
  case 'd':
    *conversion_specifier = *fmt_p;
    (*fmt_p_p)++;
    break;  
  default:
    panic("Not implemented more conversion specifier");
    break;
  }
}

void exect_converse_fmt_s(char **out_p_p, char *str, int field_width) {
  if (str) {
    size_t str_len = strlen(str);
    if (field_width > str_len) {
      int space_len = field_width - str_len;
      while (space_len) {
        (**out_p_p) = ' ';
        (*out_p_p) ++;
        space_len--;
      }
    }
    strncpy(*out_p_p, str, str_len);
    (*out_p_p) += str_len;
  }
}

void exect_converse_fmt_d(char **out_p_p, int num, bool is_zero_padded, int field_width) {
  #define VSPRINTF_NUM_STR_LEN 33
  char num_str[VSPRINTF_NUM_STR_LEN] = "";
  memset(num_str, 0, VSPRINTF_NUM_STR_LEN);
  bool is_negative = false;
  if(num < 0){
    is_negative = true;
  }
  char *result = itoa_abs(num, num_str);
  assert(result != NULL);
  size_t abs_num_str_len = strlen(num_str);
  size_t num_str_len = abs_num_str_len;
  if(is_negative){
    (**out_p_p) = '-';
    (*out_p_p) ++;
    num_str_len++;
  }
  if (field_width > num_str_len) {
    int space_len = field_width - num_str_len;
    while (space_len) {
      (**out_p_p) = is_zero_padded ? '0' : ' ';
      (*out_p_p) ++;
      space_len--;
    }
  }
  strncpy(*out_p_p, num_str, abs_num_str_len);
  (*out_p_p) += abs_num_str_len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *out_p = out;
  char *fmt_p = (char *) fmt;
  
  while (*fmt_p) {
    char fmt_c = *fmt_p;
    if(fmt_c == '%'){
      fmt_p++;
      bool is_zero_padded = false;
      check_fmt_flag_character(&fmt_p, &is_zero_padded);
      int field_width = 0;
      check_fmt_field_width(&fmt_p, &field_width);
      char conversion_specifier = 0;
      check_fmt_conversion_specifier(&fmt_p, &conversion_specifier);
      switch (conversion_specifier) {
        case 's':
          char *str = NULL;
          str = va_arg(ap, char*);
          exect_converse_fmt_s(&out_p, str, field_width);
          break;
        case 'd':
          int num = 0;
          num = va_arg(ap, int);
          exect_converse_fmt_d(&out_p, num, is_zero_padded, field_width);
          break;
        default:
          break;
      }
    } else {
      *out_p = fmt_c;
      out_p++;
      fmt_p++;
    }
  }
  *out_p = '\0';
  return out_p - out;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = vsprintf (out, fmt, arg);
  va_end (arg);

  return done;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
