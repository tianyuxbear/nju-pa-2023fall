#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  size_t open_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);

extern size_t serial_write(const void *buf, size_t offset, size_t len);

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

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
  [FD_STDIN]  = {"stdin", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, invalid_write},
#include "files.h"
};

int fs_open(const char* pathname, int flags, int mode){
  int filenum = sizeof(file_table) / sizeof(Finfo);
  for(int i = 0; i < filenum; i++){
    if(strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  Log("fs_open error: unmatched filename ==> %s", pathname);
  assert(0);
}

size_t fs_read(int fd, void* buf, size_t len){
  if(file_table[fd].read != NULL){
     size_t ret = file_table[fd].read(buf, 0, len);
     return ret;
  }

  size_t size = file_table[fd].size;
  size_t disk_offset = file_table[fd].disk_offset;
  size_t open_offset = file_table[fd].open_offset;

  assert(open_offset <= size);

  len = len > size - open_offset ? size - open_offset : len;
  ramdisk_read(buf, disk_offset + open_offset, len);
  file_table[fd].open_offset = open_offset + len;
  return len;
}

size_t fs_write(int fd, const void* buf, size_t len){
  if(file_table[fd].write != NULL){
     size_t ret = file_table[fd].write(buf, 0, len);
     return ret;
  }

  size_t size = file_table[fd].size;
  size_t disk_offset = file_table[fd].disk_offset;
  size_t open_offset = file_table[fd].open_offset;

  assert(open_offset <= size);
  
  len = len > size - open_offset ? size - open_offset : len;
  ramdisk_write(buf, disk_offset + open_offset, len);
  file_table[fd].open_offset = open_offset + len;
  return len;
}

size_t fs_lseek(int fd, size_t offset, int whence){
  assert(fd >= 3);
  size_t size = file_table[fd].size;
  size_t open_offset = file_table[fd].open_offset;
  if(whence == SEEK_SET){
    assert(offset <= size);
    file_table[fd].open_offset = offset;
  }else if(whence == SEEK_CUR){
    assert(open_offset + offset <= size);
    file_table[fd].open_offset = open_offset + offset;
  }else if(whence == SEEK_END){
    assert(size + offset <= size);
    file_table[fd].open_offset = size + offset;
  }else{
    Log("fs_lseek error: undefined whence type ==> %d\n", whence);
    assert(0);
  }
  return file_table[fd].open_offset;
}

int fs_close(int fd){
  return 0;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_STDOUT].write = serial_write;
  file_table[FD_STDERR].write = serial_write;

  int filenum = sizeof(file_table) / sizeof(Finfo);
  for(int i = 3; i < filenum; i++){
    file_table[i].open_offset = 0;
    file_table[i].read = NULL;
    file_table[i].write = NULL;
  }
}
