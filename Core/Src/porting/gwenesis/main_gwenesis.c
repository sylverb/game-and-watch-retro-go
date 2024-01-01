/*
Gwenesis : Genesis & megadrive Emulator.

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.
This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.

__author__ = "bzhxx"
__contact__ = "https://github.com/bzhxx"
__license__ = "GPLv3"

*/
#include "build/config.h"
#ifdef ENABLE_EMULATOR_MD

#include <odroid_system.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "main.h"
#include "gw_lcd.h"
#include "gw_linker.h"
#include "gw_buttons.h"
#include "gw_flash.h"
#include "lzma.h"

/* TO move elsewhere */
#include "stm32h7xx_hal.h"

#include "common.h"
#include "rom_manager.h"
#include "appid.h"
#include "rg_i18n.h"

/* Gwenesis Emulator */
#include "m68k.h"
#include "z80inst.h"
#include "ym2612.h"
#include "gwenesis_sn76489.h"
#include "gwenesis_bus.h"
#include "gwenesis_io.h"
#include "gwenesis_vdp.h"
#include "gwenesis_savestate.h"

#pragma GCC optimize("Ofast")

#define ENABLE_DEBUG_OPTIONS 0

static const uint8_t IMG_DISKETTE[] = {
    0x00, 0x00, 0x00, 0x3F, 0xFF, 0xE0, 0x7C, 0x00, 0x70, 0x7C, 0x03, 0x78,
    0x7C, 0x03, 0x7C, 0x7C, 0x03, 0x7E, 0x7C, 0x00, 0x7E, 0x7F, 0xFF, 0xFE,
    0x7F, 0xFF, 0xFE, 0x7F, 0xFF, 0xFE, 0x7F, 0xFF, 0xFE, 0x7F, 0xFF, 0xFE,
    0x7F, 0xFF, 0xFE, 0x7E, 0x00, 0x7E, 0x7C, 0x00, 0x3E, 0x7C, 0x00, 0x3E,
    0x7D, 0xFF, 0xBE, 0x7C, 0x00, 0x3E, 0x7C, 0x00, 0x3E, 0x7D, 0xFF, 0xBE,
    0x7C, 0x00, 0x3E, 0x7C, 0x00, 0x3E, 0x3F, 0xFF, 0xFC, 0x00, 0x00, 0x00,
};

static void load_rom_from_flash() {
  /* check if it's compressed */

  if (strcmp(ROM_EXT, "lzma") == 0) {

    assert((&__CACHEFLASH_END__ - &__CACHEFLASH_START__) > 0);

    /* check header  */
    assert(memcmp((uint8_t *)ROM_DATA, "SMS+", 4) == 0);

    unsigned int nb_banks = 0;
    unsigned int lzma_bank_size = 0;
    unsigned int lzma_bank_offset = 0;
    unsigned int uncompressed_rom_size = 0;

    memcpy(&nb_banks, &ROM_DATA[4], sizeof(nb_banks));

    lzma_bank_offset = 4 + 4 + 4 * nb_banks;

    for (int i = 0; i < nb_banks; i++) {
      wdog_refresh();
      memcpy(&lzma_bank_size, &ROM_DATA[8 + 4 * i], sizeof(lzma_bank_size));
      memset((uint8_t *)lcd_get_inactive_buffer(), 0x0, 320 * 240 * 2);

      uint16_t *dest = lcd_get_inactive_buffer();

      /* uncompressed in lcd framebuffer */
      size_t n_decomp_bytes;
      n_decomp_bytes =
          lzma_inflate((uint8_t *)lcd_get_active_buffer(), 2 * 320 * 240,
                       &ROM_DATA[lzma_bank_offset], lzma_bank_size);

      assert((&__CACHEFLASH_END__ - &__CACHEFLASH_START__) >=
             ((uint32_t)n_decomp_bytes + uncompressed_rom_size));

      int diff = memcmp((void *)(&__CACHEFLASH_START__ + uncompressed_rom_size),
                        (uint8_t *)lcd_get_active_buffer(), n_decomp_bytes);
      if (diff != 0) {
        wdog_refresh();
        OSPI_DisableMemoryMappedMode();

        /* display diskette during flash erase */
        uint16_t idx = 0;
        for (uint8_t i = 0; i < 24; i++) {
          for (uint8_t j = 0; j < 24; j++) {
            if (IMG_DISKETTE[idx / 8] & (1 << (7 - idx % 8))) {
              dest[286 + j + GW_LCD_WIDTH * (10 + i)] = 0xFFFF;
            }
            idx++;
          }
        }

        /* erase the cache */
        OSPI_EraseSync((&__CACHEFLASH_START__ - &__EXTFLASH_BASE__) +
                           uncompressed_rom_size,
                       (uint32_t)n_decomp_bytes);

        /* blink diskette icon during flash program */
        for (short y = 0; y < 24; y++) {
          uint16_t *dest_row = &dest[(y + 10) * GW_LCD_WIDTH + 286];
          memset(dest_row, 0x0, 24 * sizeof(uint16_t));
        }

        /* program the cache */
        wdog_refresh();
        OSPI_Program(
            (&__CACHEFLASH_START__ - &__EXTFLASH_BASE__) + uncompressed_rom_size,
            (uint8_t *)lcd_get_active_buffer(), (uint32_t)n_decomp_bytes);

        OSPI_EnableMemoryMappedMode();
        wdog_refresh();
      }

      lzma_bank_offset += lzma_bank_size;
      uncompressed_rom_size += (uint32_t)n_decomp_bytes;
    }

    /* set the rom pointer and size */
    ROM_DATA = &__CACHEFLASH_START__;
    ROM_DATA_LENGTH = uncompressed_rom_size;
  }
}

