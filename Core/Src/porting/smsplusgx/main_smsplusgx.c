#include "build/config.h"

#if defined(ENABLE_EMULATOR_SMS) || defined(ENABLE_EMULATOR_GG) || defined(ENABLE_EMULATOR_COL) || defined(ENABLE_EMULATOR_SG1000)
#include <odroid_system.h>
#include <string.h>

#include "main.h"
#include "bilinear.h"
#include "gw_lcd.h"
#include "gw_flash.h"
#include "gw_linker.h"
#include "gw_buttons.h"
#include "shared.h"
#include "rom_manager.h"
#include "common.h"
#include "main_smsplusgx.h"
#include "appid.h"
#include "rg_i18n.h"
#include "lzma.h"
#include "gw_malloc.h"
#include "filesystem.h"

#define SMS_WIDTH 256
#define SMS_HEIGHT 192

#define COL_WIDTH   272
#define COL_HEIGHT  208

#define GG_WIDTH 160
#define GG_HEIGHT 144

#define PIXEL_MASK 0x1F
#define PAL_SHIFT_MASK 0x80

#define AUDIO_BUFFER_LENGTH_SMS (AUDIO_SAMPLE_RATE / 60)
#define AUDIO_BUFFER_LENGTH_DMA_SMS ((2 * AUDIO_SAMPLE_RATE) / 60)

static uint16_t palette[32];
static uint32_t palette_spaced[32];


static bool consoleIsGG  = false;
static bool consoleIsSMS = false;
static bool consoleIsCOL = false;
static bool consoleIsSG  = false;

// TODO: Move to lcd.c/h
extern LTDC_HandleTypeDef hltdc;

void set_config();
unsigned int crc32_le(unsigned int crc, unsigned char const * buf,unsigned int len);

// --- MAIN
#define SMSROM_RAM_BUFFER_LENGTH (60*1024)
static uint8_t *ROMinRAM_DATA;

static const uint8_t IMG_DISKETTE[] = {
    0x00, 0x00, 0x00, 0x3F, 0xFF, 0xE0, 0x7C, 0x00,
    0x70, 0x7C, 0x03, 0x78, 0x7C, 0x03, 0x7C, 0x7C,
    0x03, 0x7E, 0x7C, 0x00, 0x7E, 0x7F, 0xFF, 0xFE,
    0x7F, 0xFF, 0xFE, 0x7F, 0xFF, 0xFE, 0x7F, 0xFF,
    0xFE, 0x7F, 0xFF, 0xFE, 0x7F, 0xFF, 0xFE, 0x7E,
    0x00, 0x7E, 0x7C, 0x00, 0x3E, 0x7C, 0x00, 0x3E,
    0x7D, 0xFF, 0xBE, 0x7C, 0x00, 0x3E, 0x7C, 0x00,
    0x3E, 0x7D, 0xFF, 0xBE, 0x7C, 0x00, 0x3E, 0x7C,
    0x00, 0x3E, 0x3F, 0xFF, 0xFC, 0x00, 0x00, 0x00,
};


