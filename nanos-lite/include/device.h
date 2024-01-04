#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <sys/time.h>

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);

int device_gettimeofday(struct timeval *tv, struct timezone *tz);

#endif
