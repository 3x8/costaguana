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

void bootloaderFlashRead(uint8_t*  data , uint32_t add ,int  out_buff_len);
void bootloaderFlashWrite(uint8_t *data, int length, uint32_t add);
