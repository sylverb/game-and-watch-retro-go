#include "build/config.h"

#ifdef ENABLE_EMULATOR_A7800
#include <odroid_system.h>

#include <assert.h>
#include "gw_lcd.h"
#include "gw_linker.h"
#include "gw_buttons.h"
#include "gw_malloc.h"
#include "rom_manager.h"
#include "common.h"
#include "lz4_depack.h"
#include "miniz.h"
#include "lzma.h"
#include "appid.h"
#include "bilinear.h"
#include "rg_i18n.h"

#include "Bios.h"
#include "Cartridge.h"
#include "Database.h"
#include "Maria.h"
#include "Palette.h"
#include "Pokey.h"
#include "Region.h"
#include "ProSystem.h"
#include "Tia.h"
#include "Memory.h"

#define ROM_BUFF_LENGTH 131200 // 128kB + header
// Memory to handle compressed roms
static uint8_t rom_memory[ROM_BUFF_LENGTH];

static int videoWidth                  = 320;
static int videoHeight                 = 240;

static uint16_t display_palette16[256] = {0};
static uint8_t keyboard_data[17]       = {0};

static uint8_t *pokeyMixBuffer         = NULL;

static uint8_t save_buffer[32796];

static bool LoadState(char *pathName) {
    if ((ACTIVE_FILE->save_address[0] == '7') &&
        (ACTIVE_FILE->save_address[1] == '8') &&
        (ACTIVE_FILE->save_address[2] == '0') &&
        (ACTIVE_FILE->save_address[3] == '0')) {
            printf("LoadState OK\n");
            prosystem_Load((const char *)ACTIVE_FILE->save_address+4);
        }
    return 0;
}

#if OFF_SAVESTATE==1
#pragma GCC diagnostic ignored "-Warray-bounds"
static bool LoadA7800State(int8_t *srcBuffer) {
    if ((srcBuffer[0] == '7') &&
        (srcBuffer[1] == '8') &&
        (srcBuffer[2] == '0') &&
        (srcBuffer[3] == '0')) {
            printf("LoadState OK\n");
            prosystem_Load((const char *)(srcBuffer+4));
        }
    return 0;
}
#pragma GCC diagnostic pop
#endif


static bool SaveState(char *pathName) {
    save_buffer[0] = '7';
    save_buffer[1] = '8';
    save_buffer[2] = '0';
    save_buffer[3] = '0';
    prosystem_Save((char *)save_buffer+4, false);
    uint32_t size = 32833;
#if OFF_SAVESTATE==1
    if (strcmp(pathName,"1") == 0) {
        // Save in common save slot (during a power off)
        store_save((const uint8_t *)&__OFFSAVEFLASH_START__, save_buffer, size);
    } else {
#endif
        store_save(ACTIVE_FILE->save_address, save_buffer, size);
#if OFF_SAVESTATE==1
    }
#endif
    return 0;
}

static size_t getromdata(unsigned char **data) {
    /* src pointer to the ROM data in the external flash (raw or LZ4) */
    const unsigned char *src = ROM_DATA;
    unsigned char *dest = (unsigned char *)rom_memory;

    if(strcmp(ROM_EXT, "lzma") == 0){
        size_t n_decomp_bytes;
        n_decomp_bytes = lzma_inflate(dest, ROM_BUFF_LENGTH, src, ROM_DATA_LENGTH);
        *data = dest;
        return n_decomp_bytes;
    } else {
        *data = (unsigned char *)ROM_DATA;
        return ROM_DATA_LENGTH;
    }
}

#define BLIT_VIDEO_BUFFER(typename_t, src, palette, width, height, pitch, dst) \
   {                                                                           \
      typename_t *surface = (typename_t*)dst;                                  \
      uint32_t x, y;                                                           \
                                                                               \
      for(y = 0; y < height; y++)                                              \
      {                                                                        \
         typename_t *surface_ptr = surface;                                    \
         const uint8_t *src_ptr  = src;                                        \
                                                                               \
         for(x = 0; x < width; x++)                                            \
            *(surface_ptr++) = *(palette + *(src_ptr++));                      \
                                                                               \
         surface += pitch;                                                     \
         src     += width;                                                     \
      }                                                                        \
   }

