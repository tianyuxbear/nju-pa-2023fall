#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]) {
  printf("main ==> argc: %d    argv: 0x%016x    envp: 0x%016x\n", argc, (uint64_t)argv, (uint64_t)envp);
  printf("1st arg: %s\n", (char*)argv);
  write(1, "Hello World!\n", 13);
  int i = 1;
  volatile int j = 0;
  while (1) {
    j ++;
    if (j == 10000) {
      printf("Hello World from Navy-apps for the %dth time!\n", i ++);
      j = 0;
    }
  }

  return 0;
}