static int
load_rom_from_flash(uint8_t emu_engine)
{
    static uint8 sram[0x8000];
    /* check if it's compressed */

    if (strcmp(ROM_EXT, "lzma") == 0)
    {
        /* it can fit in ITC RAM */
        if ((emu_engine == SMSPLUSGX_ENGINE_COLECO) || (emu_engine == SMSPLUSGX_ENGINE_SG1000))
        {
            size_t n_decomp_bytes;
            ROMinRAM_DATA = itc_malloc(SMSROM_RAM_BUFFER_LENGTH);
            n_decomp_bytes = lzma_inflate((uint8 *)ROMinRAM_DATA, SMSROM_RAM_BUFFER_LENGTH, (uint8 *)ROM_DATA, ROM_DATA_LENGTH);
            cart.rom = (uint8 *)ROMinRAM_DATA;
            cart.size = (uint32_t)n_decomp_bytes;
        }

        /* it can't fit in ITC RAM */
        else
        {


        //   assert ( (&__CACHEFLASH_END__ - &__CACHEFLASH_START__) > 0);

            /* check header  */
            //assert(memcmp((uint8 *)ROM_DATA, "SMS+", 4) == 0);

            unsigned int nb_banks = 0;
            unsigned int lzma_bank_size = 0;
            unsigned int lzma_bank_offset = 0;
            unsigned int uncompressed_rom_size = 0;

            memcpy(&nb_banks, &ROM_DATA[4], sizeof(nb_banks));

            lzma_bank_offset = 4 + 4 + 4 * nb_banks;

            for (int i = 0; i < nb_banks; i++)
            {
                wdog_refresh();
                memcpy(&lzma_bank_size, &ROM_DATA[8 + 4 * i], sizeof(lzma_bank_size));
                lcd_clear_inactive_buffer();

                uint16_t *dest = lcd_get_inactive_buffer();


                /* uncompressed in lcd framebuffer */
                size_t n_decomp_bytes;
                n_decomp_bytes = lzma_inflate((uint8 *)lcd_get_active_buffer(), 2 * 320 * 240, &ROM_DATA[lzma_bank_offset], lzma_bank_size);

                //assert (  (&__CACHEFLASH_END__ - &__CACHEFLASH_START__) >= ( (uint32_t)n_decomp_bytes + uncompressed_rom_size) );

                int diff = memcmp((void *)(&__CACHEFLASH_START__ + uncompressed_rom_size), (uint8 *)lcd_get_active_buffer(), n_decomp_bytes);
                if (diff != 0)
                {
                wdog_refresh(); 
                OSPI_DisableMemoryMappedMode();

                    /* display diskette during flash erase */
                    uint16_t idx = 0;
                    for(uint8_t i=0; i < 24; i++) {
                        for(uint8_t j=0; j < 24; j++) {
                            if(IMG_DISKETTE[idx / 8] & (1 << (7 - idx % 8))){
                                dest[286 + j +  GW_LCD_WIDTH * (10 + i)] = 0xFFFF;
                            }
                            idx++;
                        }
                    }

                    /* erase the trunk */
                    OSPI_EraseSync((&__CACHEFLASH_START__ - &__EXTFLASH_BASE__)+uncompressed_rom_size, (uint32_t)n_decomp_bytes);

                    /* erase diskette during flash program */
                    for (short y = 0; y < 24; y++) {
                    uint16_t *dest_row = &dest[(y + 10) * GW_LCD_WIDTH + 286];
                    memset(dest_row,0x0 , 24 * sizeof(uint16_t));
                    }

                    /* program the trunk */
                    wdog_refresh();
                    OSPI_Program((&__CACHEFLASH_START__ - &__EXTFLASH_BASE__)+uncompressed_rom_size, (uint8 *)lcd_get_active_buffer(), (uint32_t)n_decomp_bytes);

                    OSPI_EnableMemoryMappedMode();
                    wdog_refresh();
                }

                lzma_bank_offset += lzma_bank_size;
                uncompressed_rom_size += (uint32_t)n_decomp_bytes;

            }

            /* set the rom pointer and size */
            cart.rom = &__CACHEFLASH_START__; 
            cart.size = uncompressed_rom_size; 
        }
    }
    else
    {
        cart.rom = (uint8 *)ROM_DATA;
        cart.size = ROM_DATA_LENGTH;
    }

    cart.sram = sram;
    cart.pages = cart.size / 0x4000;
    cart.crc = crc32_le(0, cart.rom, cart.size);
    cart.loaded = 1;

    if (emu_engine == SMSPLUSGX_ENGINE_COLECO)
    {
        option.console = 6; // Force Coleco
    }
    else if (emu_engine == SMSPLUSGX_ENGINE_SG1000)
    {
        option.console = 5; // Force SG1000
    }
    set_config();
    printf("%s: OK. cart.size=%d, cart.crc=%#010lx\n", __func__, (int)cart.size, cart.crc);

    if (sms.console == CONSOLE_COLECO)
    {
        extern const unsigned char ColecoVision_BIOS[];
        coleco.rom = (uint8*)ColecoVision_BIOS;
    }
    return 1;
}

