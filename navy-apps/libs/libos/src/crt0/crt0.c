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

  char argv[argc][32] = {0};
  for(int i = 0; i < argc; i++){
    memcpy(argv[i], (void*)(str_area + i * 32), 32);
  }

  int envc = (str_area - addr) / 8 - 1 - 1 - argc;
  str_area += argc * 32;
  char envp[envc][32]  = {0};
  for(int i = 0; i < envc; i++){
    memcpy(envp[i], (void*)(str_area + i * 32), 32);
  }

  exit(main(argc, argv, envp));
  assert(0);
}
