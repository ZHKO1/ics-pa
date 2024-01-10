#include <NDL.h>
#include <SDL.h>
#include <assert.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

static void SDL_KeyboardEvent_init(SDL_Event *ev, char *event, char*name){
  if(!strcmp(event, "kd")) {
    ev->type = SDL_KEYDOWN;
    ev->key.type = SDL_KEYDOWN;
  }
  if(!strcmp(event, "ku")) {
    ev->type = SDL_KEYUP;
    ev->key.type = SDL_KEYUP;
  }
  uint8_t keyname_len = sizeof((keyname)) / sizeof(*(keyname));
  for(uint8_t i = 0; i < keyname_len; i++){
    char *str = keyname[i];
    if( !strcmp(str, name) ){
      ev->key.keysym.sym = i;
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
  assert(0);
  return NULL;
}
