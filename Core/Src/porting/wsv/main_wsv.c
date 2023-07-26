#include "build/config.h"

#ifdef ENABLE_EMULATOR_WSV
#include <odroid_system.h>

#include <assert.h>
#include "gw_lcd.h"
#include "gw_linker.h"
#include "gw_buttons.h"
#include "rom_manager.h"
#include "common.h"
#include "lz4_depack.h"
#include "miniz.h"
#include "lzma.h"
#include "appid.h"
#include "bilinear.h"
#include "rg_i18n.h"
#include "filesystem.h"

#include "wsv_sound.h"
#include "memorymap.h"
#include "supervision.h"
#include "controls.h"
#include "types.h"

#define WIDTH 320
#define WSV_WIDTH (160)
#define WSV_HEIGHT (160)
static uint8_t wsv_framebuffer[WSV_WIDTH*WSV_HEIGHT*2];

static odroid_video_frame_t video_frame = {WSV_WIDTH, WSV_HEIGHT, WSV_WIDTH * 2, 2, 0xFF, -1, NULL, NULL, 0, {}};

#define WSV_FPS 50 // Real hardware is 50.81fps
#define WSV_AUDIO_BUFFER_LENGTH (SV_SAMPLE_RATE / WSV_FPS)
#define AUDIO_BUFFER_LENGTH_DMA_WSV (2 * WSV_AUDIO_BUFFER_LENGTH)
static int8 audioBuffer_wsv[WSV_AUDIO_BUFFER_LENGTH*2]; // *2 as emulator is filling stereo buffer
#define WSV_ROM_BUFF_LENGTH 0x80000 // Largest Watara Supervision Rom is 512kB (Journey to the West)
// Memory to handle compressed roms
static uint8 wsv_rom_memory[WSV_ROM_BUFF_LENGTH];

static void netplay_callback(netplay_event_t event, void *arg) {
    // Where we're going we don't need netplay!
}

#define STATE_SAVE_BUFFER_LENGTH (1024 * 28)

static bool LoadState(char *pathName) {
    fs_file_t *file;
    file = fs_open(pathName, FS_READ, FS_COMPRESS);
    fs_read(file, wsv_framebuffer, STATE_SAVE_BUFFER_LENGTH);
    fs_close(file);
    supervision_load_state(wsv_framebuffer);
    return 0;
}
static bool SaveState(char *pathName) {
    int size = supervision_save_state(wsv_framebuffer);
    assert(size<STATE_SAVE_BUFFER_LENGTH);

    fs_file_t *file;
    file = fs_open(pathName, FS_WRITE, FS_COMPRESS);
    fs_write(file, wsv_framebuffer, size);
    fs_close(file);
    return 0;
}

void wsv_pcm_submit() {
    uint8_t volume = odroid_audio_volume_get();
    int32_t factor = volume_tbl[volume]/2; // Divide by 2 to prevent overflow in stereo mixing

    supervision_update_sound((uint8 *)audioBuffer_wsv,WSV_AUDIO_BUFFER_LENGTH*2);
    size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : WSV_AUDIO_BUFFER_LENGTH;
    if (audio_mute || volume == ODROID_AUDIO_VOLUME_MIN) {
        for (int i = 0; i < WSV_AUDIO_BUFFER_LENGTH; i++) {
            audiobuffer_dma[offset + i] = 0;
        }
    } else {
        for (int i = 0; i < WSV_AUDIO_BUFFER_LENGTH; i++) {
            /* mix left & right */
            int32_t sample = audioBuffer_wsv[2*i] + audioBuffer_wsv[2*i+1];
            audiobuffer_dma[offset + i] = (sample * factor);
        }
    }
}

