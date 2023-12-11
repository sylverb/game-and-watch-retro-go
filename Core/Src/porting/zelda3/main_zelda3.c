/*
TODO copyright?
*/
#include "build/config.h"
#ifdef ENABLE_HOMEBREW_ZELDA3

#include <odroid_system.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

// FIXME gw includes ?
#include "main.h"
#include "gw_lcd.h"
#include "gw_linker.h"
#include "gw_buttons.h"
#include "gw_flash.h"
#include "lzma.h"

/* TO move elsewhere */
#include "stm32h7xx_hal.h"

// FIXME retro-go includes ?
#include "common.h"
#include "rom_manager.h"
#include "appid.h"
#include "rg_i18n.h"

// FIXME zelda3 includes ?
#include "zelda3/zelda_rtl.h"

// FIXME optimization flags ?
#pragma GCC optimize("Ofast")


// FIXME zelda3 global variables, settings, etc. (c.f. main.c in zelda3 standalone)

// FIXME 30/60 fps ???
#define FRAMERATE 30

static uint32 frameCtr = 0;
static uint32 renderedFrameCtr = 0;


/* keys inputs (hw & sw) FIXME needed? */
static odroid_gamepad_state_t joystick;

// FIXME buttons mappings from zelda and mario units
#if GNW_TARGET_ZELDA != 0

#else

#endif

/* FIXME ??? callback used by the emulator to capture keys */
void zelda3_io_get_buttons()
{
}

// FIXME init game?
static void zelda3_system_init() {
}
static void zelda3_sound_start()
{
}

// FIXME audio configuration?
// FIXME overclocking??
/* AUDIO PLL controller */
static void zelda3_audio_pll_stepdown() {
}
static void zelda3_audio_pll_stepup() {
}

static void zelda3_audio_pll_center() {
}

// FIXME sound?
static void zelda3_sound_submit() {
}

// FIXME in-game retro-go menu??
static bool zelda3_submenu_setABC(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
}


// FIXME in-game retro-go menu settings??
void zelda3_save_local_data(void) {
}

void zelda3_load_local_data(void) {
}

static bool zelda3_system_SaveState(char *pathName) {
}

static bool zelda3_system_LoadState(char *pathName) {
}

/* Main */
int app_main_zelda3(uint8_t load_state, uint8_t start_paused, uint8_t save_slot)
{
  // TODO init settings + main loop

  printf("Zelda3 start\n");

  // TODO write something to current framebuffer as a first step


  common_emu_state.frame_time_10us = (uint16_t)(100000 / FRAMERATE + 0.5f);


  /* clear the screen before rendering */
  memset(lcd_get_inactive_buffer(), 0, 320 * 240 * 2);
  memset(lcd_get_active_buffer(), 0, 320 * 240 * 2);
  

  unsigned short *screen = 0;

  screen = lcd_get_active_buffer();

  /* Start at the same time DMAs audio & video */
  /* Audio period and Video period are the same (almost at least 1 hour) */
  lcd_wait_for_vblank();
  zelda3_sound_start();

  // gwenesis_init_position = 0xFFFF & lcd_get_pixel_position();
  while (true) {

    /* reset watchdog */
    wdog_refresh();

    /* hardware keys */
    odroid_input_read_gamepad(&joystick);


    odroid_dialog_choice_t options[] = {
            ODROID_DIALOG_CHOICE_LAST
    };
    // FIXME repaint???
    void _repaint()
    {
        // TODO blit game???
        common_ingame_overlay();
    }
    common_emu_input_loop(&joystick, options, &_repaint);


    bool drawFrame = common_emu_frame_loop();

    frameCtr++;


    if (drawFrame) {
      // todo switch framebuffer ??
      screen = lcd_get_active_buffer();

      // TODO draw something !!!
      memset(screen, (frameCtr % 0xff), 320 * 240 * 2);

      // FIXME in-game overlay ???
      common_ingame_overlay();

      lcd_swap();

      renderedFrameCtr++;
    }

    /*if(!common_emu_state.skip_frames)
    {
        // odroid_audio_submit(pcm.buf, pcm.pos >> 1);
        // handled in pcm_submit instead.
        static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
        for(uint8_t p = 0; p < common_emu_state.pause_frames + 1; p++) {
            while (dma_state == last_dma_state) {
                cpumon_sleep();
            }
            last_dma_state = dma_state;
        }
    }*/
    lcd_wait_for_vblank();

    // TODO How to exit game???

  }

}
#endif
