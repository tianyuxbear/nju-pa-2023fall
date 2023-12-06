#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while(*s != '\0'){
    s++;
    len++;
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
  char* ptr = dst;
  while(*src != '\0'){
    *dst++ = *src++;
  }
  *dst = '\0';

  return ptr;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  for(i = 0; i < n && src[i] != '\0'; i++)
    dst[i] = src[i];
  for(; i < n; i++)
    dst[i] = '\0';

  return dst;
}

char *strcat(char *dst, const char *src) {
  char* ptr = dst;
  while(*dst != '\0')  dst++;
  while(*src != '\0'){
    *dst++ = *src++;
  }
  *dst = '\0';

  return ptr;
}

char *strncat(char *dst, const char *src, size_t n) {
  char* ptr = dst;
  while(*dst != '\0') dst++;
  size_t i;
  for(i = 0; i < n && src[i] != '\0'; i++)
    dst[i] = src[i];
  dst[i] = '\0';

  return ptr;
}

int strcmp(const char *s1, const char *s2) {
  while(*s1 == *s2 ){
    if(*s1 == '\0') return 0;
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i;
  for(i = 0; i < n && s1[i] == s2[i]; i++){
    if(s1[i] == '\0') return 0;
  }
  if(i == n) return 0;
  return s1[i] - s2[i];
}

void *memset(void *s, int c, size_t n) {
  char* ptr = (char*) s;
  while(n--) *ptr++ = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  char* dstptr = (char*) dst;
  char* srcptr = (char*) src;
  char str[n];
  for(size_t i = 0; i < n; i++) str[i] = srcptr[i];
  for(size_t i = 0; i < n; i++) dstptr[i] = str[i];
  return dst;
}

void *memcpy(void *dst, const void *src, size_t n) {
  char* dstptr = (char*) dst;
  char* srcptr = (char*) src;
  while(n--) *dstptr++ = *srcptr++;
  return dst;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  char* s1ptr = (char*) s1;
  char* s2ptr = (char*) s2;
  size_t i;
  for(i = 0; i < n && s1ptr[i] == s2ptr[i]; i++){
    if(s1ptr[i] == '\0') return 0;
  }
  if(i == n) return 0;
  return s1ptr[i] - s2ptr[i];
}

#endif
