#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  unsigned disk_offset;
  ReadFn read;
  WriteFn write;
  unsigned open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
extern size_t serial_write(const void *buf, size_t offset, size_t len);
extern size_t events_read(void *buf, size_t offset, size_t len);
extern size_t dispinfo_read(void *buf, size_t offset, size_t len);
extern size_t fb_write(const void *buf, size_t offset, size_t len);

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  {"/dev/events", 0, 0, events_read, invalid_write},
  {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
  {"/dev/fb", 0, 0, invalid_read, fb_write},
// [FD_FB] = {"../build/ramdisk.img",0,0,fb_read,ramdisk_write},
#include "files.h"
};

// added by me
// int fs_open(const char *pathname, int flags, int mode){
//   int ftsz=sizeof(file_table)/sizeof(Finfo);
//   for(int i=0;i<ftsz;++i){
//     if(strcmp(pathname,file_table[i].name)==0){
//       file_table[i].open_offset=0;
//       return i;
//     }
//   }
//   // assert(0);
//   return -1;
// }

// size_t fs_read(int fd, void *buf, size_t len){
//   Finfo* finfo=&file_table[fd];
//   int rlen=0;
//   if(finfo->read!=NULL){
//     rlen=finfo->read(buf,finfo->open_offset,len);
//     finfo->open_offset+=rlen;
//   }
//   else{
//     rlen = finfo->open_offset + len <= finfo->size ? len : finfo->size - finfo->open_offset;
//     ramdisk_read(buf, finfo->open_offset+finfo->disk_offset, rlen);
//     finfo->open_offset += rlen;
//   }
//   return rlen;
// }

// size_t fs_write(int fd, const void *buf, size_t len){
//   Finfo* finfo=&file_table[fd];
//   int rlen=0;
//   if(finfo->write!=NULL){
//     rlen=finfo->write(buf,finfo->open_offset,len);
//     finfo->open_offset+=rlen;
//   }
//   else{
//     assert(finfo->open_offset + len -finfo->disk_offset <= finfo->size);
//     ramdisk_write(buf, finfo->disk_offset + finfo->open_offset, len);
//     rlen = len;
//     finfo->open_offset += len;
//   }
//   return rlen;
// }

// size_t fs_lseek(int fd, size_t offset, int whence){
//     Finfo *finfo = &file_table[fd];
//     printf("offset:%d\tsize:%d\n",offset,finfo->size);
//     switch(whence){
//       case SEEK_CUR:
//         assert(finfo->open_offset + offset <= finfo->size);
//         finfo->open_offset += offset;
//         break;
//       case SEEK_SET:
//         // printf("roffset:%d\toffset:%d\tsize:%d\n",offset,offset-finfo->disk_offset,finfo->size);
//         assert(offset <= finfo->size);
//         finfo->open_offset = offset;
//         break;
//       case SEEK_END:
//         assert(offset <= finfo->size);
//         finfo->open_offset = finfo->size + offset;
//         break;
//       default:
//         assert(0);
//   }
//   return finfo->open_offset;
// }

// int fs_close(int fd){
//   return 0;
// }


#define NR_FILE (sizeof(file_table) / sizeof(file_table[0]))
#define min(x, y) ((x < y) ? x : y)
int fs_open(const char *pathname, int flags, int mode)
{
  // printf("===============");
  for (size_t fd = 0; fd < NR_FILE; ++fd)
  {
    // printf("compare is %s with %s\n", file_table[fd].name, pathname);
    if (strcmp(file_table[fd].name, pathname) == 0)
    {
      file_table[fd].open_offset = 0;
      return fd;
    }
  }
  return -1;
}

size_t fs_write(int fd, const void *buf, size_t len)
{
  // printf("write fd is %d, size is %d, to write len is %d\n", fd, file_table[fd].size, len);
  if (fd == 0)
    panic("should never write to stdin");
  // assert(fd >= 0 && fd < NR_FILE);
  if (fd > 0 && fd < 5)
    return file_table[fd].write(buf, 0, len);
  if(fd == 5){
    return file_table[fd].write(buf, file_table[fd].open_offset, len);
  }
  if (fd > 5 && fd < NR_FILE)
  {
    len = min(len, file_table[fd].size - file_table[fd].open_offset);
    // if (len == 0)
    //   return 0;
    size_t count = ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
    // printf("count is %d, len is %d\n", count, len);
    file_table[fd].open_offset += count;
    // printf("open_offset is %d, size is %d, res is %d\n", file_table[fd].open_offset, file_table[fd].size, file_table[fd].open_offset < file_table[fd].size);
    // printf("open offset is less than size\n");
    assert((file_table[fd].open_offset <= file_table[fd].size));
    printf("count is %d\n", count);
    return count;
  }
  else
    return -1;
}

size_t fs_read(int fd, void *buf, size_t len)
{
  // printf("read fd is %d, size is %d, offset is %d, to read len is %d\n", fd, file_table[fd].size, file_table[fd].open_offset, len);
  assert(fd > 0 && fd < NR_FILE);
  if (fd > 0 && fd < 5)
    return file_table[fd].read(buf, 0, len);
  if (fd >= 5 && fd < NR_FILE)
  {
    len = min(len, file_table[fd].size - file_table[fd].open_offset);
    // if (len == 0)
    //   return 0;
    // printf("read some thing from %d with size %d\n", file_table[fd].disk_offset + file_table[fd].open_offset, len);
    size_t count = ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
    // printf("len is %d\n", count);
    // printf("read open_offset is %d, size is %d, res is %d\n", file_table[fd].open_offset, file_table[fd].size, file_table[fd].open_offset < file_table[fd].size);
    file_table[fd].open_offset += count;
    // printf("read open_offset is %d, size is %d, res is %d\n", file_table[fd].open_offset, file_table[fd].size, file_table[fd].open_offset < file_table[fd].size);
    assert((file_table[fd].open_offset <= file_table[fd].size));
    // printf("count is %d\n", count);
    return count;
  }
  else
    return -1;
}

size_t fs_lseek(int fd, size_t offset, int whence)
{
  // printf("change seek of fd %d\n", fd);
  assert(fd > 3 && fd < NR_FILE);
  switch (whence)
  {
  case SEEK_SET:
    file_table[fd].open_offset = offset;
    break;
  case SEEK_CUR:
    file_table[fd].open_offset += offset;
    break;
  case SEEK_END:
    file_table[fd].open_offset = file_table[fd].size + offset;
    break;
  default:
    return -1;
  }
  // printf("offset is %d\n", file_table[fd].open_offset);
  return file_table[fd].open_offset;
}

int fs_close(int fd)
{
  if (fd >= 3 && fd < NR_FILE)
  {
    file_table[fd].open_offset = 0;
    return 0;
  }
  else
    return -1;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
  AM_GPU_CONFIG_T gpu_config;
  ioe_read(AM_GPU_CONFIG,&gpu_config);
  int width = gpu_config.width, height = gpu_config.height;
  int fb_fd = fs_open("/dev/fb", 0, 0);
  file_table[fb_fd].size = width * height;
  printf("NR_FILE:%d\n",NR_FILE);
}