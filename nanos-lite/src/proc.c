#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
// 
PCB *current = NULL;
// added by me
// int all_pcb=-1;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

extern void context_kload(PCB* pcb,void (*entry)(void *),void* arg);
extern void context_uload(PCB* pcb,const char* filename,char *const argv[], char *const envp[]);
extern void naive_uload(PCB *pcb, const char *filename);
void init_proc() {
  switch_boot_pcb();

  Log("Initializing processes...");

  // loaded
  context_kload(&pcb[0], hello_fun, "hallo world");
  context_kload(&pcb[1], hello_fun, NULL);
  // char* argv[]={"hallo","world","sjtu"},*                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             envp[]={"in","proc.c"};
  // context_uload(&pcb[0], "/bin/exec-test",argv,envp);
  // context_uload(&pcb[1], "/bin/hello",NULL,NULL);

  switch_boot_pcb();
  assert(&pcb_boot!=NULL);
  
  // naive_uload(NULL,"/bin/exec-test");

  // added

  // load program here

}

Context* schedule(Context *prev) {
  current->cp=prev;
  // current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  current = &pcb[0];
  printf("schedule!!!\n");
  return current->cp;
}

extern int fs_open(const char *pathname, int flags, int mode);
extern int fs_close(int fd);

int execve(const char *pathname, char *const argv[], char *const envp[]){
  int fd = fs_open(pathname, 0, 0);
  if (fd == -1)
  {
    return -1;
  }
  else
    fs_close(fd);
  context_uload(current, (char *)pathname, argv, envp);
  // NR_PROC--;
  // assert(NR_PROC >= 0 && NR_PROC <= MAX_NR_PROC);
  switch_boot_pcb();
  yield();
  return 0;
}