static void display_ResetPalette(void)
{
   unsigned index;

   for(index = 0; index < 256; index++)
   {
      uint32_t r = palette_data[(index * 3) + 0] << 16;
      uint32_t g = palette_data[(index * 3) + 1] << 8;
      uint32_t b = palette_data[(index * 3) + 2];
      display_palette16[index] = ((r & 0xF80000) >> 8) |
                                 ((g & 0x00F800) >> 5) |
                                 ((b & 0x0000F8) >> 3);
   }
}

void update_joystick(odroid_gamepad_state_t *joystick) {
   // ----------------------------------------------------------------------------
   // SetInput
   // +----------+--------------+-------------------------------------------------
   // | Offset   | Controller   | Control
   // +----------+--------------+-------------------------------------------------
   // | 00       | Joystick 1   | Right
   // | 01       | Joystick 1   | Left
   // | 02       | Joystick 1   | Down
   // | 03       | Joystick 1   | Up
   // | 04       | Joystick 1   | Button 1
   // | 05       | Joystick 1   | Button 2
   // | 06       | Joystick 2   | Right
   // | 07       | Joystick 2   | Left
   // | 08       | Joystick 2   | Down
   // | 09       | Joystick 2   | Up
   // | 10       | Joystick 2   | Button 1
   // | 11       | Joystick 2   | Button 2
   // | 12       | Console      | Reset
   // | 13       | Console      | Select
   // | 14       | Console      | Pause
   // | 15       | Console      | Left Difficulty
   // | 16       | Console      | Right Difficulty
   // +----------+--------------+-------------------------------------------------
    keyboard_data[0]  = joystick->values[ODROID_INPUT_RIGHT] ? 1 : 0;
    keyboard_data[1]  = joystick->values[ODROID_INPUT_LEFT]  ? 1 : 0;
    keyboard_data[2]  = joystick->values[ODROID_INPUT_DOWN]  ? 1 : 0;
    keyboard_data[3]  = joystick->values[ODROID_INPUT_UP]    ? 1 : 0;
    keyboard_data[4]  = joystick->values[ODROID_INPUT_B]     ? 1 : 0;
    keyboard_data[5]  = joystick->values[ODROID_INPUT_A]     ? 1 : 0;
    if (joystick->values[ODROID_INPUT_SELECT] || joystick->values[ODROID_INPUT_Y]) {
        keyboard_data[13] = 1;
    } else {
        keyboard_data[13] = 0;
    }
    if (joystick->values[ODROID_INPUT_START] || joystick->values[ODROID_INPUT_X]) {
        keyboard_data[14] = 1;
    } else {
        keyboard_data[14] = 0;
    }
    keyboard_data[15] = 0;

}

static void sound_store(int16_t *audio_out_buf)
{
    uint8_t *tia_samples_buf = tia_buffer;
    size_t j;

    /* Mix in sound generated by POKEY chip
    * (Ballblazer, Commando, various homebrew and hacks) */
    if(cartridge_pokey)
    {
        uint8_t *pokey_samples_buf = pokey_buffer;
        uint8_t *pokey_mix_buf     = pokeyMixBuffer;

        /* Copy samples to pokeyMixBuffer */
        for(j = 0; j < tia_size; j++)
            *(pokey_mix_buf++) = (*(tia_samples_buf++) + *(pokey_samples_buf++)) >> 1;

        /* pokeyMixBuffer 'replaces' tia_buffer */
        tia_samples_buf = pokeyMixBuffer;
    }

    // MUST shift with at least 1 place, or it will brownout.
    uint8_t volume = odroid_audio_volume_get();
    int32_t factor = volume_tbl[volume];

    if (audio_mute || (volume == ODROID_AUDIO_VOLUME_MIN)) {
        // mute
        for (int i = 0; i < tia_size; i++) {
            audio_out_buf[i] = 0;
        }
        return;
    }

    // Write to DMA buffer and lower the volume accordingly
    for (int i = 0; i < tia_size; i++) {
        int32_t sample = *(tia_samples_buf++) << 8;
        audio_out_buf[i] = (sample * factor) >> 8;
    }
}

