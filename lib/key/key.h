#ifndef __KEY_H
#define __KEY_H
#define NUM_BUTTONS 1 // 按钮的数量
#define DEBOUNCE_DELAY 60  // 消除抖动的延迟时间，单位：毫秒
#define PRESS_DURATION 60  // 按钮按下的最小持续时间，单位：毫秒
// 按钮状态
enum ButtonState {
  IDLE,
  PRESSED,
  RELEASED
};

void button_init();
int updateButtonState();
#endif