__attribute__((optimize("unroll-loops")))
static inline void screen_blit_nn(int32_t dest_width, int32_t dest_height)
{
    static uint32_t lastFPSTime = 0;
    static uint32_t frames = 0;
    uint32_t currentTime = HAL_GetTick();
    uint32_t delta = currentTime - lastFPSTime;

    frames++;

    if (delta >= 1000) {
        int fps = (10000 * frames) / delta;
        printf("FPS: %d.%d, frames %ld, delta %ld ms, skipped %d\n", fps / 10, fps % 10, delta, frames, common_emu_state.skipped_frames);
        frames = 0;
        common_emu_state.skipped_frames = 0;
        lastFPSTime = currentTime;
    }

    int w1 = video_frame.width;
    int h1 = video_frame.height;
    int w2 = dest_width;
    int h2 = dest_height;

    int x_ratio = (int)((w1<<16)/w2) +1;
    int y_ratio = (int)((h1<<16)/h2) +1;
    int hpad = (320 - dest_width) / 2;
    int wpad = (240 - dest_height) / 2;

    int x2;
    int y2;

    uint16_t *screen_buf = (uint16_t*)video_frame.buffer;
    uint16_t *dest = lcd_get_active_buffer();

    PROFILING_INIT(t_blit);
    PROFILING_START(t_blit);

    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            x2 = ((j*x_ratio)>>16) ;
            y2 = ((i*y_ratio)>>16) ;
            uint16_t b2 = screen_buf[(y2*w1)+x2];
            dest[((i+wpad)*WIDTH)+j+hpad] = b2;
        }
    }

    PROFILING_END(t_blit);

#ifdef PROFILING_ENABLED
    printf("Blit: %d us\n", (1000000 * PROFILING_DIFF(t_blit)) / t_blit_t0.SecondFraction);
#endif
    common_ingame_overlay();

    lcd_swap();
}

static void screen_blit_bilinear(int32_t dest_width)
{
    static uint32_t lastFPSTime = 0;
    static uint32_t frames = 0;
    uint32_t currentTime = HAL_GetTick();
    uint32_t delta = currentTime - lastFPSTime;

    frames++;

    if (delta >= 1000) {
        int fps = (10000 * frames) / delta;
        printf("FPS: %d.%d, frames %ld, delta %ld ms, skipped %d\n", fps / 10, fps % 10, delta, frames, common_emu_state.skipped_frames);
        frames = 0;
        common_emu_state.skipped_frames = 0;
        lastFPSTime = currentTime;
    }

    int w1 = video_frame.width;
    int h1 = video_frame.height;

    int w2 = dest_width;
    int h2 = 240;
    int stride = 320;
    int hpad = (320 - dest_width) / 2;

    uint16_t *dest = lcd_get_active_buffer();

    image_t dst_img;
    dst_img.w = dest_width;
    dst_img.h = 240;
    dst_img.bpp = 2;
    dst_img.pixels = ((uint8_t *) dest) + hpad * 2;

    if (hpad > 0) {
        memset(dest, 0x00, hpad * 2);
    }

    image_t src_img;
    src_img.w = video_frame.width;
    src_img.h = video_frame.height;
    src_img.bpp = 2;
    src_img.pixels = video_frame.buffer;

    float x_scale = ((float) w2) / ((float) w1);
    float y_scale = ((float) h2) / ((float) h1);



    PROFILING_INIT(t_blit);
    PROFILING_START(t_blit);

    imlib_draw_image(&dst_img, &src_img, 0, 0, stride, x_scale, y_scale, NULL, -1, 255, NULL,
                     NULL, IMAGE_HINT_BILINEAR, NULL, NULL);

    PROFILING_END(t_blit);

#ifdef PROFILING_ENABLED
    printf("Blit: %d us\n", (1000000 * PROFILING_DIFF(t_blit)) / t_blit_t0.SecondFraction);
#endif
    common_ingame_overlay();

    lcd_swap();
}