int app_main_a7800(uint8_t load_state, uint8_t start_paused, uint8_t save_slot)
{
    size_t offset;
    const uint8_t *buffer = NULL;
    uint32_t rom_length = 0;
    uint8_t *rom_ptr = NULL;

    static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
    odroid_gamepad_state_t joystick;
    odroid_dialog_choice_t options[] = {
        ODROID_DIALOG_CHOICE_LAST
    };

    if (start_paused) {
        common_emu_state.pause_after_frames = 2;
    } else {
        common_emu_state.pause_after_frames = 0;
    }

    pokeyMixBuffer = (uint8_t*)itc_malloc(TIA_BUFFER_SIZE * sizeof(uint8_t));

    memset(keyboard_data, 0, sizeof(keyboard_data));

    /* Difficulty switches: 
     * Left position = (B)eginner, Right position = (A)dvanced
     * Left difficulty switch defaults to left position, "(B)eginner"
     */
    keyboard_data[15] = 1;
    /* Right difficulty switch defaults to right position,
     * "(A)dvanced", which fixes Tower Toppler
     */
    keyboard_data[16] = 0;

    rom_length = getromdata(&rom_ptr);

    if (cartridge_Load(true,rom_ptr,rom_length)) {
        bios_enabled = false; // Bios not loaded
    } else {
        // Rom not supported
        return 0;
    }
    display_ResetPalette();
    database_Load(cartridge_digest);
    prosystem_Reset();

    // Use detected system frequency
    common_emu_state.frame_time_10us = (uint16_t)(100000 / prosystem_frequency + 0.5f);

    // Black background
    memset(framebuffer1, 0, sizeof(framebuffer1));
    memset(framebuffer2, 0, sizeof(framebuffer2));

    odroid_system_init(APPID_A7800, tia_size*prosystem_frequency); // 31200Hz for PAL, 31440Hz for NTSC
    odroid_system_emu_init(&LoadState, &SaveState, NULL);

    // Init Sound
    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, 2*tia_size);

    if (load_state) {
#if OFF_SAVESTATE==1
        if (save_slot == 1) {
            // Load from common save slot if needed
            LoadA7800State((int8_t *)&__OFFSAVEFLASH_START__);
        } else {
#endif
            LoadState("");
#if OFF_SAVESTATE==1
        }
#endif
    }
    while (1)
    {
        wdog_refresh();
        common_emu_frame_loop();
        odroid_input_read_gamepad(&joystick);
        common_emu_input_loop(&joystick, options);
        
        uint8_t turbo_buttons = odroid_settings_turbo_buttons_get();
        bool turbo_a = (joystick.values[ODROID_INPUT_A] && (turbo_buttons & 1));
        bool turbo_b = (joystick.values[ODROID_INPUT_B] && (turbo_buttons & 2));
        bool turbo_button = odroid_button_turbos();
        if (turbo_a)
            joystick.values[ODROID_INPUT_A] = turbo_button;
        if (turbo_b)
            joystick.values[ODROID_INPUT_B] = !turbo_button;

        update_joystick(&joystick);

        prosystem_ExecuteFrame(keyboard_data);

        videoWidth  = Rect_GetLength(&maria_visibleArea);
        videoHeight = Rect_GetHeight(&maria_visibleArea);
        buffer      = maria_surface + ((maria_visibleArea.top - maria_displayArea.top) * Rect_GetLength(&maria_visibleArea));

        BLIT_VIDEO_BUFFER(uint16_t, buffer, display_palette16, 320, 240, 320, lcd_get_active_buffer());

        offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : tia_size;

        sound_store(&audiobuffer_dma[offset]);

        common_ingame_overlay();
        lcd_swap();
        if(!common_emu_state.skip_frames){
            for(uint8_t p = 0; p < common_emu_state.pause_frames + 1; p++) {
                while (dma_state == last_dma_state) {
                    cpumon_sleep();
                }
                last_dma_state = dma_state;
            }
        }
    }

    return 0;
}

#endif