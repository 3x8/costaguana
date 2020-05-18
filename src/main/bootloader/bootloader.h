#pragma once

#include "main.h"

void bootloaderFlashRead(uint8_t*  data , uint32_t add ,int  out_buff_len);
void bootloaderFlashWrite(uint8_t *data, int length, uint32_t add);
