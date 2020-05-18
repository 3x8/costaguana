#pragma once

#include "main.h"

void bootloaderFlashWrite(uint8_t *data, int length, uint32_t address);
void bootloaderFlashRead(uint8_t*  data , uint32_t address , int length);
