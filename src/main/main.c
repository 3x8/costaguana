#include "main.h"

int main(void) {
  // init
  systemClockConfig();
  systemGpioInit();
  systemTim2Init();
  ledInit();
  ledOff();

  while (true) {
    fourWayGetBuffer();
    if (fourWayReadyToJump()) {
      LED_ON(LED_RED);
      bootloaderJumpToApplication();
    }
  }
}
