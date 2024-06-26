#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>

#define MAXLINE 1000

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

static int fd_events = -1;
static int fd_dispinfo = -1;
static int fd_fb = -1;

static int canvas_w = 0;
static int canvas_h = 0;

static int canvas_x = 0;
static int canvas_y = 0;

static int fd_sb = -1;
static int fd_sbctl = -1;

uint32_t NDL_GetTicks() {
  struct timeval tv = {};
  struct timezone tz = {};
  int result = gettimeofday(&tv, &tz);
  return tv.tv_sec * 1000 + tv.tv_usec/1000;
}

int NDL_PollEvent(char *buf, int len) {
    if(read(fd_events, buf, len)){
      return 1;
    } else {
      return 0;
    };
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  if((*w == 0) && (*h == 0)) {
    canvas_w = screen_w;
    canvas_h = screen_h;
    *w = canvas_w;
    *h = canvas_h;
  } else {
    canvas_w = *w;
    canvas_h = *h;
  }
  canvas_x = (screen_w - canvas_w) / 2;
  canvas_y = (screen_h - canvas_h) / 2;
  // printf("canvas_w = %d, canvas_h = %d\n", canvas_w, canvas_h);
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  assert(w >= 0);
  assert(h >= 0);
  assert(w <= canvas_w);
  assert(h <= canvas_h);
  for(int j = 0; j < h; j++) {
    uint32_t *pixel = (uint32_t *)(uintptr_t)pixels + 0 + j * w;
    long offset = ((0 + x + canvas_x) + (j + y + canvas_y) * screen_w) * sizeof(uint32_t);
    lseek(fd_fb, offset, SEEK_SET);
    write(fd_fb, pixel, (sizeof(uint32_t)) * (size_t) w);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
  int open_audio_config[] = {freq, channels, samples};
  write(fd_sbctl, open_audio_config, sizeof(open_audio_config));
}

void NDL_CloseAudio() {
  // TODO 这里是否只需要保持空函数就可以
}

int NDL_PlayAudio(void *buf, int len) {
  size_t write_count = write(fd_sb, buf, len);
  // TODO 文档对返回的描述是 “成功播放的音频数据的字节数”
  // 猜测这个返回值可能是为了应付len大于缓冲区大小的场景
  return write_count;
}

int NDL_QueryAudio() {
  char str[100] = "";
  read(fd_sbctl, &str, 100);
  int free = 0;
  sscanf(str, "%d", &free);
  return free;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  fd_events = open("/dev/events", 0, 0);
  fd_dispinfo = open("/proc/dispinfo", 0, 0);
  fd_fb = open("/dev/fb", 0, 0);
  char dispinfo_str[MAXLINE] = "";
  read(fd_dispinfo, dispinfo_str, MAXLINE);
  sscanf(dispinfo_str, "WIDTH:%d\nHEIGHT:%d", &screen_w, &screen_h);
  // printf("screen_w = %d, screen_h = %d\n", screen_w, screen_h);

  fd_sb = open("/dev/sb", 0, 0);
  fd_sbctl = open("/dev/sbctl", 0, 0);

  return 0;
}

void NDL_Quit() {
  close(fd_events);
  close(fd_dispinfo);
  close(fd_fb);
}