static unsigned int gwenesis_show_debug_bar = 0;


unsigned int gwenesis_audio_freq;
unsigned int gwenesis_audio_buffer_lenght;
unsigned int gwenesis_audio_buffer_lenght_DMA;
static int gwenesis_vsync_mode = 0;
unsigned int gwenesis_refresh_rate;

unsigned int gwenesis_lcd_current_line;
#define GWENESIS_AUDIOSYNC_START_LCD_LINE 248

static int gwenesis_lpfilter = 0;
extern int gwenesis_H32upscaler;
static unsigned int gwenesis_audio_pll_sync = 0;

/* Clocks and synchronization */
/* system clock is video clock */
int system_clock;

/* shared variables with gwenesis_sn76589 */
int16_t gwenesis_sn76489_buffer[GWENESIS_AUDIO_BUFFER_LENGTH_PAL];
int sn76489_index; /* sn78649 audio buffer index */
int sn76489_clock; /* sn78649 clock in video clock resolution */


/* shared variables with gwenesis_ym2612 */
int16_t gwenesis_ym2612_buffer[GWENESIS_AUDIO_BUFFER_LENGTH_PAL];
int ym2612_index; /* ym2612 audio buffer index */
int ym2612_clock; /* ym2612 clock in video clock resolution */

/* keys inpus (hw & sw) */
static odroid_gamepad_state_t joystick;

/* Configurable keys mapping for A,B and C */

extern unsigned short button_state[3];

#define NB_OF_COMBO 6

#if GNW_TARGET_ZELDA != 0

const char ODROID_INPUT_DEF_C = ODROID_INPUT_X;
static int ABCkeys_value = 5;
static int PAD_A_def = ODROID_INPUT_A;
static int PAD_B_def = ODROID_INPUT_B;
static int PAD_C_def = ODROID_INPUT_DEF_C;
static const char ABCkeys_combo_str[NB_OF_COMBO][10] = {"B-A-START", "A-B-START","B-START-A","A-START-B","START-A-B","START-B-A"};
static char ABCkeys_str[10]="START-B-A";

#else

const char ODROID_INPUT_DEF_C = ODROID_INPUT_VOLUME;
static int ABCkeys_value = 5;
static int PAD_A_def = ODROID_INPUT_A;
static int PAD_B_def = ODROID_INPUT_B;
static int PAD_C_def = ODROID_INPUT_DEF_C;
static const char ABCkeys_combo_str[NB_OF_COMBO][10] = { "B-A-PAUSE", "A-B-PAUSE","B-PAUSE-A","A-PAUSE-B","PAUSE-A-B","PAUSE-B-A"};
static char ABCkeys_str[10]="PAUSE-B-A";

#endif

/* callback used by the meluator to capture keys */
void gwenesis_io_get_buttons()
{
  /* Keys mapping */
  /*
  * GAME is START (ignore)
  * TIME is SELECT
  * PAUSE/SET is VOLUME
  */

  odroid_gamepad_state_t host_joystick;

  odroid_input_read_gamepad(&host_joystick);

  /* shortcut is active ignore keys for the emulator */
  #if GNW_TARGET_ZELDA != 0
    if ( host_joystick.values[ODROID_INPUT_VOLUME] ) return;
  #else
    if ( host_joystick.values[ODROID_INPUT_SELECT] ) return;
  #endif

  button_state[0] = host_joystick.values[ODROID_INPUT_UP] << PAD_UP |
                    host_joystick.values[ODROID_INPUT_DOWN] << PAD_DOWN |
                    host_joystick.values[ODROID_INPUT_LEFT] << PAD_LEFT |
                    host_joystick.values[ODROID_INPUT_RIGHT] << PAD_RIGHT |
                    host_joystick.values[PAD_A_def] << PAD_A |
                    host_joystick.values[PAD_B_def] << PAD_B |
                    host_joystick.values[PAD_C_def] << PAD_C |
                    host_joystick.values[ODROID_INPUT_START] << PAD_S;
                  
  button_state[0] = ~ button_state[0];

}

