// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
#include <proc.h>
#include <elf.h>
#include<fs.h>
// #include<stdio.h>
#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

// 程序头：
//   Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
//   RISCV_ATTRIBUT 0x0058cb 0x00000000 0x00000000 0x0002e 0x00000 R   0x1
//   LOAD           0x000000 0x83000000 0x83000000 0x04e00 0x04e00 R E 0x1000
//   LOAD           0x005000 0x83005000 0x83005000 0x008a0 0x008e0 RW  0x1000
//   GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RW  0x10

extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_read(int fd, void *buf, size_t len);
extern size_t fs_write(int fd, const void *buf, size_t len);
extern size_t fs_lseek(int fd, size_t offset, int whence);
extern int fs_close(int fd);
// static void read(int fd, void *buf, size_t offset, size_t len){
//   fs_lseek(fd, offset, SEEK_SET);
//   fs_read(fd, buf, len);
// }

// static const int sz=500000;

// static uintptr_t loader(PCB *pcb, const char *filename) {
//   // int fd=open("/home/cyh627/ics2022/nanos-lite/build/ramdisk.img",O_RDONLY);
//   // char buf[0xffff];
//   // read(fd,buf,0x5000+0x008e0);
//   // ramdisk_write(buf,0x83005000,0x04e00);
//   // ramdisk_write(buf+0x005000,0x83005000,0x008a0);
//   // close(fd);
//   Elf_Ehdr elf_header;
//   read(fd, &elf_header, 0, sizeof(elf_header));
//   //根据小端法 0x7F E L F
//   assert(*(uint32_t *)elf_header.e_ident == 0x464c457f);

//   char buf[sz];
//   int fd=fs_open("/bin/dummy",0,0);
//   fs_read(fd,(void*)(buf),sz);
//   // ramdisk_write();
//   return (uintptr_t)(buf);
// }

// #define PG_MASK (~0xfff)
// #define ISALIGN(vaddr) ((vaddr) == ((vaddr)&PG_MASK))
// #define OFFSET(vaddr) ((vaddr) & (~PG_MASK))
// #define NEXT_PAGE(vaddr) ((ISALIGN(vaddr)) ? (vaddr) : ((vaddr)&PG_MASK) + PGSIZE)



