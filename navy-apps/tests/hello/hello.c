#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]) {
  printf("int main argc: %d\n", argc);
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