static inline void screen_blit_v3to5(void) {
    static uint32_t lastFPSTime = 0;
    static uint32_t frames = 0;
    uint32_t currentTime = HAL_GetTick();
    uint32_t delta = currentTime - lastFPSTime;

    frames++;

    if (delta >= 1000) {
        int fps = (10000 * frames) / delta;
        printf("FPS: %d.%d, frames %ld, delta %ld ms, skipped %d\n", fps / 10, fps % 10, delta, frames, common_emu_state.skipped_frames);
        frames = 0;
        common_emu_state.skipped_frames = 0;
        lastFPSTime = currentTime;
    }

    uint16_t *dest = lcd_get_active_buffer();

    PROFILING_INIT(t_blit);
    PROFILING_START(t_blit);

#define CONV(_b0)    (((0b11111000000000000000000000&_b0)>>10) | ((0b000001111110000000000&_b0)>>5) | ((0b0000000000011111&_b0)))
#define EXPAND(_b0)  (((0b1111100000000000 & _b0) << 10) | ((0b0000011111100000 & _b0) << 5) | ((0b0000000000011111 & _b0)))

    int y_src = 0;
    int y_dst = 0;
    int w = video_frame.width;
    int h = video_frame.height;
    for (; y_src < h; y_src += 3, y_dst += 5) {
        int x_src = 0;
        int x_dst = 0;
        for (; x_src < w; x_src += 1, x_dst += 2) {
            uint16_t *src_col = &((uint16_t *)video_frame.buffer)[(y_src * w) + x_src];
            uint32_t b0 = EXPAND(src_col[w * 0]);
            uint32_t b1 = EXPAND(src_col[w * 1]);
            uint32_t b2 = EXPAND(src_col[w * 2]);

            dest[((y_dst + 0) * WIDTH) + x_dst] = CONV(b0);
            dest[((y_dst + 1) * WIDTH) + x_dst] = CONV((b0+b1)>>1);
            dest[((y_dst + 2) * WIDTH) + x_dst] = CONV(b1);
            dest[((y_dst + 3) * WIDTH) + x_dst] = CONV((b1+b2)>>1);
            dest[((y_dst + 4) * WIDTH) + x_dst] = CONV(b2);

            dest[((y_dst + 0) * WIDTH) + x_dst + 1] = CONV(b0);
            dest[((y_dst + 1) * WIDTH) + x_dst + 1] = CONV((b0+b1)>>1);
            dest[((y_dst + 2) * WIDTH) + x_dst + 1] = CONV(b1);
            dest[((y_dst + 3) * WIDTH) + x_dst + 1] = CONV((b1+b2)>>1);
            dest[((y_dst + 4) * WIDTH) + x_dst + 1] = CONV(b2);
        }
    }

    PROFILING_END(t_blit);

#ifdef PROFILING_ENABLED
    printf("Blit: %d us\n", (1000000 * PROFILING_DIFF(t_blit)) / t_blit_t0.SecondFraction);
#endif
    common_ingame_overlay();

    lcd_swap();
}


static inline void screen_blit_jth(void) {
    static uint32_t lastFPSTime = 0;
    static uint32_t frames = 0;
    uint32_t currentTime = HAL_GetTick();
    uint32_t delta = currentTime - lastFPSTime;

    frames++;

    if (delta >= 1000) {
        int fps = (10000 * frames) / delta;
        printf("FPS: %d.%d, frames %ld, delta %ld ms, skipped %d\n", fps / 10, fps % 10, delta, frames, common_emu_state.skipped_frames);
        frames = 0;
        common_emu_state.skipped_frames = 0;
        lastFPSTime = currentTime;
    }


    uint16_t* screen_buf = (uint16_t*)video_frame.buffer;
    uint16_t *dest = lcd_get_active_buffer();

    PROFILING_INIT(t_blit);
    PROFILING_START(t_blit);


    int w1 = video_frame.width;
    int h1 = video_frame.height;
    int w2 = 320;
    int h2 = 240;

    const int border = 24;

    // Iterate on dest buf rows
    for(int y = 0; y < border; ++y) {
        uint16_t *src_row  = &screen_buf[y * w1];
        uint16_t *dest_row = &dest[y * w2];
        for (int x = 0, xsrc=0; x < w2; x+=2,xsrc++) {
            dest_row[x]     = src_row[xsrc];
            dest_row[x + 1] = src_row[xsrc];
        }
    }

    for (int y = border, src_y = border; y < h2-border; y+=2, src_y++) {
        uint16_t *src_row  = &screen_buf[src_y * w1];
        uint32_t *dest_row0 = (uint32_t *) &dest[y * w2];
        for (int x = 0, xsrc=0; x < w2; x++,xsrc++) {
            uint32_t col = src_row[xsrc];
            dest_row0[x] = (col | (col << 16));
        }
    }

    for (int y = border, src_y = border; y < h2-border; y+=2, src_y++) {
        uint16_t *src_row  = &screen_buf[src_y * w1];
        uint32_t *dest_row1 = (uint32_t *)&dest[(y + 1) * w2];
        for (int x = 0, xsrc=0; x < w2; x++,xsrc++) {
            uint32_t col = src_row[xsrc];
            dest_row1[x] = (col | (col << 16));
        }
    }

    for(int y = 0; y < border; ++y) {
        uint16_t *src_row  = &screen_buf[(h1-border+y) * w1];
        uint16_t *dest_row = &dest[(h2-border+y) * w2];
        for (int x = 0, xsrc=0; x < w2; x+=2,xsrc++) {
            dest_row[x]     = src_row[xsrc];
            dest_row[x + 1] = src_row[xsrc];
        }
    }


    PROFILING_END(t_blit);

#ifdef PROFILING_ENABLED
    printf("Blit: %d us\n", (1000000 * PROFILING_DIFF(t_blit)) / t_blit_t0.SecondFraction);
#endif
    common_ingame_overlay();

    lcd_swap();
}