static uintptr_t loader(PCB *pcb, const char *filename)
{
  // Elf_Ehdr ehdr;
  // ramdisk_read(&ehdr, 0, sizeof(ehdr));
  // // 7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00
  // char riscv32_magic_num[] = {0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  // printf("magic number is %d\n", *(int*)(ehdr.e_ident));
  // assert(strcmp((char *)(ehdr.e_ident), riscv32_magic_num) == 0);
  // uint32_t entry = ehdr.e_entry;
  // uint32_t ph_offset = ehdr.e_phoff;
  // uint32_t ph_num = ehdr.e_phnum;
  // printf("entry:%d\toffset:%d\tnum:%d\n",entry,ph_offset,ph_num);
  // Elf_Phdr phdr;
  // for (int i = 0; i < ph_num; ++i)
  // {
  //   ramdisk_read(&phdr, ph_offset + i * sizeof(phdr), sizeof(phdr));
  //   if(phdr.p_type!=PT_LOAD) continue;
  //   // printf("type:%d\n",phdr.p_type);
  //   uint32_t offset = phdr.p_offset;
  //   uint32_t file_size = phdr.p_filesz;
  //   uint32_t p_vaddr = phdr.p_vaddr;
  //   uint32_t mem_size = phdr.p_memsz;
  //   printf("poffset:%d\tfilesize:%d\tp_vaddr:%d\tmemsize:%d\n",offset,file_size,p_vaddr,mem_size);
  //   printf("load program from [%p, %p] to [%p, %p]\n", offset, file_size, p_vaddr, mem_size);
  //   ramdisk_read((void *)p_vaddr, offset, file_size);
  //   memset((void *)(p_vaddr + file_size), 0, mem_size - file_size);
  // }
  // printf("Loader over!!!\n");
  // return entry;

  printf("loader begin\n");
  Elf_Ehdr ehdr;
  int fd = fs_open(filename, 0, 0);
  assert(fd != -1);

  fs_read(fd, &ehdr, sizeof(ehdr));
  // 7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00
  char riscv32_magic_num[] = {0x7f, 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  printf("magic number is %d\n", *(int*)(ehdr.e_ident));
  assert(strcmp((char *)(ehdr.e_ident), riscv32_magic_num) == 0);

  uint32_t entry = ehdr.e_entry;
  uint32_t ph_offset = ehdr.e_phoff;
  uint32_t ph_num = ehdr.e_phnum;

  printf("entry:%d\toffset:%d\tnum:%d\n",entry,ph_offset,ph_num);

  // test for ftrace
  // printf("section offset:%d\tsection size:%d\tsection num:%d\n",ehdr.e_shoff,ehdr.e_shentsize,ehdr.e_shnum);
  // Elf64_Shdr sym;
  // fs_lseek(fd,ehdr.e_shoff-sizeof(sym)*3,SEEK_SET);
  // fs_read(fd,&sym,sizeof(sym));
  // printf("symbol offset:%d\tsymbol name:%d\tsymbol size:\n",sym.sh_offset,sym.sh_name,sym.sh_size);



  Elf_Phdr phdr;
  for (int i = 0; i < ph_num; ++i)
  {
    // if(phdr.p_type!=PT_LOAD) continue;
    // switch(phdr.p_type
    // ramdisk_read(&phdr, ph_offset + i * sizeof(phdr), sizeof(phdr));
    // printf("type:%d\n",phdr.p_type);
    // if(phdr.p_type!=PT_LOAD) continue;
    fs_lseek(fd, ph_offset + i * sizeof(phdr), SEEK_SET);
    fs_read(fd, &phdr, sizeof(phdr));
    if (phdr.p_type != PT_LOAD)
      continue;

    // printf("load program header %d", i);

    uint32_t offset = phdr.p_offset;
    uint32_t file_size = phdr.p_filesz;
    uint32_t p_vaddr = phdr.p_vaddr;
    uint32_t mem_size = phdr.p_memsz;

    // printf("poffset:%d\tfilesize:%d\tp_vaddr:%d\tmemsize:%d\n",offset,file_size,p_vaddr,mem_size);
    // printf("load program from [%p, %p] to [%p, %p]\n", offset, file_size, p_vaddr, mem_size);
    // ramdisk_read((void *)p_vaddr, offset, file_size);
    fs_lseek(fd, offset, SEEK_SET);
    fs_read(fd, (void*)(intptr_t)(p_vaddr), file_size);
    memset((void*)(intptr_t)(p_vaddr + file_size), 0, mem_size - file_size);
    // assert(mem_size >= file_size);
  }

  // printf("max brk is at %p when load\n", pcb->max_brk);
  // assert(fs_close(fd) != -1);
  printf("Loader over!!!\n");
  return entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", (void*)entry);
  ((void(*)())entry) ();
}


void context_kload(PCB* pcb,void (*entry)(void *),void* arg){
  // Area karea;
  // karea.start = &pcb->cp;
  // karea.end = &pcb->cp + STACK_SIZE;
  Area karea=RANGE(&pcb->cp,&pcb->cp+STACK_SIZE);
  pcb->cp = kcontext(karea, entry, arg);
  // printf("kcontext地址为:%p\n", pcb->cp);
}

// static int __len(char *const str[])
// {
//   int i = 0;
//   // printf("str1 is NULL? %d\n", str[1] == NULL);
//   for (; str[i] != NULL; ++i)
//   {
//     // printf("str is %d\n", i);
//   }
//   return i;
// }

#define NR_PAGE 8
#define PAGESIZE 4096
// extern Context *ucontext(AddrSpace *as, Area kstack, void *entry);
void context_uload(PCB* pcb,const char* filename,char *const argv[], char *const envp[]){
  Area kstack = RANGE(pcb, (char *)pcb + STACK_SIZE);
  // // TODO 用户栈就是用来存储参数的,pcb中一部分是context，一部分是栈的状态，保存上下文状态就是
  // // 把栈的现场，也就是指针，CSR寄存器以及分配的栈保存下来
  // // 现在要分配的是新的栈空间，需要把
  // // void *alloced_page = new_page(NR_PAGE) + NR_PAGE * 4096; //得到栈顶
// stack end 实际上是栈顶？
// 疑问：为什么在schedule中不需要复制context的内容，而只是指针的拷贝，但是创建的pcb中的context
// 指针是指向 stack end - sizeof（context)的


    // PCB* tmppcb=pcb;
  int argc=0,envc=0;
  if(argv!=NULL)
    while(argv[argc]!=NULL){
      argc++;
      // tmppcb++;
      // (char*)(tmppcb)=argv[argvc];
    }
  // tmppcb++;
  // (char*)(tmppcb)=NULL;
  if(envp!=NULL)
    while(envp[envc]!=NULL){
      envc++;
      // tmppcb++;
      // (char*)(tmppcb)=envp[envpc];
    }

  printf("argc:%d\tenvc:%d\n",argc,envc);
  char (*args[argc]);
  //  - 4 is important
  char *sp = (char*)(new_page(NR_PAGE)) + NR_PAGE * PAGESIZE - sizeof(Context);

  char* stack=sp+sizeof(Context);
  // char* sp=(char*)(pcb)+NR_PAGE * PAGESIZE -sizeof(Context);
  // char* sp=(char*)(pcb)+NR_PAGE * PAGESIZE;
  for (int i = 0; i < argc; ++i)
  {
    printf("argv[%d]:%s\n",i,argv[i]);
    sp -= (strlen(argv[i]) + 1);
    strcpy(sp, argv[i]);
    args[i] = sp;
  }


  char *(envs[envc]);
  for (int i = 0; i < envc; ++i)
  {
    printf("envp[%d]:%s\n",i,envp[i]);
    sp -= (strlen(envp[i]) + 1);
    strcpy(sp, envp[i]);
    envs[i] = sp;
  }

  // printf("over 1!!!\n");
  char **sptmp = (char **)sp;
  --sptmp;
  *sptmp=NULL;
  // *sptmp = "\0";
  // sp-=4;
  // (char**)(sp)=NULL;
  printf("over 2!!!\n");

  for (int i = envc - 1; i >= 0; --i)
  {
    printf("envs[%d]:%s\n",i,envs[i]);
    --sptmp;
    // sp-=4;
    *sptmp = envs[i];
    // *((char**)(sp))=envs[i];
    // printf("over 1!!!\n");
  }

  --sptmp;
  *sptmp = NULL;

  for (int i = argc - 1; i >= 0; --i)
  {
    --sptmp;
    // printf("the pos of argv is at stack %p is %p\n", sp_2, args[i]);
    *sptmp = args[i];
    printf("args is %s\n", args[i]);
  }

  // printf("some over!!!\n");
  --sptmp;
  *((int *)sptmp) = argc;
  printf("uload over!!\n");
  uintptr_t entry = loader(pcb, filename);
  // Context *context = ucontext(NULL, kstack, (void *)entry);
  pcb->cp=ucontext(NULL, kstack, (void *)entry);
  // TODO so important to understand the following code!!!
  pcb->cp->GPRx=(uintptr_t)(pcb->cp);
  pcb->cp->gpr[11]=(uintptr_t)(sptmp);
  pcb->cp->gpr[2]=(uintptr_t)(pcb->cp);
  // pcb->cp->gpr[0]=(uintptr_t)(sptmp);
  // pcb->cp->gpr[2]=(uintptr_t)(sptmp);
  // pcb->cp->gpr[10]=(uintptr_t)(sptmp);

  printf("the argc is at %p\n", sptmp);
  printf("the false argc is at %p\n",(char*)(pcb)+NR_PAGE * PAGESIZE);
  // printf("the argc is:%d\n",*(int*)(pcb->cp->GPRx));
  // printf("arg0 is %s\n", *((char **)(pcb->cp->GPRx + 1)));
  printf("the stack end is %p\n",stack);
  // printf("the pcb end is %p\n",pcb + );
  // printf("the argc is at %p\n", sp_2);
  // printf("arg1 is %s\n", *((char **)(sptmp + 1)));
  // printf("arg2 is %s\n", *((char **)(sptmp + 2)));
  // printf("arg3 is %s\n", *((char **)(sptmp + 3)));
  // printf("arg1 is %s\n", *((char **)(sptmp + 1)));

}

