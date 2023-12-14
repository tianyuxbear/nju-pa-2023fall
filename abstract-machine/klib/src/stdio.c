#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static bool has_padding = false;

static char int_str[30];
int handle_dec(int num);
int handle_hex(int num);

static int prefix_num = 0;
static char prefix_char = ' ';
static char prefix_num_str[10];
int handle_prefix(const char* format);

int    printf    (const char *format, ...);
int    sprintf   (char *str, const char *format, ...);
int    snprintf  (char *str, size_t size, const char *format, ...);
int    vsprintf  (char *str, const char *format, va_list ap);
int    vsnprintf (char *str, size_t size, const char *format, va_list ap);

int printf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  size_t size = (1ul << 31) - 1;
  int nbyte = vsnprintf(NULL, size, format, ap);
  va_end(ap);
  return nbyte;
}

int sprintf(char *str, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  size_t size = (1ul << 31) - 1;
  int nbyte = vsnprintf(str, size, format, ap);
  va_end(ap);
  return nbyte;
}

int snprintf(char *str, size_t size, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int nbyte = vsnprintf(str, size, format, ap);
  va_end(ap);
  return nbyte;
}

int vsprintf(char *str, const char *format, va_list ap) {
  size_t size = (1ul << 31) - 1;
  int nbyte = vsnprintf(str, size, format, ap);
  return nbyte;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
  int nbyte = 0, bytes = 0;
  size_t maxbytes = size - 1;
  
  int d, x;
  char* s;

  while(*format != '\0'){
    if(nbyte == maxbytes) break;
    if(*format == '%' || has_padding){
      format++;
      switch(*format)
      {
        case 's': 
          s = va_arg(ap, char*);
          bytes = strlen(s);
          if(str != NULL){
            bytes = nbyte + bytes > maxbytes ? maxbytes - nbyte : bytes;
            strncpy(str, s, bytes);
            str += bytes;
            nbyte += bytes;
          }else{
            for(int i = 0; i < bytes; i++, s++)
              putch(*s);
            nbyte += bytes;
          }
          format++;
          break;
        case 'd':
          d = va_arg(ap, int);
          bytes = handle_dec(d);
          if(str != NULL){
            bytes = nbyte + bytes > maxbytes ? maxbytes - nbyte : bytes;
            strncpy(str, int_str, bytes);
            str += bytes;
            nbyte += bytes;
          }else{
            int padding = prefix_num - bytes;
            if(padding > 0){
              for(int i = 0; i < padding; i++) putch(prefix_char);
              nbyte += padding;

              has_padding = false;
              prefix_num = 0;
              prefix_char = ' ';
            }
            for(int i = 0; i < bytes; i++)
              putch(int_str[i]);
            nbyte += bytes;
          }
          format++;
          break;
        case 'x':
          x = va_arg(ap, int);
          bytes = handle_hex(x);
          if(str != NULL){
            bytes = nbyte + bytes > maxbytes ? maxbytes - nbyte : bytes;
            strncpy(str, int_str, bytes);
            str += bytes;
            nbyte += bytes;
          }else{
            int padding = prefix_num - bytes;
            if(padding > 0){
              for(int i = 0; i < padding; i++) putch(prefix_char);
              nbyte += padding;

              has_padding = false;
              prefix_num = 0;
              prefix_char = ' ';
            }
            for(int i = 0; i < bytes; i++)
              putch(int_str[i]);
            nbyte += bytes;
          }
          format++;
          break;
        case '0':
          prefix_char = '0';
          int offset = handle_prefix(format);
          format += offset - 1;
          break;
        default:
          printf("unimplemented char:");
          putch(*format);
          assert(0);
          break;
      }
    }else{
      if(str != NULL){
        *str++ = *format++;
      }else{
        putch(*format++);
      }
      nbyte++;
    }
  }

  if(str != NULL) *str = '\0';

  return nbyte;
}


int handle_dec(int num){
  memset(int_str, 0, sizeof(int_str));
  int index = 0;

  int is_negative = 0;
  if(num < 0){
    is_negative = 1;
    num = -num;
  }

  while(num != 0){
    char byte = num % 10;
    int_str[index++] = '0' + byte;
    num /= 10;
  }
  if(index == 0) int_str[index++] = '0';
  if(is_negative) int_str[index++] = '-';

  for(int i = 0, j = index - 1; i < j; i++, j--){
    char tmp = int_str[i];
    int_str[i] = int_str[j];
    int_str[j] = tmp;
  }
  int_str[index] = '\0';

  int bytes = strlen(int_str);

  return bytes;
}

int handle_hex(int num){
  memset(int_str, 0, sizeof(int_str));
  int index = 0;

  uint32_t hex = (uint32_t)num;
  while(hex != 0){
    char byte = hex % 16;
    int_str[index++] = (byte < 10) ? byte + '0' : byte - 10 + 'a';
    hex /= 16;
  }
  if(index == 0) int_str[index++] = '0';

  for(int i = 0, j = index - 1; i < j; i++, j--){
    char tmp = int_str[i];
    int_str[i] = int_str[j];
    int_str[j] = tmp;
  }
  int_str[index] = '\0';
  int bytes = strlen(int_str);

  return bytes;
}

int handle_prefix(const char* format){
  memset(prefix_num_str, 0, sizeof(prefix_num_str));
  int index = 0;

  while(*format >= '0' && *format <= '9'){
    prefix_num_str[index++] = *format++;
  }

  for(int i = 0; i < index; i++){
    prefix_num += prefix_num * 10 + prefix_num_str[i] - '0';
  }

  has_padding = true;

  return index;
}

#endif