extern uint32 glob_bp_lut[0x10000];
#define SAVE_STATE_BUFFER_SIZE (60 * 1024)

static bool SaveState(char *savePathName, char *sramPathName)
{
    uint8_t *state_save_buffer = (uint8_t *)glob_bp_lut;
    system_save_state(state_save_buffer);

    fs_file_t *file;
    file = fs_open(savePathName, FS_WRITE, FS_COMPRESS);
    fs_write(file, state_save_buffer, SAVE_STATE_BUFFER_SIZE);
    fs_close(file);

    /* restore the contents of _bp_lut */
    render_init();
    return false;
}

static bool LoadState(char *savePathName, char *sramPathName)
{
    uint8_t *state_save_buffer = (uint8_t *)glob_bp_lut;

    fs_file_t *file;
    file = fs_open(savePathName, FS_READ, FS_COMPRESS);
    fs_read(file, state_save_buffer, SAVE_STATE_BUFFER_SIZE);
    fs_close(file);

    system_load_state((void *)state_save_buffer);
    /* restore the contents of _bp_lut */
    render_init();

    return true;
}

static uint8_t fb_buffer[COL_WIDTH*COL_HEIGHT];

#define CONV(_b0) ((0b11111000000000000000000000&_b0)>>10) | ((0b000001111110000000000&_b0)>>5) | ((0b0000000000011111&_b0));

static void
blit_gg(bitmap_t *bmp, uint16_t *framebuffer) {	/* 160 x 144 -> 320 x 240 */
    int y_src = 0;
    int y_dst = 0;
    for (; y_src < bmp->viewport.h; y_src += 3, y_dst += 5) {
        int x_src = 0;
        int x_dst = 0;
        for (; x_src < bmp->viewport.w; x_src += 1, x_dst += 2) {
            uint8_t *src_col = &bmp->data[(y_src + bmp->viewport.y) * bmp->pitch + x_src + bmp->viewport.x];
            uint32_t b0 = palette_spaced[src_col[bmp->pitch * 0] & 0x1f];
            uint32_t b1 = palette_spaced[src_col[bmp->pitch * 1] & 0x1f];
            uint32_t b2 = palette_spaced[src_col[bmp->pitch * 2] & 0x1f];

            framebuffer[((y_dst + 0) * WIDTH) + x_dst] = CONV(b0);
            framebuffer[((y_dst + 1) * WIDTH) + x_dst] = CONV((b0+b1)>>1);
            framebuffer[((y_dst + 2) * WIDTH) + x_dst] = CONV(b1);
            framebuffer[((y_dst + 3) * WIDTH) + x_dst] = CONV((b1+b2)>>1);
            framebuffer[((y_dst + 4) * WIDTH) + x_dst] = CONV(b2);

            framebuffer[((y_dst + 0) * WIDTH) + x_dst + 1] = CONV(b0);
            framebuffer[((y_dst + 1) * WIDTH) + x_dst + 1] = CONV((b0+b1)>>1);
            framebuffer[((y_dst + 2) * WIDTH) + x_dst + 1] = CONV(b1);
            framebuffer[((y_dst + 3) * WIDTH) + x_dst + 1] = CONV((b1+b2)>>1);
            framebuffer[((y_dst + 4) * WIDTH) + x_dst + 1] = CONV(b2);
        }
    }
}

