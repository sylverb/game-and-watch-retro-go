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
#include "appid.h"

/* TO move elsewhere */
#include "stm32h7xx_hal.h"

// FIXME retro-go includes ?
#include "common.h"
#include "rom_manager.h"
#include "appid.h"
#include "rg_i18n.h"

#include "zelda_assets.h"

// zelda3
#include "assets.h"
#include "zelda3/config.h"
#include "snes/ppu.h"
#include "types.h"
#include "zelda_rtl.h"
#include "hud.h"

// FIXME optimization flags ?
#pragma GCC optimize("Ofast")


// FIXME zelda3 global variables, settings, etc. (c.f. main.c in zelda3 standalone)

// TODO reconfigure audio buffer/dma at 30fps / 16000Hz ???
// TODO text KO ???

#if EXTENDED_SCREEN != 0
static int g_ppu_render_flags = kPpuRenderFlags_NewRenderer | kPpuRenderFlags_Height240;
#else
static int g_ppu_render_flags = kPpuRenderFlags_NewRenderer;
#endif /* EXTENDED_SCREEN */

static uint8 g_gamepad_buttons;
static int g_input1_state;

static uint32 frameCtr = 0;
static uint32 renderedFrameCtr = 0;

#define AUDIO_SAMPLE_RATE   (16000)   // SAI Sample rate
#if LIMIT_30FPS != 0
#define FRAMERATE 30
#else
#define FRAMERATE 60
#endif /* LIMIT_30FPS */
#define AUDIO_BUFFER_LENGTH 534  // When limited to 30 fps, audio is generated for two frames at once
#define AUDIO_BUFFER_LENGTH_DMA (2 * AUDIO_BUFFER_LENGTH) // DMA buffer contains 2 frames worth of audio samples in a ring buffer

int16_t audiobuffer_zelda3[AUDIO_BUFFER_LENGTH];  // FIXME use audioBuffer from common.h instead???

const uint8 *g_asset_ptrs[kNumberOfAssets];
uint32 g_asset_sizes[kNumberOfAssets];


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


static void LoadAssetsChunk(size_t length, uint8* data) {
  uint32 offset = 88 + kNumberOfAssets * 4 + *(uint32 *)(data + 84);
  for (size_t i = 0; i < kNumberOfAssets; i++) {
    uint32 size = *(uint32 *)(data + 88 + i * 4);
    offset = (offset + 3) & ~3;
    if ((uint64)offset + size > length)
      assert(!"Assets file corruption");
    g_asset_sizes[i] = size;
    g_asset_ptrs[i] = data + offset;
    offset += size;
  }
}

static void LoadAssets() {
  // Load some assets with assets in extflash
  LoadAssetsChunk(zelda_assets_length, zelda_assets);

  // Make sure all assets were loaded
  for (size_t i = 0; i < kNumberOfAssets; i++) {
    if (g_asset_ptrs[i] == 0) {
      assert(!"Missing asset");
    }
  }

}

MemBlk FindInAssetArray(int asset, int idx) {
  return FindIndexInMemblk((MemBlk) { g_asset_ptrs[asset], g_asset_sizes[asset] }, idx);
}


static void DrawPpuFrame(void* framebuffer) {
  wdog_refresh();
  #if EXTENDED_SCREEN == 2
  uint8 *pixel_buffer = framebuffer;
  #elif EXTENDED_SCREEN == 1
  uint8 *pixel_buffer = framebuffer + 32;    // Start 32 pixels from left
  #else
  uint8 *pixel_buffer = framebuffer + 320*8 + 32;    // Start 8 rows from the top, 32 pixels from left
  #endif /* EXTENDED_SCREEN */
  int pitch = 320 * 2;
  
  ZeldaDrawPpuFrame(pixel_buffer, pitch, g_ppu_render_flags);
}


void ZeldaApuLock() {
}

void ZeldaApuUnlock() {
}

