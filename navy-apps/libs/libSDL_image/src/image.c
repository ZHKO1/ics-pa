#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  FILE *img = fopen(filename, "rb");
  fseek(img, 0, SEEK_END);
  long img_size = ftell(img);
  char *buf = malloc(img_size);
  fseek(img, 0, SEEK_SET);
  size_t read_bytes = fread(buf, 1, img_size, img);
  assert(read_bytes == img_size);
  SDL_Surface* result = STBIMG_LoadFromMemory(buf, img_size);
  fclose(img);
  return result;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