static void gwenesis_system_init() {

  /* init emulator sound system with shared audio buffer */
 // extern int mode_pal;

  gwenesis_audio_pll_sync = 0;

  /* clear DMA audio buffer */
  memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma ));

//  memset(audiobuffer_emulator, 0, sizeof(audiobuffer_emulator));

 // if (mode_pal) {

  //  gwenesis_audio_freq = GWENESIS_AUDIO_FREQ_PAL;
  //  gwenesis_audio_buffer_lenght = GWENESIS_AUDIO_BUFFER_LENGTH_PAL;
  //  gwenesis_audio_buffer_lenght_DMA = 2 * GWENESIS_AUDIO_BUFFER_LENGTH_PAL;
  //  gwenesis_refresh_rate = GWENESIS_REFRESH_RATE_PAL;

 // } else {

    gwenesis_audio_freq = GWENESIS_AUDIO_FREQ_NTSC;
    gwenesis_audio_buffer_lenght = GWENESIS_AUDIO_BUFFER_LENGTH_NTSC;
    gwenesis_audio_buffer_lenght_DMA = 2 * GWENESIS_AUDIO_BUFFER_LENGTH_NTSC;
    gwenesis_refresh_rate = GWENESIS_REFRESH_RATE_NTSC;
 // }
    memset(gwenesis_sn76489_buffer,0,sizeof(gwenesis_sn76489_buffer));
    memset(gwenesis_ym2612_buffer,0,sizeof(gwenesis_ym2612_buffer));

  odroid_audio_init(gwenesis_audio_freq);
  lcd_set_refresh_rate(gwenesis_refresh_rate);
}
static void gwenesis_sound_start()
{
     /* Start SAI DMA */
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, gwenesis_audio_buffer_lenght_DMA);

}

/* PLL Audio controller to synchonize with video clock */
// Center value is 4566
#define GWENESIS_FRACN_PAL_DOWN 4000
#define GWENESIS_FRACN_PAL_UP 5000

// Center value is 4566
#define GWENESIS_FRACN_NTSC_DOWN 4000
#define GWENESIS_FRACN_NTSC_UP 5000

#define GWENESIS_FRACN_DOWN 3000
#define GWENESIS_FRACN_UP 6000

#define GWENESIS_FRACN_CENTER 4566

/* AUDIO PLL controller */
static void gwenesis_audio_pll_stepdown() {

  __HAL_RCC_PLL2FRACN_DISABLE();
  __HAL_RCC_PLL2FRACN_CONFIG(GWENESIS_FRACN_DOWN);
  __HAL_RCC_PLL2FRACN_ENABLE();
  gwenesis_audio_pll_sync = 0;
}
static void gwenesis_audio_pll_stepup() {

  __HAL_RCC_PLL2FRACN_DISABLE();
  __HAL_RCC_PLL2FRACN_CONFIG(GWENESIS_FRACN_UP);
  __HAL_RCC_PLL2FRACN_ENABLE();
  gwenesis_audio_pll_sync = 0;
}

static void gwenesis_audio_pll_center() {
  if (gwenesis_audio_pll_sync == 0) {
    __HAL_RCC_PLL2FRACN_DISABLE();
    __HAL_RCC_PLL2FRACN_CONFIG(GWENESIS_FRACN_CENTER);
    __HAL_RCC_PLL2FRACN_ENABLE();
    gwenesis_audio_pll_sync = 1;
  }
}

/* single-pole low-pass filter (6 dB/octave) */
const uint32_t factora  = 0x1000; // todo as UI parameter
const uint32_t factorb  = 0x10000 - factora;

