#include "key.h"
#include <Arduino.h>
// 按钮引脚定义
const int buttonPins[NUM_BUTTONS] = {19};
ButtonState buttonStates[NUM_BUTTONS] = {IDLE};

void button_init()
{
    // 设置按钮引脚为输入模式
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

}

// 更新按钮状态的函数
int updateButtonState() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    int buttonPin = buttonPins[i];
    int buttonValue = digitalRead(buttonPin);
    
    // 根据按钮状态执行相应操作
    switch (buttonStates[i]) {
      case IDLE:
        if (buttonValue == LOW) {
          buttonStates[i] = PRESSED;
        }
        break;
      case PRESSED:
        if (buttonValue == HIGH) {
          delay(DEBOUNCE_DELAY);  // 等待一段时间，消除抖动
          int buttonValue1 = digitalRead(buttonPin);  // 再次读取按钮状态
          if (buttonValue1 == HIGH) {
            buttonStates[i] = RELEASED;
            return i; // 返回被按下的按钮的索引
          } else {
            buttonStates[i] = IDLE;
          }
        }
        break;
      case RELEASED:
        if (buttonValue == HIGH) {
          delay(DEBOUNCE_DELAY);  // 等待一段时间，消除抖动
          int buttonValue1 = digitalRead(buttonPin);  // 再次读取按钮状态
          if (buttonValue1 == HIGH) {
            buttonStates[i] = IDLE;
          }
        }
        break;
      default:
        break;
    }
  }
  return -1; // 没有按钮被按下
}
