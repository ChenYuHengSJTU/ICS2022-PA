#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include<stdio.h>
int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
    printf("arg is at %p\n", args);

  printf("call main\n");
  int argc=*(int *)(args);
  printf("argc call man:%d\n",argc);
  char** argv=(char**)(args)+1,**evnp=argv;
  for(;*evnp!=NULL;++evnp){}

  exit(main(argc, argv, evnp));
  assert(0);
}
