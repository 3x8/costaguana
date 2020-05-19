#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_gpio.h"

#include "target.h"
#include "version/version.h"
#include "bootloader/bootloader.h"
#include "system/system.h"
#include "system/led.h"

#define FLASH_FKEY1 ((uint32_t)0x45670123)
#define FLASH_FKEY2 ((uint32_t)0xCDEF89AB)

#define FOUR_WAY_BAUD_RATE  19200
#define FOUR_WAY_BIT_TIME (1000000/FOUR_WAY_BAUD_RATE)
#define FOUR_WAY_BIT_TIME_HALF (500000/FOUR_WAY_BAUD_RATE)
#define FOUR_WAY_SHIFT_AMOUNT 2

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
#define EEPROM_CONF_VERSION     3

// USE_SERIAL_4WAY_BLHELI_BOOTLOADER
#define CMD_RUN               0x00
#define CMD_PROG_FLASH        0x01
#define CMD_ERASE_FLASH       0x02
#define CMD_READ_FLASH_SIL    0x03
#define CMD_VERIFY_FLASH      0x03
#define CMD_VERIFY_FLASH_ARM  0x04
#define CMD_READ_EEPROM       0x04
#define CMD_PROG_EEPROM       0x05
#define CMD_READ_SRAM         0x06
#define CMD_READ_FLASH_ATM    0x07
#define CMD_KEEP_ALIVE        0xfd
#define CMD_SET_ADDRESS       0xff
#define CMD_SET_BUFFER        0xfe
// reversed
#define CMD_ACK_OK            0x30
#define CMD_ACK_KO            0xC1


typedef union __attribute__ ((packed)) {
  uint8_t bytes[2];
  uint16_t word;
} crc_16_u;


void delayMicroseconds(uint32_t micros);
void jump();
void makeCrc(uint8_t* pBuff, uint16_t length);
bool checkCrc(uint8_t* pBuff, uint16_t length);
void fourWaySetReceive();
void fourWaySetTransmit();
void fourWayPutDeviceInfo();
void decodeInput();
void fourWayGetChar();
void fourWayPutChar(char data);
void fourWayGetBuffer();
void fourWayPutBuffer(uint8_t *data, int cmdLength);
