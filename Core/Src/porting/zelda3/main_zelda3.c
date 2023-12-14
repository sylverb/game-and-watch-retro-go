/*
TODO copyright?
*/
#include "build/config.h"
#ifdef ENABLE_HOMEBREW_ZELDA3

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
#include "bq24072.h"

#include "stm32h7xx_hal.h"

#include "common.h"
#include "rom_manager.h"
#include "appid.h"
#include "rg_i18n.h"

#include "zelda3/zelda_assets.h"

#include "zelda3/assets.h"
#include "zelda3/config.h"
#include "zelda3/snes/ppu.h"
#include "zelda3/types.h"
#include "zelda3/zelda_rtl.h"
#include "zelda3/hud.h"

#pragma GCC optimize("Ofast")


#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

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

uint8_t savestateBuffer[4096];
uint16_t bufferCount = 0;
uint32_t dstPos = 0;
uint8_t* save_address;

const uint8 *g_asset_ptrs[kNumberOfAssets];
uint32 g_asset_sizes[kNumberOfAssets];


/* keys inputs (hw & sw) */
static odroid_gamepad_state_t joystick;


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


static void DrawPpuFrame(uint16_t* framebuffer) {
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

  // Draw borders
  #if EXTENDED_SCREEN < 2
  draw_border_zelda3(framebuffer);
  #endif /* EXTENDED_SCREEN */
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

  /*if (j == kKeys_Turbo) {
    g_turbo = pressed;
    return;
  }*/


  if (j == kKeys_Load) {
    // Mute
    for (int i = 0; i < AUDIO_BUFFER_LENGTH_DMA; i++) {
        audiobuffer_dma[i] = 0;
    }
    SaveLoadSlot(kSaveLoad_Load, save_address);
  } else if (j == kKeys_Save) {
    // Mute
    for (int i = 0; i < AUDIO_BUFFER_LENGTH_DMA; i++) {
        audiobuffer_dma[i] = 0;
    }
    SaveLoadSlot(kSaveLoad_Save, save_address);
  }
}

void writeSaveStateInitImpl() {
  dstPos = 0;
  bufferCount = 0;
}
void writeSaveStateImpl(uint8_t* data, size_t size) {
  uint32_t srcPos = 0;
  size_t remaining = size;
  if (bufferCount > 0) {
    size_t a = 4096 - bufferCount;
    size_t b = size;
    size_t bufferPad = a < b ? a : b;
    memcpy(savestateBuffer + bufferCount, data, bufferPad);
    bufferCount += bufferPad;
    remaining -= bufferPad;
    srcPos += bufferPad;
    if (bufferCount == 4096) {
      store_save(save_address + dstPos, savestateBuffer, 4096);
      dstPos += 4096;
      bufferCount = 0;
    }
  }
  while (remaining >= 4096) {
    store_save(save_address + dstPos, data + srcPos, 4096);
    dstPos += 4096;
    srcPos += 4096;
    remaining -= 4096;
    wdog_refresh();
  }
  if (remaining > 0) {
    memcpy(savestateBuffer, data + srcPos, remaining);
    bufferCount += remaining;
  }
}
void writeSaveStateFinalizeImpl() {
  if (bufferCount > 0) {
    store_save(save_address + dstPos, savestateBuffer, bufferCount);
    dstPos += bufferCount;
    bufferCount = 0;
  }
  writeSaveStateInitImpl();
}

static bool zelda3_system_SaveState(char *pathName) {
  printf("Saving state...\n");
#if OFF_SAVESTATE==1
  if (strcmp(pathName,"1") == 0) {
    // Save in common save slot (during a power off)
    save_address = (unsigned char *)&__OFFSAVEFLASH_START__;
  } else {
#endif
    save_address = (unsigned char *)ACTIVE_FILE->save_address;
#if OFF_SAVESTATE==1
  }
#endif
  HandleCommand(kKeys_Save, true);
  printf("Saved state\n");
  return true;
}

static bool zelda3_system_LoadState(char *pathName) {
  printf("Loading state...\n");
  save_address = (unsigned char *)ACTIVE_FILE->save_address;
  HandleCommand(kKeys_Load, true);
}

// FIXME no support for SRAM saves ???
uint8_t* readSramImpl() {
  return NULL;//SAVE_SRAM_EXTFLASH;
}
void writeSramImpl(uint8_t* sram) {
  //store_save(SAVE_SRAM_EXTFLASH, sram, 8192);
}

static void zelda3_sound_start()
{
  memset(audiobuffer_zelda3, 0, sizeof(audiobuffer_zelda3));
  memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
  HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, AUDIO_BUFFER_LENGTH_DMA);
}

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

