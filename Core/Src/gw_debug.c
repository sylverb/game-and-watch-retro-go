#include "gw_debug.h"


/**
 * Schematics source https://raw.githubusercontent.com/Upcycle-Electronics/game-and-watch-hardware/main/Images-Version-Current/GnWschematic1v2.jpg
 *
 * On Zelda:
 *   PC11 & PC12 are connected to the START and SELECT button which are missing on Mario.
 *   PB3 are connected to unknown
 */

/**
 * Enable all debug pins as medium speed, pull down and output.
 */
void debug_pins_init()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

/**
 * Disable all debug pins.
 */
void debug_pins_deinit()
{
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_9);
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0 | GPIO_PIN_1);
}
