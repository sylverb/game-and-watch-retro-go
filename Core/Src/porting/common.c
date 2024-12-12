#include "common.h"
#include <odroid_system.h>
#include <odroid_overlay.h>

#include <string.h>
#include <nofrendo.h>
#include <nes.h>
#include <nes_input.h>
#include <nes_state.h>
#include <nes_input.h>
#include <osd.h>
#include "main.h"
#include "bitmaps.h"
#include "gw_buttons.h"
#include "gw_lcd.h"
#include "gw_audio.h"
#include "gw_linker.h"
#include "odroid_audio.h"
#include "rg_i18n.h"
#include "gw_multisync.h"
#ifdef SALEAE_DEBUG_SIGNALS
#include "gw_debug.h"
#endif

#if ENABLE_SCREENSHOT
uint16_t framebuffer_capture[GW_LCD_WIDTH * GW_LCD_HEIGHT]  __attribute__((section (".fbflash"))) __attribute__((aligned(4096)));
#endif

static void set_ingame_overlay(ingame_overlay_t type);

cpumon_stats_t cpumon_stats = {0};

const uint8_t volume_tbl[ODROID_AUDIO_VOLUME_MAX + 1] = {
    (uint8_t)(UINT8_MAX * 0.00f),
    (uint8_t)(UINT8_MAX * 0.06f),
    (uint8_t)(UINT8_MAX * 0.125f),
    (uint8_t)(UINT8_MAX * 0.187f),
    (uint8_t)(UINT8_MAX * 0.25f),
    (uint8_t)(UINT8_MAX * 0.35f),
    (uint8_t)(UINT8_MAX * 0.42f),
    (uint8_t)(UINT8_MAX * 0.60f),
    (uint8_t)(UINT8_MAX * 0.80f),
    (uint8_t)(UINT8_MAX * 1.00f),
};

bool odroid_netplay_quick_start(void)
{
    return true;
}

// TODO: Move to own file
void odroid_audio_mute(bool mute)
{
    if (mute) {
        audio_clear_buffers();
    }

    audio_mute = mute;
}

common_emu_state_t common_emu_state = {
    .frame_time_10us = (uint16_t)(100000 / 60 + 0.5f),  // Reasonable default of 60FPS if not explicitly configured.
};

static int32_t frame_integrator = 0;

void common_emu_frame_loop_reset(void){
    common_emu_state.last_sync_time = 0;
    common_emu_state.skipped_frames =0;
    common_emu_state.skip_frames =0;
    common_emu_state.pause_frames=0;
    common_emu_state.pause_after_frames=0;
    common_emu_state.startup_frames=0;
    frame_integrator = 0;
}

bool common_emu_frame_loop(void){
    rg_app_desc_t *app = odroid_system_get_app();
    int16_t frame_time_10us = common_emu_state.frame_time_10us;
    int16_t elapsed_10us = 100 * get_elapsed_time_since(common_emu_state.last_sync_time);
    bool draw_frame = common_emu_state.skip_frames < 2;

    if( !cpumon_stats.busy_ms ) cpumon_busy();
    odroid_system_tick(!draw_frame, 0, cpumon_stats.busy_ms);
    cpumon_reset();

    common_emu_state.pause_frames = 0;
    common_emu_state.skip_frames = 0;

    common_emu_state.last_sync_time = get_elapsed_time();

    if(common_emu_state.startup_frames < 3) {
        common_emu_state.startup_frames++;
        return true;
    }

    switch(app->speedupEnabled){
        case SPEEDUP_0_5x:
            frame_time_10us *= 2;
            break;
        case SPEEDUP_0_75x:
            frame_time_10us *= 5;
            frame_time_10us /= 4;
            break;
        case SPEEDUP_1_25x:
            frame_time_10us *= 4;
            frame_time_10us /= 5;
            break;
        case SPEEDUP_1_5x:
            frame_time_10us *= 2;
            frame_time_10us /= 3;
            break;
        case SPEEDUP_2x:
            frame_time_10us /= 2;
            break;
        case SPEEDUP_3x:
            frame_time_10us /= 3;
            break;
    }
    frame_integrator += (elapsed_10us - frame_time_10us);
    if(frame_integrator > frame_time_10us << 1) common_emu_state.skip_frames = 2;
    else if(frame_integrator > frame_time_10us) common_emu_state.skip_frames = 1;
    else if(frame_integrator < -frame_time_10us) common_emu_state.pause_frames = 1;
    common_emu_state.skipped_frames += common_emu_state.skip_frames;

    return draw_frame;
}

