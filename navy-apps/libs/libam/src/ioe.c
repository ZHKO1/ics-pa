#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <NDL.h>
#include <string.h>
#include <unistd.h>

#define keyname(k) #k,

static const char *AM_KEY_NAMES[] = {
  "NONE",
  AM_KEYS(keyname)
};

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  char buf[64];
  char event[4];
  char name[10];
  int result = NDL_PollEvent(buf, sizeof(buf));
  if (result) {
    sscanf(buf, "%s %s", event, name);
    if(!strcmp(event, "kd")) {
      kbd->keydown = true;
    }
    if(!strcmp(event, "ku")) {
      kbd->keydown = false;
    }
    uint8_t keyname_len = sizeof((AM_KEY_NAMES)) / sizeof(*(AM_KEY_NAMES));
    for(uint8_t i = 0; i < keyname_len; i++){
      const char *str = AM_KEY_NAMES[i];
      if(!strcmp(str, name)){
        kbd->keycode = i;
      }
    }
  } else {
    kbd->keydown = false;
    kbd->keycode = AM_KEY_NONE;
  }
};
void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
};
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uint32_t ticks = NDL_GetTicks();
  uptime->us = ticks * 1000;
};
void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  int width = 0;
  int height = 0;
  NDL_OpenCanvas(&width, &height);
  int vmemsz = width * height * sizeof(uint32_t);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = vmemsz
  };
};
void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
};
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y;
  void *pixels = ctl->pixels;
  int w = ctl->w, h = ctl->h;
  NDL_DrawRect(pixels, x, y, w, h);
};
void __am_audio_config(AM_AUDIO_CONFIG_T *) {panic("not support");};
void __am_audio_ctrl(AM_AUDIO_CTRL_T *) {panic("not support");};
void __am_audio_status(AM_AUDIO_STATUS_T *) {panic("not support");};
void __am_audio_play(AM_AUDIO_PLAY_T *) {panic("not support");};
void __am_disk_config(AM_DISK_CONFIG_T *cfg) {panic("not support");};
void __am_disk_status(AM_DISK_STATUS_T *stat) {panic("not support");};
void __am_disk_blkio(AM_DISK_BLKIO_T *io) {panic("not support");};

static void __am_timer_config(AM_TIMER_CONFIG_T *cfg) { cfg->present = true; cfg->has_rtc = true; }
static void __am_input_config(AM_INPUT_CONFIG_T *cfg) { cfg->present = true;  }
static void __am_uart_config(AM_UART_CONFIG_T *cfg)   { cfg->present = false; }
static void __am_net_config (AM_NET_CONFIG_T *cfg)    { cfg->present = false; }

typedef void (*handler_t)(void *buf);
static void *lut[128] = {
  [AM_TIMER_CONFIG] = __am_timer_config,
  [AM_TIMER_RTC   ] = __am_timer_rtc,
  [AM_TIMER_UPTIME] = __am_timer_uptime,
  [AM_INPUT_CONFIG] = __am_input_config,
  [AM_INPUT_KEYBRD] = __am_input_keybrd,
  [AM_GPU_CONFIG  ] = __am_gpu_config,
  [AM_GPU_FBDRAW  ] = __am_gpu_fbdraw,
  [AM_GPU_STATUS  ] = __am_gpu_status,
  [AM_UART_CONFIG ] = __am_uart_config,
  [AM_AUDIO_CONFIG] = __am_audio_config,
  [AM_AUDIO_CTRL  ] = __am_audio_ctrl,
  [AM_AUDIO_STATUS] = __am_audio_status,
  [AM_AUDIO_PLAY  ] = __am_audio_play,
  [AM_DISK_CONFIG ] = __am_disk_config,
  [AM_DISK_STATUS ] = __am_disk_status,
  [AM_DISK_BLKIO  ] = __am_disk_blkio,
  [AM_NET_CONFIG  ] = __am_net_config,
};

static void fail(void *buf) { panic("access nonexist register"); }

#define PMEM_SIZE (128 * 1024 * 1024)


bool ioe_init() {
  for (int i = 0; i < LENGTH(lut); i++)
    if (!lut[i]) lut[i] = fail;
  NDL_Init(0);

  heap.start = sbrk(0);
  heap.end = heap.start + PMEM_SIZE; // TODO 这里感觉heap.end的值不够严谨，但也没什么思路，只能先这样处理
  return true;
}

void ioe_read (int reg, void *buf) { ((handler_t)lut[reg])(buf); }
void ioe_write(int reg, void *buf) { ((handler_t)lut[reg])(buf); }
