#include <common.h>
#include <sys/time.h>
#include <amdev.h>
#include <stdint.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

static int screen_w = 0, screen_h = 0;
static int sb_size = 0;

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (size_t i = 0; i < len; i++) putch(*((char *)buf + i));
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    return 0;
  };
  return snprintf(buf, len, "%s %s", ev.keydown ? "kd" : "ku", keyname[ev.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  int w = io_read(AM_GPU_CONFIG).width;
  int h = io_read(AM_GPU_CONFIG).height;
  return snprintf(buf, len, "WIDTH:%d\nHEIGHT:%d\n", w, h);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int y = offset / (sizeof(uint32_t) * screen_w);
  int x = (offset - (screen_w * y * sizeof(uint32_t))) / (sizeof(uint32_t));
  int w = len / (sizeof(uint32_t));
  io_write(AM_GPU_FBDRAW, x, y, (void  *)buf, w, 1, false);
  io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
  return len;
}

size_t sb_write(const void *buf, size_t offset, size_t len) {
  size_t write_len = (len <= sb_size) ? len : sb_size;
  Area sbuf;
  sbuf.start = (void *)buf;
  sbuf.end = (uint8_t *)buf + write_len;
  io_write(AM_AUDIO_PLAY, sbuf);
  return write_len;
}

size_t sbctl_read(const void *buf, size_t offset, size_t len) {
  // assert(len == 1 * sizeof(uint32_t));
  // assert(offset == 0);
  int count = io_read(AM_AUDIO_STATUS).count;
  int free = sb_size - count;
  return snprintf((char *)buf, len, "%d\n", free);
}

size_t sbctl_write(const void *buf, size_t offset, size_t len) {
  assert(len == 3 * sizeof(uint32_t));
  // assert(offset == 0);
  int freq = *((int *)buf + 0);
  int channels = *((int *)buf + 1);
  int samples = *((int *)buf + 2);
  io_write(AM_AUDIO_CTRL, freq, channels, samples);
  return len;
}

int device_gettimeofday(struct timeval *tv, struct timezone *tz) {
  AM_TIMER_UPTIME_T uptime = io_read(AM_TIMER_UPTIME);
  tv->tv_sec = uptime.us / 1000000;
  tv->tv_usec = uptime.us - tv->tv_sec * 1000000;
  return 0;
}

int device_screen_w(){
  return screen_w;
}

int device_screen_h(){
  return screen_h;  
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
  screen_w = io_read(AM_GPU_CONFIG).width;
  screen_h = io_read(AM_GPU_CONFIG).height;

  sb_size = io_read(AM_AUDIO_CONFIG).bufsize;
}