static bool ingame_overlay_loop() {
    if(get_elapsed_time_since(common_emu_state.last_overlay_time) > 1000) {
        if (common_emu_state.overlay != INGAME_OVERLAY_NONE) {
            set_ingame_overlay(INGAME_OVERLAY_NONE);
            return true;
        }
    }
    return false;
}

/**
 * Common input/macro/menuing features inside all emu loops. This is to be called
 * after inputs are read into `joystick`, but before the actual emulation tick
 * is called.
 *
 * Note: No pending LCD swaps are allowed when calling this function.
 *       This is to ensure a stutter free handling of the in-game overlays.
 */
void common_emu_input_loop(odroid_gamepad_state_t *joystick, odroid_dialog_choice_t *game_options, void_callback_t repaint) {
    rg_app_desc_t *app = odroid_system_get_app();
    static emu_speedup_t last_speedup = SPEEDUP_1_5x;
    static int8_t last_key = -1;
    static bool pause_pressed = false;
    static bool macro_activated = false;
    static uint8_t clear_frames = 0;

    void _repaint() {
        ingame_overlay_loop();
        repaint();
    }

    if(joystick->values[ODROID_INPUT_VOLUME]){  // PAUSE/SET button
        // PAUSE/SET has been pressed, checking additional inputs for macros
        pause_pressed = true;
        if(last_key < 0) {
            if (joystick->values[ODROID_INPUT_POWER]){
                // Do NOT save-state and then poweroff
                last_key = ODROID_INPUT_POWER;
                audio_stop_playing();
                odroid_system_sleep();
            }
            else if(joystick->values[ODROID_INPUT_START]){ // GAME button
#if ENABLE_SCREENSHOT
                printf("Capturing screenshot...\n");
                odroid_audio_mute(true);
                lcd_sleep_while_swap_pending();
                store_save((uint8_t *) framebuffer_capture, lcd_get_inactive_buffer(), sizeof(framebuffer_capture));
                set_ingame_overlay(INGAME_OVERLAY_SC);
                odroid_audio_mute(false);
                common_emu_state.startup_frames = 0;
                printf("Screenshot captured\n");
#else
                printf("Screenshot support is disabled\n");
#endif
                last_key = ODROID_INPUT_START;
            }
            else if(joystick->values[ODROID_INPUT_SELECT]){ // TIME button
                // Toggle Speedup
                last_key = ODROID_INPUT_SELECT;
                if(app->speedupEnabled == SPEEDUP_1x) {
                    app->speedupEnabled = last_speedup;
                }
                else {
                    last_speedup = app->speedupEnabled;
                    app->speedupEnabled = SPEEDUP_1x;
                }
                set_ingame_overlay(INGAME_OVERLAY_SPEEDUP);
            }
            else if(joystick->values[ODROID_INPUT_LEFT]){
                // Volume Up
                last_key = ODROID_INPUT_LEFT;
                int8_t level = odroid_audio_volume_get();
                if (level > ODROID_AUDIO_VOLUME_MIN) odroid_audio_volume_set(--level);
                set_ingame_overlay(INGAME_OVERLAY_VOLUME);
            }
            else if(joystick->values[ODROID_INPUT_RIGHT]){
                // Volume Down
                last_key = ODROID_INPUT_RIGHT;
                int8_t level = odroid_audio_volume_get();
                if (level < ODROID_AUDIO_VOLUME_MAX) odroid_audio_volume_set(++level);
                set_ingame_overlay(INGAME_OVERLAY_VOLUME);
            }
            else if(joystick->values[ODROID_INPUT_UP]){
                // Brightness Up
                last_key = ODROID_INPUT_UP;
                int8_t level = odroid_display_get_backlight();
                if (level < ODROID_BACKLIGHT_LEVEL_COUNT - 1) odroid_display_set_backlight(++level);
                set_ingame_overlay(INGAME_OVERLAY_BRIGHTNESS);
            }
            else if(joystick->values[ODROID_INPUT_DOWN]){
                // Brightness Down
                last_key = ODROID_INPUT_DOWN;
                int8_t level = odroid_display_get_backlight();
                if (level > 0) odroid_display_set_backlight(--level);
                set_ingame_overlay(INGAME_OVERLAY_BRIGHTNESS);
            }
            else if(joystick->values[ODROID_INPUT_A]){
                // Save State
                last_key = ODROID_INPUT_A;
                odroid_audio_mute(true);

                // Call ingame overlay so that the save icon gets displayed first.
                set_ingame_overlay(INGAME_OVERLAY_SAVE);

                odroid_system_emu_save_state(0);
                odroid_audio_mute(false);
                common_emu_state.startup_frames = 0;
            }
            else if(joystick->values[ODROID_INPUT_B]){
                // Load State
                last_key = ODROID_INPUT_B;
                odroid_system_emu_load_state(0);
                common_emu_state.startup_frames = 0;
                set_ingame_overlay(INGAME_OVERLAY_LOAD);
            }
            else if(joystick->values[ODROID_INPUT_X]){
                last_key = ODROID_INPUT_X;
                odroid_audio_mute(true);
                //change turbo
                uint8_t turbo_key = odroid_settings_turbo_buttons_get();
                turbo_key ^= 1;
                odroid_settings_turbo_buttons_set(turbo_key);
                set_ingame_overlay(INGAME_OVERLAY_BUTTON_A);
                odroid_audio_mute(false);
                common_emu_state.startup_frames = 0;
            }
            else if(joystick->values[ODROID_INPUT_Y]){
                last_key = ODROID_INPUT_Y;
                odroid_audio_mute(true);
                //change turbo
                uint8_t turbo_key = odroid_settings_turbo_buttons_get();
                turbo_key ^= 2;
                odroid_settings_turbo_buttons_set(turbo_key);
                set_ingame_overlay(INGAME_OVERLAY_BUTTON_B);
                odroid_audio_mute(false);
                common_emu_state.startup_frames = 0;
            }
        }

        if (last_key >= 0) {
            macro_activated = true;
            if (!joystick->values[last_key]) {
                last_key = -1;
            }

            // Consume all inputs so it doesn't get passed along to the
            // running emulator
            memset(joystick, '\x00', sizeof(odroid_gamepad_state_t));
        }

        // Refresh the last_overlay_time so that it won't disappear until after
        // PAUSE/SET has been released.
        common_emu_state.last_overlay_time = get_elapsed_time();
    }
    else if (pause_pressed && !joystick->values[ODROID_INPUT_VOLUME] && !macro_activated){
        // PAUSE/SET has been released without performing any macro. Launch menu
        pause_pressed = false;

        odroid_overlay_game_menu(game_options, _repaint);
        clear_frames = 2;

        common_emu_state.startup_frames = 0;
        cpumon_stats.last_busy = 0;
    }
    else if (!joystick->values[ODROID_INPUT_VOLUME]){
        pause_pressed = false;
        macro_activated = false;
        last_key = -1;
    }

    if (ingame_overlay_loop()) {
        clear_frames = 2;
    }

    if (clear_frames) {
        clear_frames--;
        lcd_sleep_while_swap_pending();

        // Clear the active screen buffer, caller must repaint it
        lcd_clear_active_buffer();
    }

    if (joystick->values[ODROID_INPUT_POWER]) {
        // Save-state and poweroff
        audio_stop_playing();
#if OFF_SAVESTATE==1
        app->saveState("1");
#else
        app->saveState("0");
#endif
        odroid_system_sleep();
    }

    if (common_emu_state.pause_after_frames > 0) {
        (common_emu_state.pause_after_frames)--;
        if (common_emu_state.pause_after_frames == 0) {
            pause_pressed = true;
        }
    }
}

