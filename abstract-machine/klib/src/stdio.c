#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

static char *itoa(int num, char *str, size_t max_bits){
  int base = 10;
  size_t bits = 0;
  if(num < 0){
    bits++;
  }
  int num_tmp = num;
  while( num_tmp ){
    num_tmp = num_tmp / base;
    bits++;
  }
  bits++; // 考虑到末尾的\0
  if (bits > max_bits) {
    return NULL;
  }
  char *result = str;
  if(num < 0){
    num = -num;
    *str = '-';
    str++;
  }
  char *start = str;
  while( num ){
    *str = num % base + '0';
    str++;
    num = num / base;
  }
  *str = '\0';
  for (char *left = start, *right = str - 1; left < right; left++, right--){
    char tmp = *left;
    *left = *right;
    *right = tmp;
  }
  return result;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *out_p = out;
  char *fmt_p = (char *) fmt;
  
  int num = 0;
  #define VSPRINTF_NUM_STR_LEN 33
  char num_str[VSPRINTF_NUM_STR_LEN] = "";
  char *str = NULL;

  while(*fmt_p){
    char fmt_c = *fmt_p;
    if(fmt_c == '%'){
      fmt_p++;
      fmt_c = *fmt_p;
      switch (fmt_c) {
        case 's':
          str = va_arg(ap, char*);
          size_t str_len = strlen(str);
          strncpy(out_p, str, str_len);
          out_p += str_len;
          break;
        case 'd':
          num = va_arg(ap, int);
          char *result = itoa(num, num_str, VSPRINTF_NUM_STR_LEN);
          if(!result){
            panic("Error in itoa");
          }
          size_t num_str_len = strlen(num_str);
          strncpy(out_p, num_str, num_str_len);
          out_p += num_str_len;
          break;
        default:
          panic("Not implemented more template");
          break;
      }
    } else {
      *out_p = fmt_c;
      out_p++;
    }
    fmt_p++;
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