static void HandleCommand(uint32 j, bool pressed) {
  if (j <= kKeys_Controls_Last) {
    static const uint8 kKbdRemap[] = { 0, 4, 5, 6, 7, 2, 3, 8, 0, 9, 1, 10, 11 };
    if (pressed)
      g_input1_state |= 1 << kKbdRemap[j];
    else
      g_input1_state &= ~(1 << kKbdRemap[j]);
    return;
  }

  /* FIXME if (j == kKeys_Turbo) {
    g_turbo = pressed;
    return;
  }*/


  /* FIXME #if ENABLE_SAVESTATE != 0
  // FIXME Support multiple slots?
  if (j == kKeys_Load) {
    // Mute
    for (int i = 0; i < AUDIO_BUFFER_LENGTH_DMA; i++) {
        audiobuffer_dma[i] = 0;
    }
    SaveLoadSlot(kSaveLoad_Load, &SAVESTATE_EXTFLASH);
  } else if (j == kKeys_Save) {
    // Mute
    for (int i = 0; i < AUDIO_BUFFER_LENGTH_DMA; i++) {
        audiobuffer_dma[i] = 0;
    }
    SaveLoadSlot(kSaveLoad_Save, &SAVESTATE_EXTFLASH);
  }
  #endif*/
}

// FIXME SRAM save uint8_t SAVE_SRAM_EXTFLASH[8192]  __attribute__((section (".saveflash"))) __attribute__((aligned(4096)));

