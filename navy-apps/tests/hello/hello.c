#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]) {
  // for(int i = 0; i < argc; i++){
  //   printf("%s\n", argv[i]);
  // }
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
