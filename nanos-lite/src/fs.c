#include <klib-macros.h>
#include <fs.h>
#include <device.h>
#include <common.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

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
  {"/dev/fb", 0, 0, invalid_read, fb_write},
  {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

int fs_open(const char *pathname, int flags, int mode) {
  for (size_t i = 0; i < LENGTH(file_table); i++) {
    Finfo *file_ptr = &file_table[i];
    if (strcmp(file_ptr->name, pathname)  == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("Not find %s in file_table", pathname);
}

size_t fs_read(int fd, void *buf, size_t len){
  assert(fd >= 0 && fd < LENGTH(file_table));
  Finfo *file = &file_table[fd];
  if ( file->read ) {
    size_t offset = file->read(buf, file->open_offset, len);
    file->open_offset += offset;
    return offset;
  }
  if ( file->open_offset >= file->size ) {
    return 0;
  }
  int read_len = 0;
  if ( file->open_offset + len <= file->size ) {
    read_len = ramdisk_read(buf, file->disk_offset + file->open_offset, len);
    file->open_offset += read_len;
  } else {
    size_t ready_read_len = file->size - file->open_offset;
    read_len = ramdisk_read(buf, file->disk_offset + file->open_offset, ready_read_len);
    file->open_offset += read_len;
  }
  assert((file->open_offset >= 0) && (file->open_offset <= file->size));
  return read_len;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  assert(fd >= 0 && fd < LENGTH(file_table));
  Finfo *file = &file_table[fd];
  if ( file->write ) {
    size_t offset = file->write(buf, file->open_offset, len);
    file->open_offset += offset;
    return offset;
  }
  if ( file->open_offset >= file->size ) {
    return 0;
  }
  int write_len;
  if ( file->open_offset + len <= file->size ) {
    write_len = ramdisk_write(buf, file->disk_offset + file->open_offset, len);
    file->open_offset += write_len;
  } else {
    size_t ready_write_len = file->size - file->open_offset;
    write_len = ramdisk_write(buf, file->disk_offset + file->open_offset, ready_write_len);
    file->open_offset += write_len;
  }
  assert((file->open_offset >= 0) && (file->open_offset <= file->size));
  return write_len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  assert(fd >= 0 && fd < LENGTH(file_table));
  Finfo *file = &file_table[fd];
  switch (whence) {
    case SEEK_SET:
      file->open_offset = offset;
      break;
    case SEEK_CUR:
      file->open_offset += offset;
      break;
    case SEEK_END:
      file->open_offset = file->size + offset;
      break;  
    default:
      panic("Not supported whence(%d) in fs_lseek", fd);
      break;
  }
  assert((file->open_offset >= 0) && (file->open_offset <= file->size));
  return file->open_offset;
}

char *fs_get_name_by_fd(int fd) {
  assert(fd >= 0 && fd < LENGTH(file_table));
  Finfo *file = &file_table[fd];
  return file->name;
}

int fs_close(int fd){
  return 0;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
  int w = device_screen_w();
  int h = device_screen_h();
  int fd_fb = fs_open("/dev/fb", 0, 0);
  file_table[fd_fb].size = w * h * sizeof(uint32_t);
}