static void blit(void)
{
    odroid_display_scaling_t scaling = odroid_display_get_scaling_mode();
    odroid_display_filter_t filtering = odroid_display_get_filter_mode();

    switch (scaling) {
    case ODROID_DISPLAY_SCALING_OFF:
        // Original Resolution
        screen_blit_nn(160, 160);
        break;
    case ODROID_DISPLAY_SCALING_FIT:
        // Full height, borders on the side
        switch (filtering) {
        case ODROID_DISPLAY_FILTER_OFF:
            /* fall-through */
        case ODROID_DISPLAY_FILTER_SHARP:
            // crisp nearest neighbor scaling
            screen_blit_nn(266, 240);
            break;
        case ODROID_DISPLAY_FILTER_SOFT:
            // soft bilinear scaling
            screen_blit_bilinear(266);
            break;
        default:
            printf("Unknown filtering mode %d\n", filtering);
            assert(!"Unknown filtering mode");
        }
        break;
        break;
    case ODROID_DISPLAY_SCALING_FULL:
        // full height, full width
        switch (filtering) {
        case ODROID_DISPLAY_FILTER_OFF:
            // crisp nearest neighbor scaling
            screen_blit_nn(320, 240);
            break;
        case ODROID_DISPLAY_FILTER_SHARP:
            // sharp bilinear-ish scaling
            screen_blit_v3to5();
            break;
        case ODROID_DISPLAY_FILTER_SOFT:
            // soft bilinear scaling
            screen_blit_bilinear(320);
            break;
        default:
            printf("Unknown filtering mode %d\n", filtering);
            assert(!"Unknown filtering mode");
        }
        break;
    case ODROID_DISPLAY_SCALING_CUSTOM:
        // compressed top and bottom sections, full width
        screen_blit_jth();
        break;
    default:
        printf("Unknown scaling mode %d\n", scaling);
        assert(!"Unknown scaling mode");
        break;
    }
}

void wsv_render_image() {
    // WSV image is 160x160
    int y;
    pixel_t *framebuffer_active = lcd_get_active_buffer();
    int offset = 0;
    for (y=0;y<40;y++) {
        memset(framebuffer_active+y*GW_LCD_WIDTH,0x00,GW_LCD_WIDTH*2);
    }
    for (y=40;y<200;y++) {
        memset(framebuffer_active+y*GW_LCD_WIDTH,0x00,80*2);
        memcpy(framebuffer_active+y*GW_LCD_WIDTH+80,wsv_framebuffer+offset,160*2);
        memset(framebuffer_active+y*GW_LCD_WIDTH+80+160,0x00,80*2);
        offset+=320;
    }
    for (y=200;y<240;y++) {
        memset(framebuffer_active+y*GW_LCD_WIDTH,0x00,GW_LCD_WIDTH*2);
    }
}


