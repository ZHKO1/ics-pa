#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

#define keystatus_init(k) [SDLK_##k] = 0,

static uint8_t keystatus[] = {
  [SDLK_NONE] = 0,
  _KEYS(keystatus_init)
};

static void SDL_KeyboardEvent_init(SDL_Event *ev, char *event, char*name){
  uint8_t keyname_len = sizeof((keyname)) / sizeof(*(keyname));
  for(uint8_t i = 0; i < keyname_len; i++){
    const char *str = keyname[i];
    if( !strcmp(str, name) ){
      ev->key.keysym.sym = i;
      if(!strcmp(event, "kd")) {
        ev->type = SDL_KEYDOWN;
        ev->key.type = SDL_KEYDOWN;
        keystatus[i] = 1;
      }
      if(!strcmp(event, "ku")) {
        ev->type = SDL_KEYUP;
        ev->key.type = SDL_KEYUP;
        keystatus[i] = 0;
      }
    }
  }
}

int SDL_PushEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[64];
  char event[4];
  char name[10];
  int result = NDL_PollEvent(buf, sizeof(buf));
  if (result) {
    // printf("buf=%s\n", buf);
    sscanf(buf, "%s %s", event, name);
    if(!strcmp(event, "kd") || !strcmp(event, "ku")) {
      SDL_KeyboardEvent_init(ev, event, name);
    }
  }
  return result;
}

int SDL_WaitEvent(SDL_Event *event) {
  while(!SDL_PollEvent(event)){
  }
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  // assert(0);
  uint8_t keystatus_len = sizeof((keystatus)) / sizeof(*(keystatus));
  if (numkeys) {
    *numkeys = keystatus_len;
  }
  return keystatus;
}
