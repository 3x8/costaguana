#pragma once

// input ,Ok
#define INPUT_GPIO      GPIOA
#define INPUT_PIN       LL_GPIO_PIN_2

// LEDs ,no
#define LED_RED_GPIO    GPIOA
#define LED_RED_PIN     LL_GPIO_PIN_15
#define LED_GREEN_GPIO  GPIOB
#define LED_GREEN_PIN   LL_GPIO_PIN_6
#define LED_BLUE_GPIO   GPIOA
#define LED_BLUE_PIN    LL_GPIO_PIN_15
#define LED_MASK        LL_AHB1_GRP1_PERIPH_GPIOA | LL_AHB1_GRP1_PERIPH_GPIOB
