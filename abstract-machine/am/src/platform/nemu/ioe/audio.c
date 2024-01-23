#include <am.h>
#include <nemu.h>
#include <klib.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)
#define AUDIO_SBUF_RIGHT_ADDR   (AUDIO_ADDR + 0x18)
#define AUDIO_SBUF_LEFT_ADDR    (AUDIO_ADDR + 0x1c)

static uint32_t bufsize = 0;

void __am_audio_init() {

}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
  cfg->bufsize = bufsize;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  int freg = ctrl->freq;
  int channels = ctrl->channels;
  int samples = ctrl->samples;
  outl(AUDIO_FREQ_ADDR, freg);
  outl(AUDIO_CHANNELS_ADDR, channels);
  outl(AUDIO_SAMPLES_ADDR, samples);
  outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  uint32_t rest_bufsize = 0;
  uint32_t ready_write_bufsize = (uintptr_t)(ctl->buf.end) - (uintptr_t)(ctl->buf.start);
  uint32_t right = 0;
  uint32_t left = 0;
  assert(ready_write_bufsize <= bufsize);
  do {
    right = inl(AUDIO_SBUF_RIGHT_ADDR);
    left = inl(AUDIO_SBUF_LEFT_ADDR);
    assert(right >= left);
    
    // 检查缓冲区剩下的空间是否放得下
    rest_bufsize = bufsize - (right - left);
    if (rest_bufsize < ready_write_bufsize) {
      printf("Not enough space in the sbuf, waiting for SDL library to read.[rest_bufsize = 0x%x ready_write_bufsize = 0x%x]\n", rest_bufsize, ready_write_bufsize);
    }
    // 不确定会不会死循环
  } while(rest_bufsize < ready_write_bufsize);

  for(uint32_t offset = 0; offset < ready_write_bufsize; offset++) {
    uint8_t sound_byte = *((uint8_t *)ctl->buf.start + offset);
    outb(AUDIO_SBUF_ADDR + (right + offset) % bufsize, sound_byte);
  }
  outl(AUDIO_SBUF_RIGHT_ADDR, right + ready_write_bufsize);
}
