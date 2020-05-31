#include "main.h"

extern uint16_t fourWayCmdInvalid;


int main(void) {
  // init
  systemClockConfig();
  systemGpioInit();
  systemTim2Init();
  ledInit();
  ledOff();

  while (true) {
    fourWayGetBuffer();
    if (fourWayCmdInvalid > 100) {
      LED_ON(LED_RED);
      bootloaderJumpToApplication();
    }
  }
}
