#ifndef _GW_MULTISYNC_H_
#define _GW_MULTISYNC_H_

#include <stdbool.h>
#include "stm32h7xx_hal.h"

void multisync_init();
bool multisync_is_synchronized();
void multisync_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai);
void multisync_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai);

#endif
