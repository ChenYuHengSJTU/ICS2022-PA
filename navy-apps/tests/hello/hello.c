#include <unistd.h>
#include <stdio.h>

int main(int argc,char* argv[],char* evnp[]) {
  printf("argc%d\n",argc);
  for(int i=0;i<argc;++i) printf("%s\n",argv[i]);
  write(1, "Hello World!\n", 13);
  int i = 2;
  volatile int j = 0;
  while (1) {
    j ++;
    if (j == 10000) {
      // putch('j');
      printf("Hello World from Navy-apps for the %dth time!\n", i ++);
      j = 0;
    }
  }
  return 0;
}
