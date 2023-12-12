#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

static int gpu_width = 0;
static int gpu_height = 0;

void __am_gpu_init() {
  gpu_width = io_read(AM_GPU_CONFIG).width;
  gpu_height = io_read(AM_GPU_CONFIG).height;
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t result = inl(VGACTL_ADDR);
  int width = result >> 16;
  int height = result & 0xFFFF;
  int vmemsz = width * height * sizeof(uint32_t);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = vmemsz
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y;
  void *pixels = ctl->pixels;
  int w = ctl->w, h = ctl->h;
  for(int i = 0; i < w; i++) {
    for(int j = 0; j < h; j++) {
      uint32_t *pixel = (uint32_t *)(uintptr_t)pixels + i + j * w;
      uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR + (i + x) + (j + y) * gpu_width;
      outl((uintptr_t)fb, *pixel);
    }
  }
  bool sync = ctl->sync;
  if (sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
