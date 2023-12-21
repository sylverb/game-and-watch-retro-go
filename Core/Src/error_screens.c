#include <odroid_system.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "appid.h"
#include "main.h"
#include "gui.h"
#include "githash.h"
#include "main.h"
#include "gw_lcd.h"
#include "gw_buttons.h"
#include "gw_flash.h"
#include "rg_rtc.h"
#include "rg_i18n.h"
#include "bitmaps.h"
#include "error_screens.h"

static const uint8_t img_error[] = {
    0x0F, 0xFC, 0x12, 0x4A, 0x12, 0x4A,
    0x20, 0x02, 0x24, 0x22, 0x4E, 0x72,
    0x40, 0x02, 0x43, 0xC2, 0x47, 0xE2,
    0x4C, 0x32, 0x40, 0x02, 0x40, 0x02,
    0x40, 0x02, 0x3F, 0xFC,
};

static inline int compute_x(const char *str){
    return (ODROID_SCREEN_WIDTH - (8 * strlen(str))) >> 1;
}

/**
 * Draws the sad rabbit graphics used when:
 *     1. The external flash hasn't been programmed.
 *     2. The filesystem is corrupt.
 */
void draw_error_screen(const char *main_line, const char *line_1, const char *line_2){
    lcd_set_buffers(framebuffer1, framebuffer2);
    odroid_overlay_draw_fill_rect(0, 0, ODROID_SCREEN_WIDTH, ODROID_SCREEN_HEIGHT, curr_colors->bg_c);
    for (int y = 0; y < 14; y++){
        uint8_t pt = img_error[2 * y];
        for (int x = 0; x < 8; x++)
            if (pt & (0x80 >> x))
                odroid_overlay_draw_fill_rect((12 + x) * 8, (9 + y) * 8, 8, 8, curr_colors->main_c);
        pt = img_error[2 * y + 1];
        for (int x = 0; x < 8; x++)
            if (pt & (0x80 >> x))
                odroid_overlay_draw_fill_rect((20 + x) * 8, (9 + y) * 8, 8, 8, curr_colors->main_c);
    }

    odroid_overlay_draw_logo((ODROID_SCREEN_WIDTH - logo_rgo.width) / 2, 42, (retro_logo_image *)(&logo_rgo), curr_colors->sel_c);
    if (main_line) {
        odroid_overlay_draw_text_line(compute_x(main_line), 20 * 8, strlen(main_line) * 8, main_line, C_RED, curr_colors->bg_c);
    }
    if (line_1) {
        odroid_overlay_draw_text_line(compute_x(line_1), 24 * 8 - 4, strlen(line_1) * 8, line_1, curr_colors->dis_c, curr_colors->bg_c);
    }
    if (line_2) {
        odroid_overlay_draw_text_line(compute_x(line_2), 25 * 8, strlen(line_2) * 8, line_2, curr_colors->dis_c, curr_colors->bg_c);
    }
    odroid_overlay_draw_text_line(ODROID_SCREEN_WIDTH - strlen(GIT_HASH) * 8 - 4, 29 * 8 - 4, strlen(GIT_HASH) * 8, GIT_HASH, curr_colors->sel_c, curr_colors->bg_c);
}
