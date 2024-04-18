#include "gw_multisync.h"
#include <stdio.h>

#include "gw_lcd.h"
#include "gw_multisync_pid.h"

/**
 * Segal's law:
 * A man with a watch knows what time it is.
 * A man with two watches is never sure.
 */

// PID parameters
#define KP 21.0 // Proportional gain
#define KI 8    // Integral gain
#define KD 0.0  // Derivative gain

// PID clamping values
#define PID_MIN_OUTPUT (-4096)
#define PID_MAX_OUTPUT 4095

// Sync after last line
#define MIN_IN_SYNC_COUNT 10

//#define MULTISYNC_DEBUG

static PIDController pidController;
static uint32_t in_sync_count;
static bool initialized = false;

void multisync_init() {
    PID_Init(&pidController, 0, KP, KI, KD, PID_MIN_OUTPUT, PID_MAX_OUTPUT);
    in_sync_count = 0;
    initialized = true;
}

bool multisync_is_synchronized() {
    return in_sync_count >= MIN_IN_SYNC_COUNT && initialized;
}

static void multisync_common_handler() {
    if (!initialized) {
        return;
    }

    // Note: The total number of lines in the LCD, both visible and invisible is 256.
    uint8_t current_line = lcd_get_pixel_position() & 0xFF;
    int8_t normalized_line = current_line - GW_LCD_RELOAD_LINE;

    if (current_line >= GW_LCD_RELOAD_LINE - 1 && current_line <= GW_LCD_RELOAD_LINE + 1) {
        in_sync_count += 1;
    } else {
        in_sync_count = 0;
    }

    double adjust = PID_Update(&pidController, normalized_line);
    lcd_set_pll_fracn((int32_t) adjust + GW_LCD_PLL_FRACN_CENTER);
#ifdef MULTISYNC_DEBUG
    printf("MultiSync: frame: %ld line: %d adjust: %d in sync: %d\n", lcd_get_frame_counter(), normalized_line, (uint16_t) adjust, multisync_is_synchronized());
#endif
}

void multisync_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    // Call the original hook (weak or not)
    HAL_SAI_TxHalfCpltCallback(hsai);
    multisync_common_handler();
}

void multisync_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    // Call the original hook (weak or not)
    HAL_SAI_TxCpltCallback(hsai);
    multisync_common_handler();
}