/* Main */
int app_main_zelda3(uint8_t load_state, uint8_t start_paused, uint8_t save_slot)
{
  printf("Zelda3 start\n");
  odroid_system_init(APPID_ZELDA3, AUDIO_SAMPLE_RATE);
  odroid_system_emu_init(&zelda3_system_LoadState, &zelda3_system_SaveState, NULL);
  
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

  g_wanted_zelda_features = FEATURES;

  ZeldaEnableMsu(false);
  ZeldaSetLanguage(STRINGIZE_VALUE_OF(DIALOGUES_LANGUAGE));
    
  if (load_state) {
#if OFF_SAVESTATE==1
    if (save_slot == 1) {
      // Load from common save slot if needed
      save_address = (unsigned char *)&__OFFSAVEFLASH_START__;
    } else {
#endif
      save_address = (unsigned char *)ACTIVE_FILE->save_address;
#if OFF_SAVESTATE==1
    }
#endif
    HandleCommand(kKeys_Load, true);
  }

  /* Start at the same time DMAs audio & video */
  /* Audio period and Video period are the same (almost at least 1 hour) */
  lcd_wait_for_vblank();
  zelda3_sound_start();

  while (true) {

    /* reset watchdog */
    wdog_refresh();

    // Update battery level
    #if BATTERY_INDICATOR
    g_battery.level = bq24072_get_percent_filtered();
    g_battery.is_charging = (
        (bq24072_get_state() == BQ24072_STATE_CHARGING)
        || (bq24072_get_state() == BQ24072_STATE_FULL)
        );
    #endif

    /* hardware keys */
    odroid_input_read_gamepad(&joystick);


    odroid_dialog_choice_t options[] = {
            ODROID_DIALOG_CHOICE_LAST
    };
    void _repaint()
    {
      screen = lcd_get_active_buffer();
      DrawPpuFrame(screen);
      common_ingame_overlay();
    }
    common_emu_input_loop(&joystick, options, &_repaint);


    // Handle inputs
    /*
    Retro-Go controls:
      PAUSE/SET + GAME    Store a screenshot. (Disabled by default on 1MB flash builds)
      PAUSE/SET + TIME    Toggle speedup between 1x and the last non-1x speed. Defaults to 1.5x.
      PAUSE/SET + UP 	    Brightness up.
      PAUSE/SET + DOWN 	  Brightness down.
      PAUSE/SET + RIGHT 	Volume up.
      PAUSE/SET + LEFT 	  Volume down.
      PAUSE/SET + B 	    Load state.
      PAUSE/SET + A 	    Save state.
      PAUSE/SET + POWER 	Poweroff WITHOUT save-stating.
    Game controls for zelda console:
      A                   A button (Pegasus Boots / Interacting)
      B                   B button (Sword)
      TIME                X button (Show Map)
      SELECT              Y button (Use Item)
      GAME + TIME         Select button (Save Screen)
      START               Start button (Item Selection Screen)
      GAME + B            L button (Quick-swapping, if enabled)
      GAME + A            R button (Quick-swapping, if enabled)
    Game controls for mario console:
      A                   A button (Pegasus Boots / Interacting)
      B                   B button (Sword)
      GAME + B            X button (Show Map)
      TIME                Y button (Use Item)
      GAME + TIME         Select button (Save Screen)
      GAME + A            Start button (Item Selection Screen)
      ----                L button (Quick-swapping, if enabled)
      ----                R button (Quick-swapping, if enabled)
    */

    bool isPauseModifierPressed = joystick.values[ODROID_INPUT_VOLUME];
    bool isGameModifierPressed = joystick.values[ODROID_INPUT_START];

    HandleCommand(1, !isPauseModifierPressed && joystick.values[ODROID_INPUT_UP]);
    HandleCommand(2, !isPauseModifierPressed && joystick.values[ODROID_INPUT_DOWN]);
    HandleCommand(3, !isPauseModifierPressed && joystick.values[ODROID_INPUT_LEFT]);
    HandleCommand(4, !isPauseModifierPressed && joystick.values[ODROID_INPUT_RIGHT]);
    HandleCommand(7, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_A]); // A == A (Pegasus Boots/Interacting)
    HandleCommand(8, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_B]); // B == B (Sword)
    HandleCommand(5, !isPauseModifierPressed && isGameModifierPressed && joystick.values[ODROID_INPUT_SELECT]); // GAME + TIME == Select (Save Screen)

    #if GNW_TARGET_ZELDA != 0
        HandleCommand(9, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_SELECT]);  // TIME == X (Show Map)
        HandleCommand(10, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_Y]);  // SELECT == Y (Use Item)
        HandleCommand(6, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_X]); // START == Start (Item Selection Screen)
        // L & R aren't used in Zelda3, but we could enable item quick-swapping.
        HandleCommand(11, !isPauseModifierPressed && isGameModifierPressed && joystick.values[ODROID_INPUT_B]); // GAME + B == L
        HandleCommand(12, !isPauseModifierPressed && isGameModifierPressed && joystick.values[ODROID_INPUT_A]); // GAME + A == R
    #else
        HandleCommand(9, !isPauseModifierPressed && isGameModifierPressed && joystick.values[ODROID_INPUT_B]);  // GAME + B == X (Show Map)
        HandleCommand(10, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_SELECT]); // TIME == Y (Use Item)
        HandleCommand(6, !isPauseModifierPressed && isGameModifierPressed && joystick.values[ODROID_INPUT_A]); // GAME + A == Start (Item Selection Screen)
        // No button combinations available for L/R on Mario units...
    #endif /* GNW_TARGET_ZELDA */


    // Clear gamepad inputs when joypad directional inputs to avoid wonkiness
    int inputs = g_input1_state;
    if (g_input1_state & 0xf0)
      g_gamepad_buttons = 0;
    inputs |= g_gamepad_buttons;


    bool drawFrame = common_emu_frame_loop();

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

      screen = lcd_get_active_buffer();

      DrawPpuFrame(screen);

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

  }

}
#endif
