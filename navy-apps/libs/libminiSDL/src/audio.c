#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <stdlib.h>

static SDL_AudioSpec config = {};
static bool audio_init = false;
static bool is_pause = false;
static bool is_callbackhelper_called = false;
static uint32_t pre_ms = 0;

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  assert(obtained == NULL);
  assert(desired->format == AUDIO_S16SYS);
  memcpy(&config, desired, sizeof(SDL_AudioSpec));
  NDL_OpenAudio(config.freq, config.channels, config.samples);
  audio_init = true;
  return 0;
}

void CallbackHelper() {
  if (!audio_init) {
    return;
  }
  if (is_callbackhelper_called) {
    // 可能用户在config.callback里又调用了api，导致再次调用CallbackHelper，所以这里检查当前的函数调用是否属于重入
    return;
  }
  is_callbackhelper_called = true;
  uint32_t ms = config.samples * 1000 / config.freq;
  uint32_t current_ms = NDL_GetTicks();
  if (current_ms - pre_ms >= ms) {
    // printf("MiniSDL CallbackHelper 离上一次CallbackHelper过了%dms\n", current_ms - pre_ms);
    uint32_t sample_size = 0;
    switch (config.format) {
      case AUDIO_S16SYS:
        sample_size = sizeof(int16_t);
        break;
      default:
        assert(0);
        break;
    }
    uint32_t samples_size = config.samples * config.channels * sample_size;
    int free_size = NDL_QueryAudio();
    while( free_size == 0 ) {
      // printf("MiniSDL CallbackHelper 目前缓冲区已满，继续等待...\n");
      free_size = NDL_QueryAudio();
    }
    uint32_t size = free_size < samples_size ? free_size : samples_size;
    uint8_t *audio_buf = malloc(size);
    if (!is_pause) {
      // printf("MiniSDL CallbackHelper 准备读取音乐数据，缓冲区还剩下%d字节\n", free_size);
      config.callback(NULL, audio_buf, size);
    } else {
      memset(audio_buf, 0, size);
    }
    NDL_PlayAudio(audio_buf, size);
    free(audio_buf);
    pre_ms = current_ms;
  }
  is_callbackhelper_called = false;
}

void SDL_CloseAudio() {
  memset(&config, 0, sizeof(SDL_AudioSpec));
  audio_init = false;
  // assert(0);
}

void SDL_PauseAudio(int pause_on) {
  // assert(0);
  is_pause = !!pause_on;
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
  // assert(0);
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
  // assert(0);
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
  // assert(0);
}

void SDL_LockAudio() {
  // assert(0);
}

void SDL_UnlockAudio() {
  // assert(0);
}
