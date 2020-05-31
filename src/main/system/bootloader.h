#pragma once

#include "main.h"

#define FLASH_FKEY1 ((uint32_t)0x45670123)
#define FLASH_FKEY2 ((uint32_t)0xCDEF89AB)

#if defined(STM32F030x6) || defined(STM32F030x8) || defined(STM32F031x6) || defined(STM32F038xx) || defined(STM32F051x8) || defined(STM32F042x6) || defined(STM32F048xx) || defined(STM32F058xx) || defined(STM32F070x6)
  #define FLASH_PAGE_SIZE          0x400U
#endif
#if defined(STM32F071xB) || defined(STM32F072xB) || defined(STM32F078xx) || defined(STM32F070xB) || defined(STM32F091xC) || defined(STM32F098xx) || defined(STM32F030xC)
  #define FLASH_PAGE_SIZE          0x800U
#endif

#define FLASH_PAGE_COUNT        64
#define FLASH_CONFIG_SIZE       0x800
#define APPLICATION_ADDRESS     (uint32_t)0x08002000
#define EEPROM_START_ADDRESS    (0x08000000 + (uint32_t)((FLASH_PAGE_SIZE * FLASH_PAGE_COUNT) - FLASH_CONFIG_SIZE))


void bootloaderFlashWrite(uint8_t *data, int length, uint32_t address);
void bootloaderFlashRead(uint8_t*  data , uint32_t address , int length);
bool bootloaderReadyToJump();
void bootloaderJumpToApplication();
