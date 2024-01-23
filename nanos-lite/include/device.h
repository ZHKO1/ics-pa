#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <sys/time.h>

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);

size_t sb_write(const void *buf, size_t offset, size_t len);
size_t sbctl_read(void *buf, size_t offset, size_t len);
size_t sbctl_write(const void *buf, size_t offset, size_t len);

int device_gettimeofday(struct timeval *tv, struct timezone *tz);
int device_screen_w();
int device_screen_h();

#endif