void common_emu_input_loop_handle_turbo(odroid_gamepad_state_t *joystick) {
    uint8_t turbo_buttons = odroid_settings_turbo_buttons_get();
    bool turbo_a = (joystick->values[ODROID_INPUT_A] && (turbo_buttons & 1));
    bool turbo_b = (joystick->values[ODROID_INPUT_B] && (turbo_buttons & 2));
    bool turbo_button = odroid_button_turbos();
    if (turbo_a)
        joystick->values[ODROID_INPUT_A] = turbo_button;
    if (turbo_b)
        joystick->values[ODROID_INPUT_B] = !turbo_button;
}

__attribute__((optimize("unroll-loops"))) static void draw_multisync_status() {
    uint16_t color = multisync_is_synchronized() ? 0b0000011111100000 : 0b1111100000000000; // Green & red
    // The inactive buffer is actually the active buffer since swap is normally already called at this point.
    // And if not then a direct inactive buffer write is also what we need (in case of a skipped frame)
    pixel_t *fb = lcd_get_inactive_buffer();
    for (uint16_t x = 0; x < GW_LCD_WIDTH; x++) {
        fb[x + GW_LCD_WIDTH * 0] = color;
        fb[x + GW_LCD_WIDTH * (GW_LCD_HEIGHT - 1)] = color;
    }
    for (uint16_t y = 0; y < GW_LCD_HEIGHT; y++) {
        fb[0 + GW_LCD_WIDTH * y] = color;
        fb[(GW_LCD_WIDTH - 1) + GW_LCD_WIDTH * y] = color;
    }
}

