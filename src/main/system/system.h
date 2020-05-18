#pragma once

#include "main.h"

typedef void (*pFunction)(void);

void systemClockConfig(void);
void systemGpioInit(void);
void systemTim2Init(void);