static void
blit_sms(bitmap_t *bmp, uint16_t *framebuffer) {	/* 256 x 192 -> 320 x 230 */
    const int hpad = (WIDTH - 320) / 2;
    const int vpad = (HEIGHT - 230) / 2;

    uint32_t block[6 * 5]; /* workspace: 5 rows, 6 pixels wide */

    int y_src = 1;         /* 1st and last row of 192 will not be scaled */
    int y_dst = 1 + vpad;  /* the remaining 190 are scaled */
    for (; y_src < bmp->viewport.h - 1; y_src += 5, y_dst += 6) {
        int x_src = 0;
        int x_dst = hpad;
        for (; x_src < bmp->viewport.w - 1; x_src += 5, x_dst += 6) {
            for (int y = 0; y < 5; y++) {
                uint8_t *src_row = &bmp->data[(y_src + y + bmp->viewport.y) * bmp->pitch];
                uint32_t b0 = palette_spaced[src_row[x_src + 0] & 0x1f];
                uint32_t b1 = palette_spaced[src_row[x_src + 1] & 0x1f];
                uint32_t b2 = palette_spaced[src_row[x_src + 2] & 0x1f];
                uint32_t b3 = palette_spaced[src_row[x_src + 3] & 0x1f];
                uint32_t b4 = palette_spaced[src_row[x_src + 4] & 0x1f];

                block[(y * 6) + 0] = b0;
                block[(y * 6) + 1] = (b0+b1+b1+b1)>>2;
                block[(y * 6) + 2] = (b1+b2)>>1;
                block[(y * 6) + 3] = (b2+b3)>>1;
                block[(y * 6) + 4] = (b3+b3+b3+b4)>>2;
                block[(y * 6) + 5] = b4;
            }

            for (int x = 0; x < 6; x++) {
                uint32_t b0 = block[(0 * 6) + x];
                uint32_t b1 = block[(1 * 6) + x];
                uint32_t b2 = block[(2 * 6) + x];
                uint32_t b3 = block[(3 * 6) + x];
                uint32_t b4 = block[(4 * 6) + x];

                framebuffer[((y_dst + 0) * WIDTH) + x + x_dst] = CONV(b0);
                framebuffer[((y_dst + 1) * WIDTH) + x + x_dst] = CONV((b0+b1+b1+b1)>>2);
                framebuffer[((y_dst + 2) * WIDTH) + x + x_dst] = CONV((b1+b2)>>1);
                framebuffer[((y_dst + 3) * WIDTH) + x + x_dst] = CONV((b2+b3)>>1);
                framebuffer[((y_dst + 4) * WIDTH) + x + x_dst] = CONV((b3+b3+b3+b4)>>2);
                framebuffer[((y_dst + 5) * WIDTH) + x + x_dst] = CONV(b4);
            }
        }

        /* Last column, x_src = 255 */
        uint8_t *src_col = &bmp->data[(y_src + bmp->viewport.y) * bmp->pitch + x_src];
        uint32_t b0 = palette_spaced[src_col[bmp->pitch * 0] & 0x1f];
        uint32_t b1 = palette_spaced[src_col[bmp->pitch * 1] & 0x1f];
        uint32_t b2 = palette_spaced[src_col[bmp->pitch * 2] & 0x1f];
        uint32_t b3 = palette_spaced[src_col[bmp->pitch * 3] & 0x1f];
        uint32_t b4 = palette_spaced[src_col[bmp->pitch * 4] & 0x1f];

        framebuffer[((y_dst + 0) * WIDTH) + x_dst] = CONV(b0);
        framebuffer[((y_dst + 1) * WIDTH) + x_dst] = CONV((b0+b1+b1+b1)>>2);
        framebuffer[((y_dst + 2) * WIDTH) + x_dst] = CONV((b1+b2)>>1);
        framebuffer[((y_dst + 3) * WIDTH) + x_dst] = CONV((b2+b3)>>1);
        framebuffer[((y_dst + 4) * WIDTH) + x_dst] = CONV((b3+b3+b3+b4)>>2);
        framebuffer[((y_dst + 5) * WIDTH) + x_dst] = CONV(b4);
    }

    y_src = 0;		   /* First & last row */
    y_dst = 0 + vpad;
    for (; y_src < bmp->viewport.h; y_src += 191, y_dst += 228) {
        uint8_t *src_row = &bmp->data[(y_src + bmp->viewport.y) * bmp->pitch];
        uint16_t *dest_row = &framebuffer[WIDTH * y_dst];
        int x_src = 0;
        int x_dst = hpad;
        for (; x_src < bmp->viewport.w - 1; x_src += 5, x_dst += 6) {
            uint32_t b0 = palette_spaced[src_row[x_src + 0] & 0x1f];
            uint32_t b1 = palette_spaced[src_row[x_src + 1] & 0x1f];
            uint32_t b2 = palette_spaced[src_row[x_src + 2] & 0x1f];
            uint32_t b3 = palette_spaced[src_row[x_src + 3] & 0x1f];
            uint32_t b4 = palette_spaced[src_row[x_src + 4] & 0x1f];

            dest_row[x_dst + 0]   = CONV(b0);
            dest_row[x_dst + 1] = CONV((b0+b1+b1+b1)>>2);
            dest_row[x_dst + 2] = CONV((b1+b2)>>1);
            dest_row[x_dst + 3] = CONV((b2+b3)>>1);
            dest_row[x_dst + 4] = CONV((b3+b3+b3+b4)>>2);
            dest_row[x_dst + 5] = CONV(b4);
        }
        /* Last column, x_src = 255 */
        dest_row[x_dst] = CONV(palette_spaced[src_row[x_src] & 0x1f]);
    }
}