static void gwenesis_sound_submit() {
  uint8_t volume = odroid_audio_volume_get();
  int16_t factor = volume_tbl[volume];

  static int16_t gwenesis_audio_out = 0;

  size_t offset;
  offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : gwenesis_audio_buffer_lenght;

  if (audio_mute || volume == ODROID_AUDIO_VOLUME_MIN) {
    for (int i = 0; i < gwenesis_audio_buffer_lenght; i++) {
      audiobuffer_dma[i + offset] = 0;
    }
  } else {
    // filter on
    if (gwenesis_lpfilter) {
      for (int i = 0; i < gwenesis_audio_buffer_lenght; i++) {

        // low pass filter
        int32_t gwenesis_audio_tmp = gwenesis_audio_out * factorb + (gwenesis_ym2612_buffer[i] + gwenesis_sn76489_buffer[i]) * factora;
        gwenesis_audio_out = gwenesis_audio_tmp >> 16;
        audiobuffer_dma[i + offset] = ((gwenesis_audio_out) * factor ) / 128;
      }
    // filter off
    } else {
      for (int i = 0; i < gwenesis_audio_buffer_lenght; i++) {

        // single mone left or right
       // audiobuffer_dma[i + offset] = ((gwenesis_ym2612_buffer[i]) * factor) / 256;
        audiobuffer_dma[i + offset] = ((gwenesis_ym2612_buffer[i] + gwenesis_sn76489_buffer[i]) * factor) / 512;
      }
    }
  }
}

/************************ Debug function in overlay START *******************************/

/* performance monitoring */
/* Emulator loop monitoring
    ( unit is 1/systemcoreclock 1/280MHz or 1/312MHz or 1/340MHz)
    loop_cycles
        -measured duration of the loop.
    end_cycles
        - estimated duration of overall processing.
    */

static unsigned int loop_cycles = 1,end_cycles = 1;
/* DWT counter used to measure time execution */
volatile unsigned int *GWENESIS_DWT_CONTROL = (unsigned int *)0xE0001000;
volatile unsigned int *GWENESIS_DWT_CYCCNT = (unsigned int *)0xE0001004;
volatile unsigned int *GWENESIS_DEMCR = (unsigned int *)0xE000EDFC;
volatile unsigned int *GWENESIS_LAR = (unsigned int *)0xE0001FB0; // <-- lock access register

#define get_dwt_cycles() *GWENESIS_DWT_CYCCNT
#define clear_dwt_cycles() *GWENESIS_DWT_CYCCNT = 0
static unsigned int overflow_count = 0;

static void enable_dwt_cycles()
{
    /* Use DWT cycle counter to get precision time elapsed during loop.
    The DWT cycle counter is cleared on every loop
    it may crash if the DWT is used during trace profiling */

    *GWENESIS_DEMCR = *GWENESIS_DEMCR | 0x01000000;    // enable trace
    *GWENESIS_LAR = 0xC5ACCE55;                  // <-- added unlock access to DWT (ITM, etc.)registers
    *GWENESIS_DWT_CYCCNT = 0;                    // clear DWT cycle counter
    *GWENESIS_DWT_CONTROL = *GWENESIS_DWT_CONTROL | 1; // enable DWT cycle counter
}

static void gwenesis_debug_bar()
{

  static unsigned int loop_duration_us = 1;
  static unsigned int end_duration_us = 1;
  static unsigned int cpu_workload = 0;
  static const unsigned int SYSTEM_CORE_CLOCK_MHZ = 353; //340; //312; // 280;

  static bool debug_init_done = false;

  if (!debug_init_done) {
    enable_dwt_cycles();
    debug_init_done = true;
  }

  char debugMsg[120];

  end_duration_us = end_cycles / SYSTEM_CORE_CLOCK_MHZ;
  loop_duration_us = loop_cycles / SYSTEM_CORE_CLOCK_MHZ;
  cpu_workload = 100 * end_cycles / loop_cycles;

  if (gwenesis_lcd_current_line == GWENESIS_AUDIOSYNC_START_LCD_LINE)
    sprintf(debugMsg, "%05dus %05dus%3d %6ld %3d SYNC Y%3d", loop_duration_us,
            end_duration_us, cpu_workload, frame_counter, overflow_count,
            gwenesis_lcd_current_line);
  else
    sprintf(debugMsg, "%05dus %05dus%3d %6ld %3d  ..  Y%3d", loop_duration_us,
            end_duration_us, cpu_workload, frame_counter, overflow_count,
            gwenesis_lcd_current_line);


  odroid_overlay_draw_text(0, 0, 320, debugMsg, C_GW_YELLOW, C_GW_RED);

}
/************************ Debug function in overlay END ********************************/
/**************************** */

unsigned int lines_per_frame = LINES_PER_FRAME_NTSC; //262; /* NTSC: 262, PAL: 313 */
unsigned int scan_line;
unsigned int drawFrame = 1;


