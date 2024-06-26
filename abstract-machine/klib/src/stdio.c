#include <am.h>
#include <stdint.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define MAXLINE 0x1000
char printf_str[MAXLINE];

int printf(const char *fmt, ...) {
  va_list arg;
  va_start (arg, fmt);
  int done = vsprintf(printf_str, fmt, arg);
  va_end (arg);
  assert(done <= MAXLINE);
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

static int get_base_of_unsigned_int(char conversion_specifier){
  int base = 0;
  switch (conversion_specifier) {
    case 'o':
      base = 8;
      break;
    case 'u':
      base = 10;
      break;
    case 'x':
      base = 16;
      break;          
    default:
      break;
  }
  return base;
}

static char *itoa_abs_d(int num, char *str) {
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
static char *itoa_unsigned_int(unsigned int num, char *str, int base) {
  if(num == 0){
    *str = '0';
    return str;
  }
  char *bit = str;
  unsigned int num_rest = num;
  while( num_rest ) {
    int num_bit = num_rest % base;
    if(num_bit >= 0 && num_bit <= 9){
      *bit = num_bit + '0';
    } else if (num_bit > 9) {
      *bit = num_bit - 10 + 'a';
    } else {      
    }
    bit++;
    num_rest = num_rest / base;
  }
  *bit = '\0';
  reverse(str);
  return str;
}
static char *itoa_ptr(uintptr_t num, char *str) {
  if(num == 0){
    strcpy(str, "(nil)");
    return str;
  }
  int base = 16;
  char *bit = str;
  uintptr_t num_rest = num;
  while( num_rest ) {
    int num_bit = num_rest % base;
    if(num_bit >= 0 && num_bit <= 9){
      *bit = num_bit + '0';
    } else if (num_bit > 9) {
      *bit = num_bit - 10 + 'a';
    } else {      
    }
    bit++;
    num_rest = num_rest / base;
  }
  bit = bit + 2;
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
  case 'c':
  case 'o':
  case 'u':
  case 'x':
  case 'p':
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

void exect_converse_fmt_c(char **out_p_p, char c, int field_width) {
  size_t str_len = 1;
  if (field_width > str_len) {
    int space_len = field_width - str_len;
    while (space_len) {
      (**out_p_p) = ' ';
      (*out_p_p) ++;
      space_len--;
    }
  }
  (**out_p_p) = c;
  (*out_p_p) += str_len;
}

void exect_converse_fmt_unsigned_int(char **out_p_p, unsigned int num, bool is_zero_padded, int field_width, int base) {
  #define VSPRINTF_NUM_STR_LEN 33
  char num_str[VSPRINTF_NUM_STR_LEN] = "";
  memset(num_str, 0, VSPRINTF_NUM_STR_LEN);
  char *result = itoa_unsigned_int(num, num_str, base);
  assert(result != NULL);
  size_t abs_num_str_len = strlen(num_str);
  size_t num_str_len = abs_num_str_len;
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

void exect_converse_fmt_d(char **out_p_p, int num, bool is_zero_padded, int field_width) {
  #define VSPRINTF_NUM_STR_LEN 33
  char num_str[VSPRINTF_NUM_STR_LEN] = "";
  memset(num_str, 0, VSPRINTF_NUM_STR_LEN);
  bool is_negative = false;
  if(num < 0){
    is_negative = true;
  }
  char *result = itoa_abs_d(num, num_str);
  assert(result != NULL);
  size_t abs_num_str_len = strlen(num_str);
  size_t num_str_len = abs_num_str_len;
  if(is_negative){
    num_str_len++;
  }
  if(is_negative && is_zero_padded){
    (**out_p_p) = '-';
    (*out_p_p) ++;
  }
  if (field_width > num_str_len) {
    int space_len = field_width - num_str_len;
    while (space_len) {
      (**out_p_p) = is_zero_padded ? '0' : ' ';
      (*out_p_p) ++;
      space_len--;
    }
  }
  if(is_negative && !is_zero_padded){
    (**out_p_p) = '-';
    (*out_p_p) ++;
  }
  strncpy(*out_p_p, num_str, abs_num_str_len);
  (*out_p_p) += abs_num_str_len;
}

void exect_converse_fmt_p(char **out_p_p, uintptr_t address, bool is_zero_padded, int field_width) {
  #define VSPRINTF_NUM_STR_LEN 33
  char num_str[VSPRINTF_NUM_STR_LEN] = "";
  memset(num_str, 0, VSPRINTF_NUM_STR_LEN);
  char *result = itoa_ptr(address, num_str);
  assert(result != NULL);

  size_t abs_num_str_len = strlen(num_str);
  size_t num_str_len = abs_num_str_len;

  if (address) {
    num_str_len = num_str_len + 2;
    if(is_zero_padded){
      (**out_p_p) = '0';
      (*((*out_p_p) + 1)) = 'x';
      (*out_p_p) += 2;
    }
  } else {
    is_zero_padded = false;
  }
  if (field_width > num_str_len) {
    int space_len = field_width - num_str_len;
    while (space_len) {
      (**out_p_p) = is_zero_padded ? '0' : ' ';
      (*out_p_p) ++;
      space_len--;
    }
  }
  if(address && !is_zero_padded) {
    (**out_p_p) = '0';
    (*((*out_p_p) + 1)) = 'x';
    (*out_p_p) += 2;
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
      int base = 10;
      switch (conversion_specifier) {
        case 's':
          char *str = NULL;
          str = va_arg(ap, char*);
          exect_converse_fmt_s(&out_p, str, field_width);
          break;
        case 'c':
          char c = 0;
          c = (char) va_arg(ap, int);
          exect_converse_fmt_c(&out_p, c, field_width);
          break;
        case 'o':
        case 'u':
        case 'x':
          base = get_base_of_unsigned_int(conversion_specifier);
          unsigned int num_x = 0;
          num_x = va_arg(ap, unsigned int);
          exect_converse_fmt_unsigned_int(&out_p, num_x, is_zero_padded, field_width, base);
          break;
        case 'd':
          int num_d = 0;
          num_d = va_arg(ap, int);
          exect_converse_fmt_d(&out_p, num_d, is_zero_padded, field_width);
          break;
        case 'p':
          uintptr_t ptr_address = 0;
          ptr_address = va_arg(ap, uintptr_t);
          exect_converse_fmt_p(&out_p, ptr_address, is_zero_padded, field_width);
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
  char tmp[MAXLINE];
  
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = vsprintf (tmp, fmt, arg);
  va_end (arg);

  assert(done + 1 <= MAXLINE);

  strncpy(out, tmp, n);
  out[n - 1] = '\0';
  return done;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
