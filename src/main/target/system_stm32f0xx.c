#include "stm32f0xx.h"

#if !defined (HSE_VALUE)
  #define HSE_VALUE ((uint32_t)8000000) // External oscillator in Hz.
#endif

#if !defined (HSI_VALUE)
  #define HSI_VALUE ((uint32_t)8000000)  // Internal oscillator in Hz.
#endif

#if !defined (HSI48_VALUE)
  #define HSI48_VALUE ((uint32_t)48000000) // HSI48 Internal oscillator in Hz.
#endif

uint32_t SystemCoreClock = 8000000;
const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};


void SystemInit(void) {
  // noop
}

void SystemCoreClockUpdate (void) {
  uint32_t tmp = 0, pllmull = 0, pllsource = 0, predivfactor = 0;

  tmp = RCC->CFGR & RCC_CFGR_SWS;
  switch (tmp) {
    case RCC_CFGR_SWS_HSI:
      SystemCoreClock = HSI_VALUE;
      break;
    case RCC_CFGR_SWS_HSE:
      SystemCoreClock = HSE_VALUE;
      break;
    case RCC_CFGR_SWS_PLL:
      pllmull = RCC->CFGR & RCC_CFGR_PLLMUL;
      pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;
      pllmull = ( pllmull >> 18) + 2;
      predivfactor = (RCC->CFGR2 & RCC_CFGR2_PREDIV) + 1;
      if (pllsource == RCC_CFGR_PLLSRC_HSE_PREDIV) {
        SystemCoreClock = (HSE_VALUE/predivfactor) * pllmull;
      }
      #if defined(STM32F042x6) || defined(STM32F048xx) || defined(STM32F072xB) || defined(STM32F078xx) || defined(STM32F091xC) || defined(STM32F098xx)
        else if (pllsource == RCC_CFGR_PLLSRC_HSI48_PREDIV) {
          SystemCoreClock = (HSI48_VALUE/predivfactor) * pllmull;
        }
      #endif
      else {
        #if defined(STM32F042x6) || defined(STM32F048xx)  || defined(STM32F070x6) \
         || defined(STM32F078xx) || defined(STM32F071xB)  || defined(STM32F072xB) \
         || defined(STM32F070xB) || defined(STM32F091xC) || defined(STM32F098xx)  || defined(STM32F030xC)

          SystemCoreClock = (HSI_VALUE/predivfactor) * pllmull;
        #else
          SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
        #endif
      }
      break;
    default:
      SystemCoreClock = HSI_VALUE;
      break;
  }
  tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
  SystemCoreClock >>= tmp;
}