// static char gwenesis_GameGenie_str[10]="....-....";
// static bool gwenesis_submenu_GameGenie(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
// {

//     // if (event == ODROID_DIALOG_PREV)
//     //     flag_lcd_deflicker_level = flag_lcd_deflicker_level > 0 ? flag_lcd_deflicker_level - 1 : max_flag_lcd_deflicker_level;

//     // if (event == ODROID_DIALOG_NEXT)
//     //     flag_lcd_deflicker_level = flag_lcd_deflicker_level < max_flag_lcd_deflicker_level ? flag_lcd_deflicker_level + 1 : 0;

//     // if (flag_lcd_deflicker_level == 0) strcpy(option->value, "0-none");
//     // if (flag_lcd_deflicker_level == 1) strcpy(option->value, "1-medium");
//     // if (flag_lcd_deflicker_level == 2) strcpy(option->value, "2-high");

//     return event == ODROID_DIALOG_ENTER;
// }

// static char gwenesis_GameGenie_reverse_str[10]="....-....";
// static bool gwenesis_submenu_GameGenie_reverse(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
// {


//     // if (event == ODROID_DIALOG_PREV)
//     //     flag_lcd_deflicker_level = flag_lcd_deflicker_level > 0 ? flag_lcd_deflicker_level - 1 : max_flag_lcd_deflicker_level;

//     // if (event == ODROID_DIALOG_NEXT)
//     //     flag_lcd_deflicker_level = flag_lcd_deflicker_level < max_flag_lcd_deflicker_level ? flag_lcd_deflicker_level + 1 : 0;

//     // if (flag_lcd_deflicker_level == 0) strcpy(option->value, "0-none");
//     // if (flag_lcd_deflicker_level == 1) strcpy(option->value, "1-medium");
//     // if (flag_lcd_deflicker_level == 2) strcpy(option->value, "2-high");

//     return event == ODROID_DIALOG_ENTER;
// }

static bool gwenesis_submenu_setABC(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{

    if (event == ODROID_DIALOG_PREV)
      ABCkeys_value = ABCkeys_value > 0 ? ABCkeys_value - 1 : NB_OF_COMBO-1;

    if (event == ODROID_DIALOG_NEXT)
      ABCkeys_value = ABCkeys_value < NB_OF_COMBO-1 ? ABCkeys_value + 1 : 0;

    strcpy(option->value, ABCkeys_combo_str[ABCkeys_value]);

    switch (ABCkeys_value) {
    case 0:
      PAD_A_def = ODROID_INPUT_B;
      PAD_B_def = ODROID_INPUT_A;
      PAD_C_def = ODROID_INPUT_DEF_C;
      break;
    case 1:
      PAD_A_def = ODROID_INPUT_A;
      PAD_B_def = ODROID_INPUT_B;
      PAD_C_def = ODROID_INPUT_DEF_C;
      break;
    case 2:
      PAD_A_def = ODROID_INPUT_B;
      PAD_B_def = ODROID_INPUT_DEF_C;
      PAD_C_def = ODROID_INPUT_A;
      break;
    case 3:
      PAD_A_def = ODROID_INPUT_A;
      PAD_B_def = ODROID_INPUT_DEF_C;
      PAD_C_def = ODROID_INPUT_B;
      break;
    case 4:
      PAD_A_def = ODROID_INPUT_DEF_C;
      PAD_B_def = ODROID_INPUT_A;
      PAD_C_def = ODROID_INPUT_B;
      break;
    case 5:
      PAD_A_def = ODROID_INPUT_DEF_C;
      PAD_B_def = ODROID_INPUT_B;
      PAD_C_def = ODROID_INPUT_A;
      break;
    default:
      PAD_A_def = ODROID_INPUT_A;
      PAD_B_def = ODROID_INPUT_B;
      PAD_C_def = ODROID_INPUT_DEF_C;
      break;
    }

    return event == ODROID_DIALOG_ENTER;
}

static bool gwenesis_submenu_setAudioFilter(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
    gwenesis_lpfilter = gwenesis_lpfilter == 0 ? 1 : 0;
    }

    if (gwenesis_lpfilter == 0) strcpy(option->value, curr_lang->s_md_Option_OFF);
    if (gwenesis_lpfilter == 1) strcpy(option->value, curr_lang->s_md_Option_ON);

    return event == ODROID_DIALOG_ENTER;
}

