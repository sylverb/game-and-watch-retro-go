#include "build/config.h"
#ifdef ENABLE_HOMEBREW_SMW

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

#include "stm32h7xx_hal.h"

#include "common.h"
#include "rom_manager.h"
#include "appid.h"
#include "rg_i18n.h"

#include "smw/smw_assets.h"

#include "smw/assets/smw_assets.h"
#include "smw/src/config.h"
#include "smw/src/snes/ppu.h"
#include "smw/src/types.h"
#include "smw/src/smw_rtl.h"
#include "smw/src/common_cpu_infra.h"
#include "smw/src/smw_spc_player.h"

#pragma GCC optimize("Ofast")


static int g_ppu_render_flags = kPpuRenderFlags_NewRenderer;

static uint8 g_gamepad_buttons;
static int g_input1_state;

bool g_new_ppu = true;
bool g_other_image = true;
bool g_debug_flag = false;
uint32 g_wanted_features;
struct SpcPlayer *g_spc_player;

static uint32 frameCtr = 0;
static uint32 renderedFrameCtr = 0;

#define SMW_AUDIO_SAMPLE_RATE   (16000)   // SAI Sample rate
#if LIMIT_30FPS != 0
#define FRAMERATE 30
#else
#define FRAMERATE 60
#endif /* LIMIT_30FPS */
#define SMW_AUDIO_BUFFER_LENGTH 534  // When limited to 30 fps, audio is generated for two frames at once
#define SMW_AUDIO_BUFFER_LENGTH_DMA (2 * SMW_AUDIO_BUFFER_LENGTH) // DMA buffer contains 2 frames worth of audio samples in a ring buffer

int16_t audiobuffer_smw[SMW_AUDIO_BUFFER_LENGTH];  // FIXME use audioBuffer from common.h instead???

uint8_t savestateBuffer[4096];
uint16_t bufferCount = 0;
uint32_t dstPos = 0;
uint8_t* save_address;

const uint8 *g_asset_ptrs[kNumberOfAssets];
uint32 g_asset_sizes[kNumberOfAssets];


/* keys inputs (hw & sw) */
static odroid_gamepad_state_t joystick;


void NORETURN Die(const char *error) {
  printf("Error: %s\n", error);
  assert(!"Die in SMW");
}