void common_emu_sound_sync(bool use_nops) {
    if (odroid_settings_DebugMenuMultisyncDebug_get()) {
        draw_multisync_status();
    }

    if (!common_emu_state.skip_frames) {
#ifdef SALEAE_DEBUG_SIGNALS
        HAL_GPIO_WritePin(DEBUG_PORT_PIN_4, DEBUG_PIN_4, GPIO_PIN_SET);
#endif
        static uint32_t last_dma_counter = 0;
        if (last_dma_counter == 0) {
            last_dma_counter = dma_counter;
        }
        for (uint8_t p = 0; p < common_emu_state.pause_frames + 1; p++) {
            while (dma_counter == last_dma_counter) {
                if (use_nops) {
                    __NOP();
                } else {
                    cpumon_sleep();
                }
            }
            last_dma_counter = dma_counter;
        }
#ifdef SALEAE_DEBUG_SIGNALS
        HAL_GPIO_WritePin(DEBUG_PORT_PIN_4, DEBUG_PIN_4, GPIO_PIN_RESET);
#endif
    }
#ifdef SALEAE_DEBUG_SIGNALS
    HAL_GPIO_TogglePin(DEBUG_PORT_PIN_5, DEBUG_PIN_5);
#endif
}

bool common_emu_sound_loop_is_muted() {
    if (audio_mute || odroid_audio_volume_get() == ODROID_AUDIO_VOLUME_MIN) {
        audio_clear_active_buffer();
        return true;
    }
    return false;
}

uint8_t common_emu_sound_get_volume() {
    return volume_tbl[odroid_audio_volume_get()];
}

/* DWT counter used to measure time execution */
volatile unsigned int *DWT_CONTROL = (unsigned int *)0xE0001000;
volatile unsigned int *DWT_CYCCNT = (unsigned int *)0xE0001004;
volatile unsigned int *DEMCR = (unsigned int *)0xE000EDFC;
volatile unsigned int *LAR = (unsigned int *)0xE0001FB0; // <-- lock access register

void common_emu_enable_dwt_cycles()
{
    /* Use DWT cycle counter to get precision time elapsed during loop.
       The DWT cycle counter is cleared on every loop.
       It may crash if the DWT is used during trace profiling */

    *DEMCR = *DEMCR | 0x01000000;    // enable trace
    *LAR = 0xC5ACCE55;               // <-- added unlock access to DWT (ITM, etc.)registers
    *DWT_CYCCNT = 0;                 // clear DWT cycle counter
    *DWT_CONTROL = *DWT_CONTROL | 1; // enable DWT cycle counter
}

