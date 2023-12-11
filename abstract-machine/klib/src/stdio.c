#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)



static char num_str[25];
int handle_int(int num);

int printf(const char *format, ...) {
  va_list ap;
  int d;
  char* s;

  int nbyte = 0, bytes = 0;
  va_start(ap, format);
  while(*format != '\0'){
    if(*format == '%'){
      format++;
      switch(*format)
      {
        case 's': 
          s = va_arg(ap, char*);
          bytes = strlen(s);
          for(int i = 0; i < bytes; i++, s++)
            putch(*s);
          format++;
          nbyte += bytes;
          break;
        case 'd':
          d = va_arg(ap, int);
          bytes = handle_int(d);
          for(int i = 0; i < bytes; i++)
            putch(num_str[i]);
          format++;
          nbyte += bytes;
          break;
        default:
          assert(0);
          break;
      }
    }else{
      putch(*format);
      format++;
      nbyte++;
    }
  }

  va_end(ap);

  return nbyte;
}

int sprintf(char *str, const char *format, ...) {
  va_list ap;
  int d;
  char* s;

  int nbyte = 0, bytes = 0;
  va_start(ap, format);
  while(*format != '\0'){
    if(*format == '%'){
      format++;
      switch(*format)
      {
        case 's': 
          s = va_arg(ap, char*);
          bytes = strlen(s);
          strncpy(str, s, bytes);
          str += bytes;
          format++;
          nbyte += bytes;
          break;
        case 'd':
          d = va_arg(ap, int);
          bytes = handle_int(d);
          strncpy(str, num_str, bytes);
          str += bytes;
          format++;
          nbyte += bytes;
          break;
        default:
          assert(0);
          break;
      }
    }else{
      *str++ = *format++;
      nbyte++;
    }
  }

  va_end(ap);
  *str = '\0';

  return nbyte;
}

int snprintf(char *str, size_t size, const char *format, ...) {
  panic("Not implemented");
}

int vsprintf(char *str, const char *format, va_list ap) {
  panic("Not implemented");
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
  panic("Not implemented");
}


int handle_int(int num){
  memset(num_str, 0, sizeof(num_str));
  int index = 0;

  int is_negative = 0;
  if(num < 0){
    is_negative = 1;
    num = -num;
  }

  while(num != 0){
    char byte = num % 10;
    num_str[index++] = '0' + byte;
    num /= 10;
  }
  if(index == 0) num_str[index++] = '0';
  if(is_negative) num_str[index++] = '-';

  for(int i = 0, j = index - 1; i < j; i++, j--){
    char tmp = num_str[i];
    num_str[i] = num_str[j];
    num_str[j] = tmp;
  }
  num_str[index] = '\0';

  int bytes = strlen(num_str);

  return bytes;
}

#endif
