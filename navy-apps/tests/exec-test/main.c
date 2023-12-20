#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[], char* envp[]) {
  printf("test argv[1]: %p\n", (uint64_t)argv + 32);
  printf("error argv[1]: %p\n", (uint64_t)argv[1]);
  int n = (argc >= 2 ? atoi(argv[1]) : 1);
  printf("%s: argv[1] = %d\n", argv[0], n);

  char buf[16];
  sprintf(buf, "%d", n + 1);
  execl((char*)((uint64_t)argv), (char*)((uint64_t)argv), buf, NULL);
  return 0;
}