static void LoadAssetsChunk(size_t length, const uint8* data) {
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

static bool VerifyAssetsFile(const uint8 *data, size_t length) {
  static const char kAssetsSig[] = { kAssets_Sig };
  if (length < 16 + 32 + 32 + 8 + kNumberOfAssets * 4 ||
    memcmp(data, kAssetsSig, 48) != 0 ||
    *(uint32 *)(data + 80) != kNumberOfAssets)
    return false;
  return true;
}

static void LoadAssets() {
  if (!VerifyAssetsFile(smw_assets, smw_assets_length))
    Die("Mismatching assets file - Please re run 'python assets/restool.py'");

  // Load some assets with assets in extflash
  LoadAssetsChunk(smw_assets_length, smw_assets);

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

const uint8 *FindPtrInAsset(int asset, uint32 addr) {
  return FindAddrInMemblk((MemBlk){g_asset_ptrs[asset], g_asset_sizes[asset]}, addr);
}


void RtlDrawPpuFrame(uint8 *pixel_buffer, size_t pitch, uint32 render_flags) {
  g_rtl_game_info->draw_ppu_frame();
}

static void DrawPpuFrame(uint16_t* framebuffer) {
  wdog_refresh();
  uint8 *pixel_buffer = (uint8 *)(framebuffer + 320*8 + 32);    // Start 8 rows from the top, 32 pixels from left
  int pitch = 320 * 2;

  PpuBeginDrawing(g_my_ppu, pixel_buffer, pitch, g_ppu_render_flags);
  RtlDrawPpuFrame(pixel_buffer, pitch, g_ppu_render_flags);

  // Draw borders
  draw_border_smw(framebuffer);
}


void RtlApuLock() {
}

void RtlApuUnlock() {
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
    for (int i = 0; i < SMW_AUDIO_BUFFER_LENGTH_DMA; i++) {
        audiobuffer_dma[i] = 0;
    }
    RtlSaveLoad(kSaveLoad_Load, save_address);
  } else if (j == kKeys_Save) {
    // Mute
    for (int i = 0; i < SMW_AUDIO_BUFFER_LENGTH_DMA; i++) {
        audiobuffer_dma[i] = 0;
    }
    RtlSaveLoad(kSaveLoad_Save, save_address);
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

static bool smw_system_SaveState(char *pathName) {
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

static bool smw_system_LoadState(char *pathName) {
  printf("Loading state...\n");
  save_address = (unsigned char *)ACTIVE_FILE->save_address;
  HandleCommand(kKeys_Load, true);
  return true;
}

// FIXME no support for SRAM saves ???
uint8_t* readSramImpl() {
  return NULL;//SAVE_SRAM_EXTFLASH;
}
void writeSramImpl(uint8_t* sram) {
  //store_save(SAVE_SRAM_EXTFLASH, sram, 2048);
}

static void smw_sound_start()
{
  memset(audiobuffer_smw, 0, sizeof(audiobuffer_smw));
  memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
  HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, SMW_AUDIO_BUFFER_LENGTH_DMA);
}

static void smw_sound_submit() {
  uint8_t volume = odroid_audio_volume_get();
  int16_t factor = volume_tbl[volume];

  size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : SMW_AUDIO_BUFFER_LENGTH;

  if (audio_mute || volume == ODROID_AUDIO_VOLUME_MIN) {
    for (int i = 0; i < SMW_AUDIO_BUFFER_LENGTH; i++) {
      audiobuffer_dma[i + offset] = 0;
    }
  } else {
    for (int i = 0; i < SMW_AUDIO_BUFFER_LENGTH; i++) {
      int32_t sample = audiobuffer_smw[i];
      audiobuffer_dma[i + offset] = (sample * factor) >> 8;
    }
  }
}

/* Main */
int app_main_smw(uint8_t load_state, uint8_t start_paused, uint8_t save_slot)
{
  printf("SMW start\n");
  odroid_system_init(APPID_SMW, SMW_AUDIO_SAMPLE_RATE);
  odroid_system_emu_init(&smw_system_LoadState, &smw_system_SaveState, NULL);
  
  if (start_paused) {
      common_emu_state.pause_after_frames = 2;
  } else {
      common_emu_state.pause_after_frames = 0;
  }
  common_emu_state.frame_time_10us = (uint16_t)(100000 / FRAMERATE + 0.5f);

  /* clear the screen before rendering */
  memset(lcd_get_inactive_buffer(), 0, 320 * 240 * 2);
  memset(lcd_get_active_buffer(), 0, 320 * 240 * 2);

  unsigned short *screen = 0;
  screen = lcd_get_active_buffer();

  LoadAssets();

  SnesInit(NULL, 0);
    
  g_wanted_features = FEATURES;

  g_spc_player = SmwSpcPlayer_Create();
  g_spc_player->initialize(g_spc_player);
    
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
  smw_sound_start();

  printf("Entering loop\n");
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
      A                   A button (Spin Jump)
      B                   B button (Regular Jump)
      SELECT | START      X/Y button (Dash/Shoot)
      TIME                Select button (Use Reserve Item)
      GAME + TIME         Start button (Pause Game)
      GAME + B            L button (Scroll Screen Left)
      GAME + A            R button (Scroll Screen Right)
    Game controls for mario console:
      A                   A button (Spin Jump)
      B                   B button (Regular Jump)
      TIME                X/Y button (Dash/Shoot)
      GAME + A            Select button (Use Reserve Item)
      GAME + TIME         Start button (Pause Game)
      ----                L button (Scroll Screen Left)
      ----                R button (Scroll Screen Right)
    */

    bool isPauseModifierPressed = joystick.values[ODROID_INPUT_VOLUME];
    bool isGameModifierPressed = joystick.values[ODROID_INPUT_START];

    HandleCommand(1, !isPauseModifierPressed && joystick.values[ODROID_INPUT_UP]);
    HandleCommand(2, !isPauseModifierPressed && joystick.values[ODROID_INPUT_DOWN]);
    HandleCommand(3, !isPauseModifierPressed && joystick.values[ODROID_INPUT_LEFT]);
    HandleCommand(4, !isPauseModifierPressed && joystick.values[ODROID_INPUT_RIGHT]);
    HandleCommand(7, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_A]); // A == A (Spin Jump)
    HandleCommand(8, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_B]); // B == B (Regular Jump)
    HandleCommand(6, !isPauseModifierPressed && isGameModifierPressed && joystick.values[ODROID_INPUT_SELECT]); // GAME + TIME == Start (Pause Game)

    #if GNW_TARGET_ZELDA != 0
        HandleCommand(9, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_Y]); // SELECT == X (Dash/Shoot)
        HandleCommand(10, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_X]);  // START == Y (Dash/Shoot)
        HandleCommand(5, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_SELECT]);  // TIME == Select (Use reserve item)
        // L & R are used to scroll screen
        HandleCommand(11, !isPauseModifierPressed && isGameModifierPressed && joystick.values[ODROID_INPUT_B]); // GAME + B == L
        HandleCommand(12, !isPauseModifierPressed && isGameModifierPressed && joystick.values[ODROID_INPUT_A]); // GAME + A == R
    #else
        HandleCommand(9, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_SELECT]);  // TIME == X (Dash/Shoot)
        HandleCommand(10, !isPauseModifierPressed && !isGameModifierPressed && joystick.values[ODROID_INPUT_SELECT]); // TIME == Y (Dash/Shoot)
        HandleCommand(5, !isPauseModifierPressed && isGameModifierPressed && joystick.values[ODROID_INPUT_A]);  // GAME + A == Select (Use reserve item)
        // No button combinations available for L/R on Mario units...
    #endif /* GNW_TARGET_ZELDA */


    // Clear gamepad inputs when joypad directional inputs to avoid wonkiness
    int inputs = g_input1_state;
    if (g_input1_state & 0xf0)
      g_gamepad_buttons = 0;
    inputs |= g_gamepad_buttons;


    bool drawFrame = common_emu_frame_loop();

    RtlRunFrame(inputs);

    frameCtr++;

    #if LIMIT_30FPS != 0
    // Render audio to DMA buffer
    RtlRenderAudio(audiobuffer_smw, SMW_AUDIO_BUFFER_LENGTH / 2, 1);
    // Render two frames worth of gameplay / audio for each screen render
    RtlRunFrame(inputs);
    RtlRenderAudio(audiobuffer_smw + (SMW_AUDIO_BUFFER_LENGTH / 2), SMW_AUDIO_BUFFER_LENGTH / 2, 1);
    #else
    RtlRenderAudio(audiobuffer_smw, SMW_AUDIO_BUFFER_LENGTH, 1);
    #endif /* LIMIT_30FPS*/


    if (drawFrame) {

      /* copy audio samples for DMA */
      smw_sound_submit();

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