#if ENABLE_DEBUG_OPTIONS != 0
// Some options using submenu
static bool gwenesis_submenu_debug_bar(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
  if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
      gwenesis_show_debug_bar = gwenesis_show_debug_bar == 0 ? 1 : 0;
    }
    if (gwenesis_show_debug_bar == 0) strcpy(option->value, curr_lang->s_md_Option_OFF);
    if (gwenesis_show_debug_bar == 1) strcpy(option->value, curr_lang->s_md_Option_ON);

    return event == ODROID_DIALOG_ENTER;
}

static bool gwenesis_submenu_setVideoUpscaler(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
  if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
    gwenesis_H32upscaler = gwenesis_H32upscaler == 0 ? 1 : 0;
  }

    if (gwenesis_H32upscaler == 0) strcpy(option->value, curr_lang->s_md_Option_OFF);
    if (gwenesis_H32upscaler == 1) strcpy(option->value, curr_lang->s_md_Option_ON);

    return event == ODROID_DIALOG_ENTER;
}

static bool gwenesis_submenu_sync_mode(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
  if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
    gwenesis_vsync_mode = gwenesis_vsync_mode == 0 ? 1 : 0;
  }

    if (gwenesis_vsync_mode == 0) strcpy(option->value, curr_lang->s_md_Synchro_Audio);
    if (gwenesis_vsync_mode == 1) strcpy(option->value, curr_lang->s_md_Synchro_Vsync);

    return event == ODROID_DIALOG_ENTER;
}

static char debug_bar_str[2];
static char VideoUpscaler_str[2];
static char gwenesis_sync_mode_str[8];
#endif


static char AudioFilter_str[2];
void gwenesis_save_local_data(void) {
  SaveState *state = saveGwenesisStateOpenForWrite("gwenesis");


  saveGwenesisStateSetBuffer(state, "ABCkeys_value", &ABCkeys_value, sizeof(int));
  saveGwenesisStateSet(state, "PAD_A_def", PAD_A_def);
  saveGwenesisStateSet(state, "PAD_B_def", PAD_B_def);
  saveGwenesisStateSet(state, "PAD_C_def", PAD_C_def);
  
    // Audio low pass filter "ON or OFF"
  saveGwenesisStateSetBuffer(state, "AudioFilter_str", &AudioFilter_str, sizeof(int));
  saveGwenesisStateSet(state, "gwenesis_lpfilter",gwenesis_lpfilter);
}

void gwenesis_load_local_data(void) {
  SaveState *state = saveGwenesisStateOpenForWrite("gwenesis");

  // A-B-C keys mapping
  saveGwenesisStateGetBuffer(state, "ABCkeys_value", &ABCkeys_value, sizeof(int));
  PAD_A_def = saveGwenesisStateGet(state, "PAD_A_def");
  PAD_B_def = saveGwenesisStateGet(state, "PAD_B_def");
  PAD_C_def = saveGwenesisStateGet(state, "PAD_C_def");
  
  // Audio low pass filter "ON or OFF"
  saveGwenesisStateGetBuffer(state, "AudioFilter_str", &AudioFilter_str, sizeof(int));
  gwenesis_lpfilter = saveGwenesisStateGet(state, "gwenesis_lpfilter");
}

static bool gwenesis_system_SaveState(char *pathName) {
  int size = 0;
  printf("Saving state...\n");
#if OFF_SAVESTATE==1
  if (strcmp(pathName,"1") == 0) {
    // Save in common save slot (during a power off)
    size = saveGwenesisState((unsigned char *)&__OFFSAVEFLASH_START__,ACTIVE_FILE->save_size);
  } else {
#endif
    size = saveGwenesisState((unsigned char *)ACTIVE_FILE->save_address,ACTIVE_FILE->save_size);
#if OFF_SAVESTATE==1
  }
#endif
  printf("Saved state size:%d\n", size);
  return true;
}

static bool gwenesis_system_LoadState(char *pathName) {
  printf("Loading state...\n");
  return loadGwenesisState((unsigned char *)ACTIVE_FILE->save_address);
}