void sms_pcm_submit() {
    uint8_t volume = odroid_audio_volume_get();
    int32_t factor = volume_tbl[volume] / 2; // Divide by 2 to prevent overflow in stereo mixing
    size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : AUDIO_BUFFER_LENGTH_SMS;
    if (audio_mute || volume == ODROID_AUDIO_VOLUME_MIN) {
        for (int i = 0; i < AUDIO_BUFFER_LENGTH_SMS; i++) {
            audiobuffer_dma[i + offset] = 0;
        }
    } else {
        for (int i = 0; i < AUDIO_BUFFER_LENGTH_SMS; i++) {
            /* mix left & right */
            int32_t sample = (sms_snd.output[0][i] + sms_snd.output[1][i]);
            audiobuffer_dma[i + offset] = (sample * factor) >> 8;
        }
    }
}

static void blit()
{
    pixel_t* curr_framebuffer = lcd_get_active_buffer();
    if (sms.console == CONSOLE_GG)     blit_gg(&bitmap, curr_framebuffer);
    else                               blit_sms(&bitmap, curr_framebuffer);
    common_ingame_overlay();
}

static void sms_draw_frame()
{
  static uint32_t lastFPSTime = 0;
  static uint32_t frames = 0;

  uint32_t currentTime = HAL_GetTick();
  uint32_t delta = currentTime - lastFPSTime;

  frames++;

  if (delta >= 1000) {
      int fps = (10000 * frames) / delta;
      printf("FPS: %d.%d, frames %ld, delta %ld ms\n", fps / 10, fps % 10, frames, delta);
      frames = 0;
      lastFPSTime = currentTime;
  }

  render_copy_palette((uint16_t *)palette);
  for (int i = 0; i < 32; i++) {
      uint16_t p = (palette[i] << 8) | (palette[i] >> 8);
      palette_spaced[i] = ((0b1111100000000000 & p) << 10) |
                          ((0b0000011111100000 & p) << 5) |
                          ((0b0000000000011111 & p));
  }

  blit();
  lcd_swap();
}

static void sms_update_keys( odroid_gamepad_state_t* joystick )
{
  uint8 k = 0;

  input.pad[0] = 0x00;
  input.pad[1] = 0x00;
  input.system = 0x00;

  if (joystick->values[ODROID_INPUT_UP])    { input.pad[0] |= INPUT_UP;       k = 1; }
  if (joystick->values[ODROID_INPUT_RIGHT]) { input.pad[0] |= INPUT_RIGHT;    k = 2; }
  if (joystick->values[ODROID_INPUT_DOWN])  { input.pad[0] |= INPUT_DOWN;     k = 3; }
  if (joystick->values[ODROID_INPUT_LEFT])  { input.pad[0] |= INPUT_LEFT;     k = 4; }
  if (joystick->values[ODROID_INPUT_A])     { input.pad[0] |= INPUT_BUTTON2;  k = 5; }
  if (joystick->values[ODROID_INPUT_B])     { input.pad[0] |= INPUT_BUTTON1;  k = 6; }

  if (consoleIsGG)
  {
      if ((joystick->values[ODROID_INPUT_SELECT]) || (joystick->values[ODROID_INPUT_Y]))  input.system |= INPUT_PAUSE;
      if ((joystick->values[ODROID_INPUT_START]) || (joystick->values[ODROID_INPUT_X]))  input.system |= INPUT_START;
  }
  else if (consoleIsCOL)
  {
      // G&W Key      : Coleco
      // Game         : 1
      // Game + Up    : 2
      // Game + Left  : 3
      // Game + Down  : 4
      // Game + Right : 5
      // Game + A     : 6
      // Game + B     : 0
      // Time         : #
      // Time + Up    : *
      // Time + Left  : 9
      // Time + Down  : 8
      // Time + Right : 7
      // Time + A     : 6
      // Time + B     : 5

      coleco.keypad[0] = 0xf0;
      if ((joystick->values[ODROID_INPUT_SELECT]) || (joystick->values[ODROID_INPUT_Y])) coleco.keypad[0] = 11 - k;
      else
      if ((joystick->values[ODROID_INPUT_START]) || (joystick->values[ODROID_INPUT_X]))  coleco.keypad[0] = (1 + k) % 7;
  }
  else { // Default like SMS
      if ((joystick->values[ODROID_INPUT_SELECT]) || (joystick->values[ODROID_INPUT_Y])) input.system |= INPUT_PAUSE;
      if ((joystick->values[ODROID_INPUT_START]) || (joystick->values[ODROID_INPUT_X]))  input.system |= INPUT_START;
  }
}


