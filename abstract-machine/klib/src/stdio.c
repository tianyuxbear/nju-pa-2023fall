#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static bool has_padding = false;

static char int_str[30];
int handle_int(int num);

static int prefix_num = 0;
static char prefix_char = '\0';
static char prefix_num_str[10];
void handle_prefix(const char* format);

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
  
  int d;
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
          bytes = handle_int(d);
          if(str != NULL){
            bytes = nbyte + bytes > maxbytes ? maxbytes - nbyte : bytes;
            strncpy(str, int_str, bytes);
            str += bytes;
            nbyte += bytes;
          }else{
            int padding = prefix_num - bytes;
            if(prefix_char != '\0' && padding > 0){
              for(int i = 0; i < padding; i++) putch(prefix_char);
              nbyte += padding;

              has_padding = false;
              prefix_num = 0;
              prefix_char = '\0';
            }
            for(int i = 0; i < bytes; i++)
              putch(int_str[i]);
            nbyte += bytes;
          }
          format++;
          break;
        case '0':
          prefix_char = '0';
          handle_prefix(format);
          format--;
          break;
        default:
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


int handle_int(int num){
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

void handle_prefix(const char* format){
  memset(prefix_num_str, 0, sizeof(prefix_num_str));
  int index = 0;

  format++;
  while(*format >= '0' && *format <= '9'){
    putch(*format);
    prefix_num_str[index++] = *format++;
  }

  printf("left while");
  for(int i = 0; i < index; i++){
    prefix_num += prefix_num * 10 + prefix_num_str[i] - '0';
  }
  printf("%d", prefix_num);
  has_padding = true;
}

#endif
