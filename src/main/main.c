#include "main.h"

extern uint16_t fourWayCmdInvalid;


int main(void) {
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  // prefetch buffer enable
  FLASH->ACR |= FLASH_ACR_PRFTBE;

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
