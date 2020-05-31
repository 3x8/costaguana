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
    if (bootloaderReadyToJump()) {
      LED_ON(LED_RED);
      bootloaderJumpToApplication();
    }
  }
}