uint8_t* readSramImpl() {
  return NULL;// FIXME SRAM save SAVE_SRAM_EXTFLASH;
}
void writeSramImpl(uint8_t* sram) {
  // FIXME SRAM save store_save(SAVE_SRAM_EXTFLASH, sram, 8192);
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
  uint8_t volume = odroid_audio_volume_get();
  int16_t factor = volume_tbl[volume];

  size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : AUDIO_BUFFER_LENGTH;

  if (audio_mute || volume == ODROID_AUDIO_VOLUME_MIN) {
    for (int i = 0; i < AUDIO_BUFFER_LENGTH; i++) {
      audiobuffer_dma[i + offset] = 0;
    }
  } else {
    for (int i = 0; i < AUDIO_BUFFER_LENGTH; i++) {
      int32_t sample = audiobuffer_zelda3[i];
      audiobuffer_dma[i + offset] = (sample * factor) >> 8;
    }
  }
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
  odroid_system_init(APPID_ZELDA3, AUDIO_SAMPLE_RATE);
  odroid_system_emu_init(&zelda3_system_LoadState, &zelda3_system_SaveState, NULL);

  // Init Sound
  memset(audiobuffer_zelda3, 0, sizeof(audiobuffer_zelda3));
  memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
  HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, AUDIO_BUFFER_LENGTH_DMA);
  
  common_emu_state.frame_time_10us = (uint16_t)(100000 / FRAMERATE + 0.5f);

  /* clear the screen before rendering */
  memset(lcd_get_inactive_buffer(), 0, 320 * 240 * 2);
  memset(lcd_get_active_buffer(), 0, 320 * 240 * 2);
  
  unsigned short *screen = 0;
  screen = lcd_get_active_buffer();

  LoadAssets();
    
  ZeldaInitialize();

  #if EXTENDED_SCREEN == 2
  g_zenv.ppu->extraLeftRight = UintMin(32, kPpuExtraLeftRight);
  #else
  g_zenv.ppu->extraLeftRight = 0;
  #endif /* EXTENDED_SCREEN */

  g_wanted_zelda_features = 0;  //FIXME FEATURES;

  ZeldaEnableMsu(false);

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
    void _repaint()
    {
        // FIXME blit game???
        common_ingame_overlay();
    }
    common_emu_input_loop(&joystick, options, &_repaint);


    // FIXME Handle inputs
    // FIXME Play well with retro-go's controls
    HandleCommand(1, joystick.values[ODROID_INPUT_UP]);
    HandleCommand(2, joystick.values[ODROID_INPUT_DOWN]);
    HandleCommand(3, joystick.values[ODROID_INPUT_LEFT]);
    HandleCommand(4, joystick.values[ODROID_INPUT_RIGHT]);

    HandleCommand(7, joystick.values[ODROID_INPUT_A]);  // A (Pegasus Boots/Interacting)
    HandleCommand(8, joystick.values[ODROID_INPUT_B]);  // B (Sword)

    #if GNW_TARGET_ZELDA != 0
        HandleCommand(9, joystick.values[ODROID_INPUT_VOLUME]);    // X (Show Map)
        HandleCommand(10, joystick.values[ODROID_INPUT_Y]);  // Y (Use Item)
        
        HandleCommand(5, joystick.values[ODROID_INPUT_SELECT]);   // Select (Save Screen)
        HandleCommand(6, joystick.values[ODROID_INPUT_X]);  // Start (Item Selection Screen)
        
        // L & R aren't used in Zelda3, but we could enable item quick-swapping.
        // FIXME HandleCommand(11, (buttons & B_GAME) && (buttons & B_SELECT)); // L
        // FIXME HandleCommand(12, (buttons & B_GAME) && (buttons & B_START)); // R
    #else 
        HandleCommand(9, !joystick.values[ODROID_INPUT_START] && joystick.values[ODROID_INPUT_SELECT]);    // X
        HandleCommand(10, !joystick.values[ODROID_INPUT_START] && joystick.values[ODROID_INPUT_VOLUME]);  // Y
        
        HandleCommand(5, joystick.values[ODROID_INPUT_START] && joystick.values[ODROID_INPUT_SELECT]);   // Select
        HandleCommand(6, joystick.values[ODROID_INPUT_START] && joystick.values[ODROID_INPUT_VOLUME]);  // Start

        // No button combinations available for L/R on Mario units...
        //HandleCommand(11, (buttons & B_GAME) && (buttons & B_B)); // L
        //HandleCommand(12, (buttons & B_GAME) && (buttons & B_A)); // R
    #endif /* GNW_TARGET_ZELDA */


    // Clear gamepad inputs when joypad directional inputs to avoid wonkiness
    int inputs = g_input1_state;
    if (g_input1_state & 0xf0)
      g_gamepad_buttons = 0;
    inputs |= g_gamepad_buttons;


    bool drawFrame = common_emu_frame_loop();

    // FIXME Run to frames at 30fps
    bool is_replay = ZeldaRunFrame(inputs);

    frameCtr++;

    #if LIMIT_30FPS != 0
    // Render audio to DMA buffer
    ZeldaRenderAudio(audiobuffer_zelda3, AUDIO_BUFFER_LENGTH / 2, 1);
    // Render two frames worth of gameplay / audio for each screen render
    ZeldaRunFrame(inputs);
    ZeldaRenderAudio(audiobuffer_zelda3 + (AUDIO_BUFFER_LENGTH / 2), AUDIO_BUFFER_LENGTH / 2, 1);
    #else
    ZeldaRenderAudio(audiobuffer_zelda3, AUDIO_BUFFER_LENGTH, 1);
    #endif /* LIMIT_30FPS*/


    if (drawFrame) {

      /* copy audio samples for DMA */
      zelda3_sound_submit();

      ZeldaDiscardUnusedAudioFrames();

      // todo switch framebuffer ??
      screen = lcd_get_active_buffer();

      // TODO draw something !!!
      //memset(screen, (frameCtr % 0xff), 320 * 240 * 2);
      DrawPpuFrame(screen);

      // FIXME in-game overlay ???
      common_ingame_overlay();

      lcd_swap();

      renderedFrameCtr++;
    }

    if(!common_emu_state.skip_frames)
    {
        static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
        for(uint8_t p = 0; p < common_emu_state.pause_frames + 1; p++) {
            while (dma_state == last_dma_state) {
                cpumon_sleep();
            }
            last_dma_state = dma_state;
        }
    }
    //lcd_wait_for_vblank();

  }

}
#endif
