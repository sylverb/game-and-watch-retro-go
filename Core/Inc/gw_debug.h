#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "main.h"

#define DEBUG_PIN_0 (GPIO_PIN_0)
#define DEBUG_PIN_1 (GPIO_PIN_1)
#define DEBUG_PIN_2 (GPIO_PIN_9)
#define DEBUG_PIN_3 (GPIO_PIN_7)
#define DEBUG_PIN_4 (GPIO_PIN_6)
#define DEBUG_PIN_5 (GPIO_PIN_4)
#define DEBUG_PORT_PIN_0 GPIOE
#define DEBUG_PORT_PIN_1 GPIOE
#define DEBUG_PORT_PIN_2 GPIOB
#define DEBUG_PORT_PIN_3 GPIOB
#define DEBUG_PORT_PIN_4 GPIOB
#define DEBUG_PORT_PIN_5 GPIOB

void debug_pins_init();
void debug_pins_deinit();

#endif
