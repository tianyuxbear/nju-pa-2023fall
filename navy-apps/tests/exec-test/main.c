#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[], char* envp[]) {
  printf("main ==> argc: %d    argv: 0x%016x    envp: 0x%016x\n", argc, (uint64_t)argv, (uint64_t)envp);
  for(int i = 0; i < argc; i++){
    printf("%d arg ==> %s\n", i + 1, (char*)((uint64_t)argv + i * 32));
  }
  //int n = (argc >= 2 ? atoi(argv[1]) : 1);
  //printf("%s: argv[1] = %d\n", argv[0], n);
  printf("================================\n");
  int n = 1;
  char buf[16];
  sprintf(buf, "%d", n + 1);
  printf("argv[0]: %p    buf: %p\n", (uint64_t)argv[0], (uint64_t)buf);
  execl(argv[0], argv[0], buf, NULL);
  return 0;
}