static bool palette_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{

    const char *palette_names[] = {
        curr_lang->s_wsv_palette_Default, 
        curr_lang->s_wsv_palette_Amber, 
        curr_lang->s_wsv_palette_Green, 
        curr_lang->s_wsv_palette_Blue, 
        curr_lang->s_wsv_palette_BGB, 
        curr_lang->s_wsv_palette_Wataroo};
    
    int8 wsv_pal = supervision_get_color_scheme();
    int max = SV_COLOR_SCHEME_COUNT - 1;

    if (event == ODROID_DIALOG_PREV) wsv_pal = wsv_pal > 0 ? wsv_pal - 1 : max;
    if (event == ODROID_DIALOG_NEXT) wsv_pal = wsv_pal < max ? wsv_pal + 1 : 0;

    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        odroid_settings_Palette_set(wsv_pal);
        supervision_set_color_scheme(wsv_pal);
    }

    strcpy(option->value, palette_names[wsv_pal]);

    return event == ODROID_DIALOG_ENTER;
}

void wsv_input_read(odroid_gamepad_state_t *joystick) {
    uint8 controls_state = 0x00;
    if (joystick->values[ODROID_INPUT_LEFT])   controls_state|=0x02;
    if (joystick->values[ODROID_INPUT_RIGHT])  controls_state|=0x01;
    if (joystick->values[ODROID_INPUT_UP])     controls_state|=0x08;
    if (joystick->values[ODROID_INPUT_DOWN])   controls_state|=0x04;
    if (joystick->values[ODROID_INPUT_A])      controls_state|=0x20;
    if (joystick->values[ODROID_INPUT_B])      controls_state|=0x10;
    if (joystick->values[ODROID_INPUT_START] || joystick->values[ODROID_INPUT_X])  controls_state|=0x80;
    if (joystick->values[ODROID_INPUT_SELECT] || joystick->values[ODROID_INPUT_Y]) controls_state|=0x40;
    supervision_set_input(controls_state);
}

size_t wsv_getromdata(unsigned char **data) {
    /* src pointer to the ROM data in the external flash (raw or LZ4) */
    const unsigned char *src = ROM_DATA;
    unsigned char *dest = (unsigned char *)wsv_rom_memory;

    if(strcmp(ROM_EXT, "lzma") == 0){
        size_t n_decomp_bytes;
        n_decomp_bytes = lzma_inflate(dest, WSV_ROM_BUFF_LENGTH, src, ROM_DATA_LENGTH);
        *data = dest;
        return n_decomp_bytes;
    } else {
        *data = (unsigned char *)ROM_DATA;
        return ROM_DATA_LENGTH;
    }
}

int app_main_wsv(uint8_t load_state, uint8_t start_paused, uint8_t save_slot)
{
    char pal_name[16];
    uint32 rom_length = 0;
    uint8 *rom_ptr = NULL;
    odroid_gamepad_state_t joystick;
    odroid_dialog_choice_t options[] = {
        {100, curr_lang->s_Palette, pal_name, 1, &palette_update_cb},
        ODROID_DIALOG_CHOICE_LAST
    };

    if (start_paused) {
        common_emu_state.pause_after_frames = 2;
        odroid_audio_mute(true);
    } else {
        common_emu_state.pause_after_frames = 0;
    }
    common_emu_state.frame_time_10us = (uint16_t)(100000 / WSV_FPS + 0.5f);

    video_frame.buffer = wsv_framebuffer;
    memset(framebuffer1, 0, sizeof(framebuffer1));
    memset(framebuffer2, 0, sizeof(framebuffer2));

    odroid_system_init(APPID_WSV, SV_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, &netplay_callback);

    // Init Sound
    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, AUDIO_BUFFER_LENGTH_DMA_WSV );

    supervision_set_color_scheme(SV_COLOR_SCHEME_DEFAULT);

    supervision_init(); //Init the emulator

    rom_length = wsv_getromdata(&rom_ptr);
    supervision_load(rom_ptr, rom_length);

    if (load_state) {
        odroid_system_emu_load_state(save_slot);
    }
    while(1)
    {
        wdog_refresh();
        bool drawFrame = common_emu_frame_loop();

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

        wsv_input_read(&joystick);

        supervision_exec((uint16 *)wsv_framebuffer);
        if (drawFrame) {
            blit();
        }
        wsv_pcm_submit();
        if(!common_emu_state.skip_frames){
            static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
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
