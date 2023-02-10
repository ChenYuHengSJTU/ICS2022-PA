#include <am.h>
#include <nemu.h>
#include<stdio.h>
#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  // printf("called!!!\n");
  // uint32_t keycode = inl(KBD_ADDR);
  // kbd->keydown = keycode & KEYDOWN_MASK;
  // kbd->keycode = keycode & (~(KEYDOWN_MASK));
  uint32_t input = inl(KBD_ADDR);
  if (input != AM_KEY_NONE){
    kbd->keydown = true;
    kbd->keycode = input & (~KEYDOWN_MASK);
    // printf("code:%d",kbd->keycode);
  }else {
    kbd->keydown = false;
    kbd->keycode = AM_KEY_NONE;
  }
}
