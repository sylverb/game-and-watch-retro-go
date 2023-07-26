#pragma once

#define LOGI printf
#define CPC_SCREEN_WIDTH 384
#define CPC_SCREEN_HEIGHT 272

#include "filesystem.h"

extern uint8_t amstrad_framebuffer[];

// COMPUTER/EMU STATUS
#define COMPUTER_OFF     0
#define COMPUTER_BOOTING 1
#define COMPUTER_READY   2
extern int emu_status;
extern int cpc_kbd[];

void load_amstrad_data(fs_file_t *file);
void save_amstrad_data(fs_file_t *file);
void video_set_palette_antialias_8bpp(void);
unsigned int rgb2color_8bpp(unsigned int r, unsigned int g, unsigned int b);
unsigned int * amstrad_getScreenPtr();
void app_main_amstrad(uint8_t load_state, uint8_t start_paused, uint8_t save_slot);
