#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <stdlib.h>

static void SDL_LoadWAV_read(void *buf, size_t offset, size_t len, FILE *file);

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
  // TODO 这里有个问题就是SDL_MixAudio是怎么判断format的？按理说8位和16位的混音方式应该有差异？
  uint32_t size = len / sizeof(int16_t);
  for (size_t i = 0; i < size; i++) {
    int16_t src_val = *((int16_t *)src + i);
    int16_t dst_val = *((int16_t *)dst + i);
    int32_t val = (int32_t)dst_val + (int32_t)src_val;
    int16_t result = 0;
    if(val > INT16_MAX) {
      result = INT16_MAX;
    } else if(val < INT16_MIN) {
      result = INT16_MIN;
    } else {
      result = (int16_t)val;
    }
    *((int16_t *)dst + i) = result * volume / SDL_MIX_MAXVOLUME;
  }
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
  // assert(0);
  FILE *wave_file = fopen(file, "r");
  assert(wave_file);

  uint32_t ChunkID = 0;
  SDL_LoadWAV_read(&ChunkID, 0, 4, wave_file);
  assert(ChunkID == 0x46464952);
  
  uint32_t Format = 0;
  SDL_LoadWAV_read(&Format, 8, 4, wave_file);
  assert(Format == 0x45564157);
  
  uint32_t Subchunk1ID = 0;
  SDL_LoadWAV_read(&Subchunk1ID, 12, 4, wave_file);
  assert(Subchunk1ID == 0x20746D66);

  uint32_t Subchunk1Size = 0;
  SDL_LoadWAV_read(&Subchunk1Size, 16, 4, wave_file);

  uint16_t AudioFormat = 0;
  SDL_LoadWAV_read(&AudioFormat, 20, 2, wave_file);
  assert(AudioFormat == 1); // 如果不是1，就是压缩的PCM，暂不支持

  uint16_t NumChannels = 0;
  SDL_LoadWAV_read(&NumChannels, 22, 2, wave_file);

  uint32_t SampleRate = 0;
  SDL_LoadWAV_read(&SampleRate, 24, 4, wave_file);

  uint32_t ByteRate = 0;
  SDL_LoadWAV_read(&ByteRate, 28, 4, wave_file);

  uint16_t BitsPerSample = 0;
  SDL_LoadWAV_read(&BitsPerSample, 34, 2, wave_file);
  assert(BitsPerSample == 16); // 目前应该是只支持AUDIO_S16SYS

  size_t Subchunk2ID_offset = 16 + 4 + Subchunk1Size;
  uint32_t Subchunk2ID = 0;
  SDL_LoadWAV_read(&Subchunk2ID, Subchunk2ID_offset, 4, wave_file);
  assert(Subchunk2ID == 0x61746164);

  size_t Subchunk2Size_offset = Subchunk2ID_offset + 4;
  uint32_t Subchunk2Size = 0;
  SDL_LoadWAV_read(&Subchunk2Size, Subchunk2Size_offset, 4, wave_file);

  if (audio_buf) {
    size_t data_offset = Subchunk2Size_offset + 4;
    uint8_t *buf = malloc(Subchunk2Size);
    assert(buf);
    SDL_LoadWAV_read(buf, data_offset, Subchunk2Size, wave_file);
    *audio_buf = buf;
  }
  if (audio_len) {
    *audio_len = Subchunk2Size;
  }
  fclose(wave_file);
  if (spec) {
    spec->freq = SampleRate;
    spec->format = AUDIO_S16SYS;
    spec->channels = NumChannels;
    spec->samples = Subchunk2Size / (NumChannels * BitsPerSample / 8);
    return spec;
  } else {
    return NULL;
  }
}

static void SDL_LoadWAV_read(void *buf, size_t offset, size_t len, FILE *file) {
  fseek(file, offset, SEEK_SET);
  int ret = fread(buf, len, 1, file);
  assert(ret == 1);
}

void SDL_FreeWAV(uint8_t *audio_buf) {
  free(audio_buf);
}

void SDL_LockAudio() {
  // assert(0);
}

void SDL_UnlockAudio() {
  // assert(0);
}
