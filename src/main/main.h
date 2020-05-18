#pragma once

#include <stdbool.h>
#include <string.h>

#include "target.h"
#include "build/version.h"

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

#define FLASH_FKEY1 ((uint32_t)0x45670123)
#define FLASH_FKEY2 ((uint32_t)0xCDEF89AB)

#define BAUDRATE 19200
#define BITTIME 1000000/BAUDRATE
#define HALFBITTIME 500000/BAUDRATE
#define SHIFT_AMOUNT 2

#define APPLICATION_ADDRESS (uint32_t)0x08001000               // 4k
#define EEPROM_START_ADD  (uint32_t)0x08007C00

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
#define CMD_KEEP_ALIVE        0xFD
#define CMD_SET_ADDRESS       0xFF
#define CMD_SET_BUFFER        0xFE


void bootloaderFlashRead(uint8_t*  data , uint32_t add ,int  out_buff_len);
void bootloaderFlashWrite(uint8_t *data, int length, uint32_t add);

typedef void (*pFunction)(void);