int
app_main_smsplusgx(uint8_t load_state, uint8_t start_paused, int8_t save_slot, uint8_t is_coleco)
{
    // Allocate the maximum samples count for a frame on SMS
    odroid_set_audio_dma_size(AUDIO_BUFFER_LENGTH_SMS);

    if (start_paused) {
        common_emu_state.pause_after_frames = 2;
        odroid_audio_mute(true);
    } else {
        common_emu_state.pause_after_frames = 0;
    }

    odroid_system_init(APPID_SMS, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, NULL);

    system_reset_config();
    load_rom_from_flash( is_coleco );

    sms.use_fm = 0;

    if (sms.console == CONSOLE_COLECO) {
        bitmap.width  = COL_WIDTH;
        bitmap.height = COL_HEIGHT;
        bitmap.pitch  = bitmap.width;
        bitmap.data   = fb_buffer;
    } else {
        bitmap.width = SMS_WIDTH;
        bitmap.height = SMS_HEIGHT;
        bitmap.pitch = bitmap.width;
        bitmap.data = fb_buffer;
    }

    // sms.dummy = framebuffer[0]; //A normal cart shouldn't access this memory ever. Point it to vram just in case.

    option.sndrate = AUDIO_SAMPLE_RATE;
    option.overscan = 0;
    option.extra_gg = 0;

    system_init2();
    system_reset();

    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, AUDIO_BUFFER_LENGTH_DMA_SMS);

    consoleIsSMS = sms.console == CONSOLE_SMS || sms.console == CONSOLE_SMS2;
    consoleIsGG  = sms.console == CONSOLE_GG || sms.console == CONSOLE_GGMS;
    consoleIsCOL = sms.console == CONSOLE_COLECO;
    consoleIsSG  = sms.console == CONSOLE_SG1000;

    if (sms.display == DISPLAY_NTSC) {
        common_emu_state.frame_time_10us = (uint16_t)(100000 / FPS_NTSC + 0.5f);
    }
    else {
        common_emu_state.frame_time_10us = (uint16_t)(100000 / FPS_PAL + 0.5f);
    }

    // Video
    lcd_clear_buffers();

    if (load_state) {
        odroid_system_emu_load_state(save_slot);
    }

    odroid_gamepad_state_t joystick;
    odroid_dialog_choice_t options[] = {
            ODROID_DIALOG_CHOICE_LAST
    };
    while (true)
    {
        wdog_refresh();

        bool drawFrame = common_emu_frame_loop();

        odroid_input_read_gamepad(&joystick);
        common_emu_input_loop(&joystick, options, &blit);
        common_emu_input_loop_handle_turbo(&joystick);

        sms_update_keys( &joystick );

        system_frame(!drawFrame);

        if (drawFrame) {
            sms_draw_frame();
        }

        sms_pcm_submit();

        common_emu_sound_sync(false);
    }
}

#endif