inline __attribute__((always_inline))
unsigned int common_emu_get_dwt_cycles() {
    return *DWT_CYCCNT;
}

inline __attribute__((always_inline))
void common_emu_clear_dwt_cycles() {
    *DWT_CYCCNT = 0;
}

static void cpumon_common(bool sleep){
    uint t0 = get_elapsed_time();
    if(cpumon_stats.last_busy){
        cpumon_stats.busy_ms += t0 - cpumon_stats.last_busy;
    }
    else{
        cpumon_stats.busy_ms = 0;
    }
    if(sleep) __WFI();
    uint t1 = get_elapsed_time();
    cpumon_stats.last_busy = t1;
    cpumon_stats.sleep_ms += t1 - t0;
}


void cpumon_busy(void){
    cpumon_common(false);
}

void cpumon_sleep(void){
    cpumon_common(true);
}

void cpumon_reset(void){
    cpumon_stats.busy_ms = 0;
    cpumon_stats.sleep_ms = 0;
}

#define OVERLAY_COLOR_565 0xFFFF

static const uint8_t ROUND[] = {  // This is the top/left of a 8-pixel radius circle
    0b00000001,
    0b00000111,
    0b00001111,
    0b00011111,
    0b00111111,
    0b01111111,
    0b01111111,
    0b11111111,
};

// These must be multiples of 8
#define IMG_H 24
#define IMG_W 24


__attribute__((optimize("unroll-loops")))
static void draw_img(pixel_t *fb, const uint8_t *img, uint16_t x, uint16_t y){
    uint16_t idx = 0;
    for(uint8_t i=0; i < IMG_H; i++) {
        for(uint8_t j=0; j < IMG_W; j++) {
            if(img[idx / 8] & (1 << (7 - idx % 8))){
                fb[x + j +  GW_LCD_WIDTH * (y + i)] = OVERLAY_COLOR_565;
            }
            idx++;
        }
    }
}

#define DARKEN_MASK_565 0x7BEF  // Mask off the MSb of each color
#define DARKEN_ADD_565 0x2104  // value of 4-red, 8-green, 4-blue to add back in a little gray, especially on black backgrounds
static inline void darken_pixel(pixel_t *p){
    // Quickly divide all colors by 2
    *p = ((*p >> 1) & DARKEN_MASK_565) + DARKEN_ADD_565;
}

__attribute__((optimize("unroll-loops")))
static void draw_rectangle(pixel_t *fb, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    for(uint16_t i=y1; i < y2; i++){
        for(uint16_t j=x1; j < x2; j++){
            fb[j + GW_LCD_WIDTH * i] = OVERLAY_COLOR_565;
        }
    }
}

__attribute__((optimize("unroll-loops")))
static void draw_darken_rectangle(pixel_t *fb, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    for(uint16_t i=y1; i < y2; i++){
        for(uint16_t j=x1; j < x2; j++){
            darken_pixel(&fb[j + GW_LCD_WIDTH * i]);
        }
    }
}

