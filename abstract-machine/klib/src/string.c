#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while(*(s + len)){
    len++;
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
  size_t len = 0;
  char src_c = 0;
  while((src_c = src[len])) {
    dst[len] = src_c;
    len++;
  }
  dst[len] = 0;
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t len = 0;
  char src_c = 0;
  while((src_c = src[len]) && (len < n)) {
    dst[len] = src_c;
    len++;
  }
  while( len < n){
    dst[len] = 0;
    len++;
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t dst_len = strlen(dst);
  size_t len = 0;
  char src_c = '\0';
  while((src_c = src[len])) {
    dst[dst_len + len] = src_c;
    len++;
  }
  dst[dst_len + len] = 0;
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  size_t len = 0;
  char s1_c= *(s1 + len);
  char s2_c= *(s2 + len);
  while( s1 && s2_c ) {
    if(s1_c == s2_c){
      len++;
      s1_c = *(s1 + len);
      s2_c = *(s2 + len);
    } else {
      break;
    }
  }
  return (int)((unsigned char)s1_c - (unsigned char)s2_c);
}

int strncmp(const char *s1, const char *s2, size_t n) {
  char s1_c;
  char s2_c;
  for (size_t len = 0; len < n; len++)
  {
    s1_c = *(s1 + len);
    s2_c = *(s2 + len);
    if (s1_c != s2_c)
    {
      return (int)((unsigned char)s1_c - (unsigned char)s2_c);
    }
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  for(size_t i = 0; i < n; i++){
    *((char *)s + i) = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  char tmp[n];
  for (size_t i = 0; i < n; i++) {
    *((char *)tmp + i) = *((char *)src + i);
  }
  for(size_t i = 0; i < n; i++) {
    *((char *)dst + i) = *((char *)tmp + i);
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  for(size_t i = 0; i < n; i++){
    *((char *)out + i) = *((char *)in + i);
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  for(size_t i = 0; i < n; i++){
    unsigned char s1_v = *((unsigned char *)s1 + i);
    unsigned char s2_v = *((unsigned char *)s2 + i);
    if(s1_v != s2_v){
      return (int) s1_v - s2_v;
    }
  }
  return 0;
}

#endif
