#pragma once

// input begin
#define INPUT_GPIO      GPIOA
#define INPUT_PIN       LL_GPIO_PIN_6

// LEDs
#define LED_RED_GPIO    GPIOA
#define LED_RED_PIN     LL_GPIO_PIN_3
#define LED_GREEN_GPIO  GPIOA
#define LED_GREEN_PIN   LL_GPIO_PIN_3
#define LED_BLUE_GPIO   GPIOA
#define LED_BLUE_PIN    LL_GPIO_PIN_3
#define LED_MASK        LL_AHB1_GRP1_PERIPH_GPIOA | LL_AHB1_GRP1_PERIPH_GPIOB