__attribute__((optimize("unroll-loops")))
static void draw_darken_rounded_rectangle(pixel_t *fb, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2){
    // *1 is inclusive, *2 is exclusive
    uint16_t h = y2 - y1;
    uint16_t w = x2 - x1;
    if (w < 16 || h < 16) {
        // Draw not rounded rectangle
        draw_darken_rectangle(fb, x1, y1, x2, y2);
        return;
    }

    // Draw upper left round
    for(uint8_t i=0; i < 8; i++) for(uint8_t j=0; j < 8; j++)
        if(ROUND[i] & (1 << (7 - j))) darken_pixel(&fb[x1 + j + GW_LCD_WIDTH * (y1 + i)]);

    // Draw upper right round
    for(uint8_t i=0; i < 8; i++) for(uint8_t j=0; j < 8; j++)
        if(ROUND[i] & (1 << (7 - j))) darken_pixel(&fb[x2 - j - 1 + GW_LCD_WIDTH * (y1 + i)]);

    // Draw lower left round
    for(uint8_t i=0; i < 8; i++) for(uint8_t j=0; j < 8; j++)
        if(ROUND[i] & (1 << (7 - j))) darken_pixel(&fb[x1 + j + GW_LCD_WIDTH * (y2 - i - 1)]);

    // Draw lower right round
    for(uint8_t i=0; i < 8; i++) for(uint8_t j=0; j < 8; j++)
        if(ROUND[i] & (1 <<  (7 - j))) darken_pixel(&fb[x2 - j - 1 + GW_LCD_WIDTH * (y2 - i - 1)]);

    // Draw upper rectangle
    for(uint16_t i=x1+8; i < x2 - 8; i++) for(uint8_t j=0; j < 8; j++)
        darken_pixel(&fb[ i + GW_LCD_WIDTH * (y1 + j)]);

    // Draw central rectangle
    for(uint16_t i=x1; i < x2; i++) for(uint16_t j=y1+8; j < y2-8; j++)
        darken_pixel(&fb[i+GW_LCD_WIDTH * j]);

    // Draw lower rectangle
    for(uint16_t i=x1+8; i < x2 - 8; i++) for(uint8_t j=0; j < 8; j++)
        darken_pixel(&fb[ i + GW_LCD_WIDTH * (y2 - j - 1)]);
}

#define INGAME_OVERLAY_X 265
#define INGAME_OVERLAY_Y 10
#define INGAME_OVERLAY_BARS_H 128
#define INGAME_OVERLAY_W 39
#define INGAME_OVERLAY_BORDER 4
#define INGAME_OVERLAY_BOX_GAP 2

#define INGAME_OVERLAY_BARS_W INGAME_OVERLAY_W
#define INGAME_OVERLAY_IMG_H  (IMG_H + 2 * INGAME_OVERLAY_BORDER)  // For when only an image is showing

#define INGAME_OVERLAY_BOX_W (INGAME_OVERLAY_BARS_W - (2 * INGAME_OVERLAY_BORDER) - 6)
#define INGAME_OVERLAY_BOX_X (INGAME_OVERLAY_X + ((INGAME_OVERLAY_BARS_W - INGAME_OVERLAY_BOX_W) / 2))
#define INGAME_OVERLAY_BOX_Y (INGAME_OVERLAY_Y + INGAME_OVERLAY_BORDER + 3)

// For when only an image is shown
#define INGAME_OVERLAY_IMG_X (INGAME_OVERLAY_X + ((INGAME_OVERLAY_BARS_W - IMG_W) / 2))
#define INGAME_OVERLAY_IMG_Y (INGAME_OVERLAY_Y + INGAME_OVERLAY_BORDER)

// Places the image at the bottom for bars-related overlay (volume, brightness)
#define INGAME_OVERLAY_BARS_IMG_X INGAME_OVERLAY_IMG_X
#define INGAME_OVERLAY_BARS_IMG_Y (INGAME_OVERLAY_Y + INGAME_OVERLAY_BARS_H - IMG_H - INGAME_OVERLAY_BORDER)

#define DARKEN_IMG_ONLY() draw_darken_rounded_rectangle(fb, \
                    INGAME_OVERLAY_X, \
                    INGAME_OVERLAY_Y, \
                    INGAME_OVERLAY_X + INGAME_OVERLAY_BARS_W, \
                    INGAME_OVERLAY_Y + INGAME_OVERLAY_IMG_H)


static uint8_t box_height(uint8_t n) {
    return ((INGAME_OVERLAY_BARS_IMG_Y - INGAME_OVERLAY_BOX_Y) / n) - INGAME_OVERLAY_BOX_GAP;
}

