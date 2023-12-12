#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t am_scancode = inl(KBD_ADDR);
  bool keydown = (am_scancode & KEYDOWN_MASK) ? true : false;
  int keycode = am_scancode & 0xFF;
  kbd->keydown = keydown;
  kbd->keycode = keycode;
}