/* Main */
int app_main_gwenesis(uint8_t load_state, uint8_t start_paused, uint8_t save_slot)
{

    printf("Genesis start\n");
    odroid_system_init(APPID_MD, GWENESIS_AUDIO_FREQ_NTSC);
    odroid_system_emu_init(&gwenesis_system_LoadState, &gwenesis_system_SaveState, NULL);
   // rg_app_desc_t *app = odroid_system_get_app();

    common_emu_state.frame_time_10us = (uint16_t)(100000 / 60.0 + 0.5f);

    if (start_paused) {
      common_emu_state.pause_after_frames = 2;
      odroid_audio_mute(true);
    } else {
       common_emu_state.pause_after_frames = 0;
    }

    /*** load ROM  */
    load_rom_from_flash();
    load_cartridge();

    gwenesis_system_init();

    power_on();
    reset_emulation();

    /* clear the screen before rendering */
    memset(lcd_get_inactive_buffer(), 0, 320 * 240 * 2);
    memset(lcd_get_active_buffer(), 0, 320 * 240 * 2);

    unsigned short *screen = 0;

    screen = lcd_get_active_buffer();
    gwenesis_vdp_set_buffer(&screen[0]);
    extern unsigned char gwenesis_vdp_regs[0x20];
    extern unsigned int gwenesis_vdp_status;
    extern unsigned int screen_width, screen_height;
    static int hori_screen_offset, vert_screen_offset;
    int hint_counter;
    extern int hint_pending;
    volatile unsigned int current_frame;
    
    if (load_state) {
#if OFF_SAVESTATE==1
      if (save_slot == 1) {
        // Load from common save slot if needed
        loadGwenesisState((unsigned char *)&__OFFSAVEFLASH_START__);
      } else {
#endif
        loadGwenesisState((unsigned char *)ACTIVE_FILE->save_address);
#if OFF_SAVESTATE==1
      }
#endif
    }

    /* Start at the same time DMAs audio & video */
    /* Audio period and Video period are the same (almost at least 1 hour) */
    lcd_wait_for_vblank();
    gwenesis_sound_start();
    gwenesis_audio_pll_sync = 1;

    // gwenesis_init_position = 0xFFFF & lcd_get_pixel_position();
    while (true) {

      /* capture the frame processed by the LCD controller */
      current_frame = frame_counter;

      /* clear DWT counter used to monitor performances */
      clear_dwt_cycles();

      /* reset watchdog */
      wdog_refresh();

      /* hardware keys */
      odroid_input_read_gamepad(&joystick);

      /* SWAP TIME & PAUSE/SET for MARIO G&W device */
      #if GNW_TARGET_MARIO != 0
      unsigned int key_state = joystick.values[ODROID_INPUT_VOLUME];
      joystick.values[ODROID_INPUT_VOLUME] = joystick.values[ODROID_INPUT_SELECT];
      joystick.values[ODROID_INPUT_SELECT] = key_state;
      #endif

    odroid_dialog_choice_t options[] = {
        {301, curr_lang->s_md_keydefine, ABCkeys_str, 1, &gwenesis_submenu_setABC},
        {302, curr_lang->s_md_AudioFilter, AudioFilter_str, 1, &gwenesis_submenu_setAudioFilter},
#if ENABLE_DEBUG_OPTIONS != 0
        {303, curr_lang->s_md_VideoUpscaler, VideoUpscaler_str, 1, &gwenesis_submenu_setVideoUpscaler},
        {304, curr_lang->s_md_Synchro, gwenesis_sync_mode_str, 1, &gwenesis_submenu_sync_mode},
        {310, curr_lang->s_md_Debug_bar, debug_bar_str, 1, &gwenesis_submenu_debug_bar},
#endif
        //  {320, "+GameGenie", gwenesis_GameGenie_str, 0, &gwenesis_submenu_GameGenie},
        //  {330, "-GameGenie", gwenesis_GameGenie_reverse_str, 0, &gwenesis_submenu_GameGenie_reverse},

        ODROID_DIALOG_CHOICE_LAST};

    hint_counter = gwenesis_vdp_regs[10];

      screen_height = REG1_PAL ? 240 : 224;
      screen_width = REG12_MODE_H40 ? 320 : 256;
      lines_per_frame = REG1_PAL ? LINES_PER_FRAME_PAL : LINES_PER_FRAME_NTSC;
      vert_screen_offset = REG1_PAL ? 0 : 320 * (240 - 224) / 2;

      hori_screen_offset = 0; //REG12_MODE_H40 ? 0 : (320 - 256) / 2;

    void _repaint()
    {
        screen = lcd_get_active_buffer();
        gwenesis_vdp_set_buffer(&screen[vert_screen_offset + hori_screen_offset]);
        for (int l = 0; l < lines_per_frame; l++)
        {
            gwenesis_vdp_render_line(l); /* render scan_line */
        }
        common_ingame_overlay();
    }

    common_emu_input_loop(&joystick, options, &_repaint);
    common_emu_input_loop_handle_turbo(&joystick);

    // bool drawFrame =
    common_emu_frame_loop();

      /* Eumulator loop */
      screen = lcd_get_active_buffer();
      gwenesis_vdp_set_buffer(&screen[vert_screen_offset + hori_screen_offset]);

      gwenesis_vdp_render_config();

      /* Reset the difference clocks and audio index */
      system_clock = 0;
      zclk = 0;

      ym2612_clock = 0;
      ym2612_index = 0;

      sn76489_clock = 0;
      sn76489_index = 0;

    scan_line=0;

    while (scan_line < lines_per_frame) {

        /* CPUs */
        m68k_run(system_clock + VDP_CYCLES_PER_LINE);
        z80_run(system_clock + VDP_CYCLES_PER_LINE);

        /* Audio */
        /*  GWENESIS_AUDIO_ACCURATE:
        *    =1 : cycle accurate mode. audio is refreshed when CPUs are performing a R/W access
        *    =0 : line  accurate mode. audio is refreshed every lines.
        */
       if (GWENESIS_AUDIO_ACCURATE == 0) {
          gwenesis_SN76489_run(system_clock + VDP_CYCLES_PER_LINE);
          ym2612_run(system_clock + VDP_CYCLES_PER_LINE);
        }

        /* Video */
        if (drawFrame)
          gwenesis_vdp_render_line(scan_line); /* render scan_line */

        // On these lines, the line counter interrupt is reloaded
        if ((scan_line == 0) || (scan_line > screen_height)) {
          //  if (REG0_LINE_INTERRUPT != 0)
          //    printf("HINTERRUPT counter reloaded: (scan_line: %d, new
          //    counter: %d)\n", scan_line, REG10_LINE_COUNTER);
          hint_counter = REG10_LINE_COUNTER;
        }

        // interrupt line counter
        if (--hint_counter < 0) {
          if ((REG0_LINE_INTERRUPT != 0) && (scan_line <= screen_height)) {
            hint_pending = 1;
            // printf("Line int pending %d\n",scan_line);
            if ((gwenesis_vdp_status & STATUS_VIRQPENDING) == 0)
              m68k_update_irq(4);
          }
          hint_counter = REG10_LINE_COUNTER;
        }

        scan_line++;

        // vblank begin at the end of last rendered line
        if (scan_line == screen_height) {

          if (REG1_VBLANK_INTERRUPT != 0) {
            // printf("IRQ VBLANK\n");
            gwenesis_vdp_status |= STATUS_VIRQPENDING;
            m68k_set_irq(6);
          }
          z80_irq_line(1);
        }
        if (scan_line == (screen_height + 1)) {

          z80_irq_line(0);
        }

        system_clock += VDP_CYCLES_PER_LINE;
    }

      /* Audio
      * synchronize YM2612 and SN76489 to system_clock
      * it completes the missing audio sample for accurate audio mode
      */
      if (GWENESIS_AUDIO_ACCURATE == 1) {
        gwenesis_SN76489_run(system_clock);
        ym2612_run(system_clock);
      }

      // reset m68k cycles to the begin of next frame cycle
      m68k.cycles -= system_clock;

      /* copy audio samples for DMA */
      gwenesis_sound_submit();

      if (gwenesis_show_debug_bar == 1)
        gwenesis_debug_bar();

      if (drawFrame)
        common_ingame_overlay();

      end_cycles = get_dwt_cycles();

      /* VSYNC mode */
      if (gwenesis_vsync_mode) {
        /* Check if we are still in the same frame as at the beginning of the
         * loop if it's different we are in overflow : skip next frame using
         * (drawFrame = 0) otherwise we are in the same frame. wait the end of
         * frame.
         */
        if (current_frame != frame_counter) {
          overflow_count++;
          drawFrame = 0;

        } else {
          lcd_swap();
          drawFrame = 1;
          lcd_sleep_while_swap_pending();
        }

        /* AUDIO SYNC mode */
        /* default as Audio/Video are synchronized */
      } else {

        lcd_swap();

        common_emu_state.pause_frames = 0;
        common_emu_state.skip_frames = 0;
        common_emu_sound_sync(gwenesis_show_debug_bar);
      }
      // Get current line LCD position to check A/V synchronization
      gwenesis_lcd_current_line = 0xFFFF & lcd_get_pixel_position();

      /*  SYNC A/V */
      if (gwenesis_lcd_current_line > GWENESIS_AUDIOSYNC_START_LCD_LINE)
        gwenesis_audio_pll_stepup();
      if (gwenesis_lcd_current_line < GWENESIS_AUDIOSYNC_START_LCD_LINE)
        gwenesis_audio_pll_stepdown();
      if (gwenesis_lcd_current_line == GWENESIS_AUDIOSYNC_START_LCD_LINE)
        gwenesis_audio_pll_center();

      /* get how cycles have been spent inside this loop */
      loop_cycles = get_dwt_cycles();

    } // end of loop
}
#endif
