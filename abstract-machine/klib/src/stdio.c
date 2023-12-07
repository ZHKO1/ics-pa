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

static char *itoa(int num, char *str){
  int base = 10;
  if(num == 0){
    *str = '0';
    return str;
  }
  char *bit = str;
  if(num < 0){
    *bit = '-';
    bit++;
  }
  char *start = bit;
  while( num ){
    *bit = abs(num % base) + '0';
    bit++;
    num = num / base;
  }
  *bit = '\0';
  for (char *left = start, *right = bit - 1; left < right; left++, right--){
    char tmp = *left;
    *left = *right;
    *right = tmp;
  }
  return str;
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
          str = NULL;
          str = va_arg(ap, char*);
          if (str) {
            size_t str_len = strlen(str);
            strncpy(out_p, str, str_len);
            out_p += str_len;
          }
          break;
        case 'd':
          num = 0;
          num = va_arg(ap, int);
          memset(num_str, 0, VSPRINTF_NUM_STR_LEN);
          char *result = itoa(num, num_str);
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
