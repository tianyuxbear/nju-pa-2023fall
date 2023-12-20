#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  uint64_t addr = (uint64_t)args;
  int argc = *((int*)addr);
  addr += 8;
  uint64_t str_area = addr;
  int i = 0;
  while(i != 2){
    if(*((uint64_t*)str_area) == 0) i++;
    str_area += 8;
  }
  exit(main(argc, (char**)(str_area), (char**)(str_area + argc * 32)));
  assert(0);
}