void common_ingame_overlay(void) {
    rg_app_desc_t *app = odroid_system_get_app();
    pixel_t *fb = lcd_get_active_buffer();
    int8_t level;
    uint8_t bh;
    uint8_t turbo_key;
    uint16_t by = INGAME_OVERLAY_BOX_Y;

    uint16_t percentage = odroid_input_read_battery().percentage;
    if (percentage <= 15) {
        if ((get_elapsed_time() % 1000) < 300)
            odroid_overlay_draw_battery(150, 90);
    }

    switch(common_emu_state.overlay)
    {
        case INGAME_OVERLAY_NONE:
            break;
        case INGAME_OVERLAY_VOLUME:
            level = odroid_audio_volume_get();
            bh = box_height(ODROID_AUDIO_VOLUME_MAX);

            draw_darken_rounded_rectangle(fb,
                    INGAME_OVERLAY_X,
                    INGAME_OVERLAY_Y,
                    INGAME_OVERLAY_X + INGAME_OVERLAY_BARS_W,
                    INGAME_OVERLAY_Y + INGAME_OVERLAY_BARS_H);
            draw_img(fb, IMG_SPEAKER, INGAME_OVERLAY_BARS_IMG_X, INGAME_OVERLAY_BARS_IMG_Y);

            for(int8_t i=ODROID_AUDIO_VOLUME_MAX; i > 0; i--){
                if(i <= level)
                    draw_rectangle(fb,
                            INGAME_OVERLAY_BOX_X,
                            by,
                            INGAME_OVERLAY_BOX_X + INGAME_OVERLAY_BOX_W,
                            by + bh);
                else
                    draw_darken_rectangle(fb,
                            INGAME_OVERLAY_BOX_X,
                            by,
                            INGAME_OVERLAY_BOX_X + INGAME_OVERLAY_BOX_W,
                            by + bh);

                by += bh + INGAME_OVERLAY_BOX_GAP;
            }
            break;
        case INGAME_OVERLAY_BRIGHTNESS:
            level = odroid_display_get_backlight();
            bh = box_height(ODROID_BACKLIGHT_LEVEL_COUNT - 1);

            draw_darken_rounded_rectangle(fb,
                    INGAME_OVERLAY_X,
                    INGAME_OVERLAY_Y,
                    INGAME_OVERLAY_X + INGAME_OVERLAY_BARS_W,
                    INGAME_OVERLAY_Y + INGAME_OVERLAY_BARS_H);
            draw_img(fb, IMG_SUN, INGAME_OVERLAY_BARS_IMG_X, INGAME_OVERLAY_BARS_IMG_Y);

            for(int8_t i=ODROID_BACKLIGHT_LEVEL_COUNT-1; i > 0; i--){
                if(i <= level)
                    draw_rectangle(fb,
                            INGAME_OVERLAY_BOX_X,
                            by,
                            INGAME_OVERLAY_BOX_X + INGAME_OVERLAY_BOX_W,
                            by + bh);
                else
                    draw_darken_rectangle(fb,
                            INGAME_OVERLAY_BOX_X,
                            by,
                            INGAME_OVERLAY_BOX_X + INGAME_OVERLAY_BOX_W,
                            by + bh);

                by += bh + INGAME_OVERLAY_BOX_GAP;
            }
            break;
        case INGAME_OVERLAY_LOAD:
            DARKEN_IMG_ONLY();
            draw_img(fb, IMG_FOLDER, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
            break;
        case INGAME_OVERLAY_SAVE:
            DARKEN_IMG_ONLY();
            draw_img(fb, IMG_DISKETTE, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
            break;
        case INGAME_OVERLAY_SC:
            DARKEN_IMG_ONLY();
            draw_img(fb, IMG_SC, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
            break;
        case INGAME_OVERLAY_BUTTON_A:
            DARKEN_IMG_ONLY();
            turbo_key = odroid_settings_turbo_buttons_get();
            if (turbo_key & 1)
                draw_img(fb, IMG_BUTTON_A_P, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
            else
                draw_img(fb, IMG_BUTTON_A, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
            break;
        case INGAME_OVERLAY_BUTTON_B:
            DARKEN_IMG_ONLY();
            turbo_key = odroid_settings_turbo_buttons_get();
            if (turbo_key & 2)
                draw_img(fb, IMG_BUTTON_B_P, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
            else
                draw_img(fb, IMG_BUTTON_B, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
            break;
        case INGAME_OVERLAY_SPEEDUP:
            DARKEN_IMG_ONLY();
            switch(app->speedupEnabled){
                case SPEEDUP_0_5x:
                    draw_img(fb, IMG_0_5X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
                case SPEEDUP_0_75x:
                    draw_img(fb, IMG_0_75X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
                case SPEEDUP_1x:
                    draw_img(fb, IMG_1X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
                case SPEEDUP_1_25x:
                    draw_img(fb, IMG_1_25X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
                case SPEEDUP_1_5x:
                    draw_img(fb, IMG_1_5X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
                case SPEEDUP_2x:
                    draw_img(fb, IMG_2X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
                case SPEEDUP_3x:
                    draw_img(fb, IMG_3X, INGAME_OVERLAY_IMG_X, INGAME_OVERLAY_IMG_Y);
                    break;
            }
            break;

    }
}

static void set_ingame_overlay(ingame_overlay_t type){
    common_emu_state.overlay = type;
    common_emu_state.last_overlay_time = get_elapsed_time();
}

#define OVERLAY_COLOR_565 0xFFFF
#define BORDER_COLOR_565 0x1082  // Dark Dark Gray

#define BORDER_HEIGHT 240
#define BORDER_WIDTH 32

#define BORDER_Y_OFFSET (((GW_LCD_HEIGHT) - (BORDER_HEIGHT)) / 2)

void draw_border_zelda3(pixel_t * fb){
    uint32_t start, bit_index;
    start = 0;
    bit_index = 0;
    for(uint16_t i=0; i < BORDER_HEIGHT; i++){
        uint32_t offset = start + i * GW_LCD_WIDTH;
        for(uint8_t j=0; j < BORDER_WIDTH; j++){
            fb[offset + j] =
                (IMG_BORDER_ZELDA3[bit_index >> 3] << (bit_index & 0x07)) & 0x80 ? BORDER_COLOR_565 : 0x0000;
            bit_index++;
        }
    }
    start = 32 + 256;
    bit_index = 0;
    for(uint16_t i=0; i < BORDER_HEIGHT; i++){
        uint32_t offset = start + i * GW_LCD_WIDTH;
        for(uint8_t j=0; j < BORDER_WIDTH; j++){
            fb[offset + j] =
                (IMG_BORDER_ZELDA3[bit_index >> 3] << (bit_index & 0x07)) & 0x80 ? BORDER_COLOR_565 : 0x0000;
            bit_index++;
        }
    }

}

void draw_border_smw(pixel_t * fb){
    uint32_t start, bit_index;
    start = 0;
    bit_index = 0;
    for(uint16_t i=0; i < BORDER_HEIGHT; i++){
        uint32_t offset = start + i * GW_LCD_WIDTH;
        for(uint8_t j=0; j < BORDER_WIDTH; j++){
            fb[offset + j] =
                (IMG_BORDER_LEFT_SMW[bit_index >> 3] << (bit_index & 0x07)) & 0x80 ? BORDER_COLOR_565 : 0x0000;
            bit_index++;
        }
    }
    start = 32 + 256;
    bit_index = 0;
    for(uint16_t i=0; i < BORDER_HEIGHT; i++){
        uint32_t offset = start + i * GW_LCD_WIDTH;
        for(uint8_t j=0; j < BORDER_WIDTH; j++){
            fb[offset + j] =
                (IMG_BORDER_RIGHT_SMW[bit_index >> 3] << (bit_index & 0x07)) & 0x80 ? BORDER_COLOR_565 : 0x0000;
            bit_index++;
        }
    }
}

void snes_upscale(pixel_t * fb) {
  const int x_ratio = 52428; // Precomputed (256 << 16) / 320
  const int y_ratio = 61166; // Precomputed (224 << 16) / 240

  for (int y = 240 - 1; y >= 0; --y) {
    int src_y = (y * y_ratio) >> 16; // Compute source row
    uint16_t* dest_row = fb + y * 320; // Destination row pointer
    const uint16_t* src_row = fb + src_y * 320; // Source row pointer

    for (int x = 320 - 1; x >= 0; --x) {
      int src_x = (x * x_ratio) >> 16; // Compute source column
      dest_row[x] = src_row[src_x];
    }
  }
}
