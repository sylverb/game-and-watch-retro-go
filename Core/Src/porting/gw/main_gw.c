#include "build/config.h"

#ifdef ENABLE_EMULATOR_GW
#include <odroid_system.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "main.h"
#include "gw_lcd.h"
#include "gw_linker.h"
#include "gw_buttons.h"
#include "appid.h"
#include "filesystem.h"

/* TO move elsewhere */
#include "stm32h7xx_hal.h"

#include "common.h"
#include "rom_manager.h"
#include "rg_i18n.h"
#include "gui.h"
#include "rg_rtc.h"

/* G&W system support */
#include "gw_system.h"
#include "gw_romloader.h"

/* access to internals for debug purpose */
#include "sm510.h"

/* Uncomment to enable debug menu in overlay */
//#define GW_EMU_DEBUG_OVERLAY


#define ODROID_APPID_GW 6

/* Audio buffer length */
#define GW_AUDIO_BUFFER_LENGTH_DMA ((2 * GW_AUDIO_FREQ) / GW_REFRESH_RATE)

/* keys inpus (hw & sw) */
static odroid_gamepad_state_t joystick;
static bool softkey_time_pressed = 0;
static bool softkey_alarm_pressed = 0;
static bool softkey_A_pressed = 0;
static bool softkey_only = 0;

static unsigned int softkey_duration = 0;

static void gw_set_time() {

    struct tm tm;
    GW_GetUnixTM(&tm);

    gw_time_t time;
    time.hours = tm.tm_hour;
    time.minutes = tm.tm_min;
    time.seconds = tm.tm_sec;

    // set time of the emulated system
    gw_system_set_time(time);
    printf("Set time done!\n");
}

static void gw_get_time() {

    gw_time_t time = {0};

    // check if the system is able to get the time
    time = gw_system_get_time();
    if (time.hours > 24) return;

    // Set times
    struct tm tm;
    GW_GetUnixTM(&tm);
    tm.tm_hour = time.hours;
    tm.tm_min = time.minutes;
    tm.tm_sec = time.seconds;
    GW_SetUnixTM(&tm);
}

static void gw_check_time() {

    static unsigned int is_gw_time_sync=0;

    struct tm tm;
    GW_GetUnixTM(&tm);

    // Set times
    gw_time_t time;
    time.hours = tm.tm_hour;
    time.minutes = tm.tm_min;
    time.seconds = tm.tm_sec;

    // update time every 30s
    if ( (time.seconds == 30) || (is_gw_time_sync==0) ) {
        is_gw_time_sync = 1;
        gw_system_set_time(time);
    }
}
static unsigned char state_save_buffer[sizeof(gw_state_t)];

static bool gw_system_SaveState(char *pathName)
{
    printf("Saving state...\n");
    fs_file_t *file;
    file = fs_open(pathName, FS_WRITE, FS_COMPRESS);

    memset(state_save_buffer, '\x00', sizeof(state_save_buffer));
    gw_state_save(state_save_buffer);

    fs_write(file, state_save_buffer, sizeof(state_save_buffer));
    fs_close(file);

    return false;
}

static bool gw_system_LoadState(char *pathName)
{
    printf("Loading state...\n");
    fs_file_t *file;
    file = fs_open(pathName, FS_READ, FS_COMPRESS);
    fs_read(file, state_save_buffer, sizeof(state_save_buffer));
    fs_close(file);
    return gw_state_load((unsigned char *)state_save_buffer);

}

/* callback to get buttons state */
unsigned int gw_get_buttons()
{
    unsigned int hw_buttons = 0;
    if (!softkey_only)
    {
        hw_buttons |= joystick.values[ODROID_INPUT_LEFT];
        hw_buttons |= joystick.values[ODROID_INPUT_UP] << 1;
        hw_buttons |= joystick.values[ODROID_INPUT_RIGHT] << 2;
        hw_buttons |= joystick.values[ODROID_INPUT_DOWN] << 3;
        hw_buttons |= joystick.values[ODROID_INPUT_A] << 4;
        hw_buttons |= joystick.values[ODROID_INPUT_B] << 5;
        hw_buttons |= joystick.values[ODROID_INPUT_SELECT] << 6;
        hw_buttons |= joystick.values[ODROID_INPUT_START] << 7;
        hw_buttons |= joystick.values[ODROID_INPUT_VOLUME] << 8;
        hw_buttons |= joystick.values[ODROID_INPUT_POWER] << 9;
        hw_buttons |= joystick.values[ODROID_INPUT_X] << 10;
        hw_buttons |= joystick.values[ODROID_INPUT_Y] << 11;
    }

    // software keys
    hw_buttons |= ((unsigned int)softkey_A_pressed) << 4;
    hw_buttons |= ((unsigned int)softkey_time_pressed) << 10;
    hw_buttons |= ((unsigned int)softkey_alarm_pressed) << 11;

    return hw_buttons;
}

static void gw_sound_init()
{

    /* init emulator sound system with shared audio buffer */
    gw_system_sound_init();

    /* clear DMA audio buffer */
    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));

    /* Start SAI DMA */
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, GW_AUDIO_BUFFER_LENGTH_DMA);
}

static void gw_sound_submit()
{

    uint8_t volume = odroid_audio_volume_get();
    int16_t factor = volume_tbl[volume];

    /** Enables the following code to track audio rendering issues **/
    /*
    if (gw_audio_buffer_idx < GW_AUDIO_BUFFER_LENGTH) {
        printf("audio underflow:%u < %u \n",gw_audio_buffer_idx , GW_AUDIO_BUFFER_LENGTH);
        assert(0);
    }

    if (gw_audio_buffer_idx > (GW_AUDIO_BUFFER_LENGTH +12) ) {
        printf("audio oveflow:%u < %u \n",gw_audio_buffer_idx , GW_AUDIO_BUFFER_LENGTH);
        assert(0);
    }
    */

    size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : GW_AUDIO_BUFFER_LENGTH;

    if (audio_mute || volume == ODROID_AUDIO_VOLUME_MIN)
    {
        for (int i = 0; i < GW_AUDIO_BUFFER_LENGTH; i++)
        {
            audiobuffer_dma[i + offset] = 0;
        }
    }
    else
    {
        for (int i = 0; i < GW_AUDIO_BUFFER_LENGTH; i++)
        {
            audiobuffer_dma[i + offset] = (factor) * (gw_audio_buffer[i] << 4);
        }
    }

    gw_audio_buffer_copied = true;
}

/************************ Debug function in overlay START *******************************/

/* performance monitoring */
/* Emulator loop monitoring
    ( unit is 1/systemcoreclock 1/280MHz )
    loop_cycles
        -measured duration of the loop.
    proc_cycles
        - estimated duration of emulated CPU for a bunch of emulated system clock.
    blit_cycles
        - estimated duration of graphics rendering.
    end_cycles
        - estimated duration of overall processing.
    */

static unsigned int loop_cycles = 1, end_cycles = 1, proc_cycles = 1, blit_cycles = 1;

/* DWT counter used to measure time execution */
volatile unsigned int *DWT_CONTROL = (unsigned int *)0xE0001000;
volatile unsigned int *DWT_CYCCNT = (unsigned int *)0xE0001004;
volatile unsigned int *DEMCR = (unsigned int *)0xE000EDFC;
volatile unsigned int *LAR = (unsigned int *)0xE0001FB0; // <-- lock access register

#define get_dwt_cycles() *DWT_CYCCNT
#define clear_dwt_cycles() *DWT_CYCCNT = 0

#ifdef GW_EMU_DEBUG_OVERLAY
static void enable_dwt_cycles()
{

    /* Use DWT cycle counter to get precision time elapsed during loop.
    The DWT cycle counter is cleared on every loop
    it may crash if the DWT is used during trace profiling */

    *DEMCR = *DEMCR | 0x01000000;    // enable trace
    *LAR = 0xC5ACCE55;               // <-- added unlock access to DWT (ITM, etc.)registers
    *DWT_CYCCNT = 0;                 // clear DWT cycle counter
    *DWT_CONTROL = *DWT_CONTROL | 1; // enable DWT cycle counter
}
#endif

static void gw_debug_bar()
{

#ifdef GW_EMU_DEBUG_OVERLAY
    static unsigned int loop_duration_us = 1, end_duration_us = 1, proc_duration_us = 1, blit_duration_us = 1;
    static const unsigned int SYSTEM_CORE_CLOCK_MHZ = 280;

    static bool debug_init_done = false;

    if (!debug_init_done)
    {
        enable_dwt_cycles();
        debug_init_done = true;
    }

    static unsigned int overflow_count = 0;
    static unsigned int busy_percent = 0;

    char debugMsg[120];

    proc_duration_us = proc_cycles / SYSTEM_CORE_CLOCK_MHZ;
    blit_duration_us = blit_cycles / SYSTEM_CORE_CLOCK_MHZ;
    end_duration_us = end_cycles / SYSTEM_CORE_CLOCK_MHZ;
    loop_duration_us = loop_cycles / SYSTEM_CORE_CLOCK_MHZ;

    busy_percent = 100 * (proc_duration_us + blit_duration_us) / loop_duration_us;

    if (end_duration_us > 1000000 / GW_REFRESH_RATE)
        overflow_count++;

    if (m_halt != 0)
        sprintf(debugMsg, "%04dus EMU:%04dus FX:%04dus %d%%+%d HALT", loop_duration_us, proc_duration_us, blit_duration_us, busy_percent, overflow_count);
    else
        sprintf(debugMsg, "%04dus EMU:%04dus FX:%04dus %d%%+%d", loop_duration_us, proc_duration_us, blit_duration_us, busy_percent, overflow_count);

    odroid_overlay_draw_text(0, 0, GW_SCREEN_WIDTH, debugMsg, curr_colors->sel_c, curr_colors->main_c);

#endif
}
/************************ Debug function in overlay END ********************************/

/************************ G&W options Menu ********************************/
// Press Auto Clear ACL
// Auto Set Time
// Press TIME
// Press ALARM

static bool gw_debug_submenu_autoclear(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    if (event == ODROID_DIALOG_ENTER)
        gw_system_reset();

    return event == ODROID_DIALOG_ENTER;
}

static bool gw_debug_submenu_autoset_time(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{

    if (event == ODROID_DIALOG_ENTER)
    {
        gw_set_time();
    }

    return event == ODROID_DIALOG_ENTER;
}

static bool gw_debug_submenu_autoget_time(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{

    if (event == ODROID_DIALOG_ENTER)
    {
        gw_get_time();
    }

    return event == ODROID_DIALOG_ENTER;
}

static bool gw_debug_submenu_press_time(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    if (event == ODROID_DIALOG_ENTER)
    {
        softkey_time_pressed = 1;
        softkey_duration = GW_REFRESH_RATE;
    }
    return event == ODROID_DIALOG_ENTER;
}

static bool gw_debug_submenu_press_alarm(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    if (event == ODROID_DIALOG_ENTER)
    {
        softkey_alarm_pressed = 1;
        softkey_duration = GW_REFRESH_RATE;
    }
    return event == ODROID_DIALOG_ENTER;
}


static char LCD_deflicker_value[10];
static bool gw_debug_submenu_set_deflicker(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    /* LCD deflicker filter level */
    /*
    0 : filter is disabled
    1 : refreshed on keys polling and call subroutine return
    2 : refreshed on keys polling only
    */
    unsigned int max_flag_lcd_deflicker_level = 2;

    if (event == ODROID_DIALOG_PREV)
        flag_lcd_deflicker_level = flag_lcd_deflicker_level > 0 ? flag_lcd_deflicker_level - 1 : max_flag_lcd_deflicker_level;

    if (event == ODROID_DIALOG_NEXT)
        flag_lcd_deflicker_level = flag_lcd_deflicker_level < max_flag_lcd_deflicker_level ? flag_lcd_deflicker_level + 1 : 0;

    if (flag_lcd_deflicker_level == 0) strcpy(option->value, curr_lang->s_filter_0_none);
    if (flag_lcd_deflicker_level == 1) strcpy(option->value, curr_lang->s_filter_1_medium);
    if (flag_lcd_deflicker_level == 2) strcpy(option->value, curr_lang->s_filter_2_high);

    return event == ODROID_DIALOG_ENTER;
}

// Debug menu strings

static char display_ram_value[10];

// Display RAM bool
static unsigned int debug_display_ram = 0;
static bool gw_debug_submenu_display_ram(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT)
        debug_display_ram = debug_display_ram == 0 ? 1 : 0;

    if (debug_display_ram == 0) strcpy(option->value, curr_lang->s_No);
    if (debug_display_ram == 1) strcpy(option->value, curr_lang->s_Yes);

    return event == ODROID_DIALOG_ENTER;
}

static char draw_line_content[1+2*17];

static void gw_display_ram_overlay(){

  //  char *p;
   // p = (char *)&draw_line_content[0];
    sprintf(draw_line_content, "   0 1 2 3 4 5 6 7 8 9 A B C D E F");
    odroid_overlay_draw_text(10, 72, 300, draw_line_content, curr_colors->sel_c, curr_colors->main_c);

    for (unsigned char i=0;i<8;i++) {
        sprintf(draw_line_content, "%2u%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x%2x",i, \
        gw_ram[i*16], gw_ram[(i*16)+1], gw_ram[(i*16)+2],gw_ram[(i*16)+3],gw_ram[(i*16)+4],gw_ram[(i*16)+5],gw_ram[(i*16)+6],gw_ram[(i*16)+7], \
        gw_ram[(i*16)+8], gw_ram[(i*16)+9], gw_ram[(i*16)+10],gw_ram[(i*16)+11],gw_ram[(i*16)+12],gw_ram[(i*16)+13],gw_ram[(i*16)+14],gw_ram[(i*16)+15]);
    odroid_overlay_draw_text(10, 80+8*i, 300, draw_line_content, curr_colors->sel_c, curr_colors->main_c);
    }
}


/* Main */
int app_main_gw(uint8_t load_state, int8_t save_slot)
{

    odroid_dialog_choice_t options[] = {
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {309, curr_lang->s_Press_ACL, "", 1, &gw_debug_submenu_autoclear},
        {310, curr_lang->s_Press_TIME, "", 1, &gw_debug_submenu_press_time},
        {320, curr_lang->s_Press_ALARM, "", 1, &gw_debug_submenu_press_alarm},
        {330, curr_lang->s_copy_RTC_to_GW_time, "", 1, &gw_debug_submenu_autoset_time},
        {331, curr_lang->s_copy_GW_time_to_RTC, "", 1, &gw_debug_submenu_autoget_time},
        {360, curr_lang->s_LCD_filter, LCD_deflicker_value, 1, &gw_debug_submenu_set_deflicker},
        {370, curr_lang->s_Display_RAM, display_ram_value, 1, &gw_debug_submenu_display_ram},
        ODROID_DIALOG_CHOICE_LAST};

    odroid_system_init(ODROID_APPID_GW, GW_AUDIO_FREQ);
    odroid_system_emu_init(&gw_system_LoadState, &gw_system_SaveState, NULL);
    //rg_app_desc_t *app = odroid_system_get_app();
    static unsigned previous_m_halt = 2;

    common_emu_state.frame_time_10us = (uint16_t)(100000 / GW_REFRESH_RATE + 0.5f);

    /*** load ROM  */
    bool rom_status = gw_system_romload();

    if (!rom_status)
        odroid_system_switch_app(0);

    /*** Clear audio buffer */
    gw_sound_init();
    printf("Sound initialized\n");

    /* clear soft keys */
    softkey_time_pressed = 0;
    softkey_alarm_pressed = 0;
    softkey_duration = 0;

    /*** Configure the emulated system */
    gw_system_config();
    printf("G&W configured\n");

    /*** Start and Reset the emulated system */
    gw_system_start();
    printf("G&W start\n");

    gw_system_reset();
    printf("G&W reset\n");

    /* check if we have to load state */
    bool LoadState_done = false;
    if (load_state != 0) {
        LoadState_done = odroid_system_emu_load_state(save_slot);

        if (LoadState_done) {
            gw_check_time();
            gw_set_time();
        }
    }

    /* emulate watch mode */
    if (!LoadState_done) {
        softkey_time_pressed = 0;
        softkey_alarm_pressed = 0;
        softkey_A_pressed = 0;

        // disable user keys
        softkey_only = 1;

        printf("G&W emulate watch mode\n");

        gw_system_reset();

        // From reset state : run
        gw_system_run(GW_AUDIO_FREQ*2);

        // press TIME to exit TIME settings mode
        softkey_time_pressed = 1;
        gw_system_run(GW_AUDIO_FREQ/2);
        softkey_time_pressed = 0;
        gw_system_run(GW_AUDIO_FREQ*2);

        // synchronize G&W with RTC and run
        gw_check_time();
        gw_set_time();
        gw_system_run(GW_AUDIO_FREQ);

        // press A required by some game
        softkey_A_pressed = 1;
        gw_system_run(GW_AUDIO_FREQ/2);
        softkey_A_pressed = 0;
        gw_system_run(GW_AUDIO_FREQ);

        // enable user keys
        softkey_only = 0;
    }

    /*** Main emulator loop */
    printf("Main emulator loop start\n");

    clear_dwt_cycles();

    while (true)
    {
        /* clear DWT counter used to monitor performances */
        clear_dwt_cycles();

        wdog_refresh();

        /* refresh internal G&W timer on emulated CPU state transition */
        if (previous_m_halt != m_halt) gw_check_time();

        previous_m_halt = m_halt;

        //hardware keys
        odroid_input_read_gamepad(&joystick);

        //soft keys emulation
        if (softkey_duration > 0)
            softkey_duration--;

        if (softkey_duration == 0)
        {
            softkey_time_pressed = 0;
            softkey_alarm_pressed = 0;
        }

        void _blit()
        {
            gw_system_blit(lcd_get_active_buffer());
            common_ingame_overlay();
        }

        common_emu_input_loop(&joystick, options, &_blit);

        bool drawFrame = common_emu_frame_loop();

        /* Emulate and Blit */
        // Call the emulator function with number of clock cycles
        // to execute on the emulated device
        gw_system_run(GW_SYSTEM_CYCLES);

        /* get how many cycles have been spent in the emulator */
        proc_cycles = get_dwt_cycles();

        /* update the screen only if there is no pending frame to render */
        if (!lcd_is_swap_pending() && drawFrame)
        {
            _blit();
            gw_debug_bar();
            if(debug_display_ram == 1) gw_display_ram_overlay();
            lcd_swap();

            /* get how many cycles have been spent in graphics rendering */
            blit_cycles = get_dwt_cycles() - proc_cycles;
        }
        /****************************************************************************/

        /* copy audio samples for DMA */
        if (drawFrame)
        {
            gw_sound_submit();
        }

        /* get how many cycles have been spent to process everything */
        end_cycles = get_dwt_cycles();

        if (!common_emu_state.skip_frames)
        {
            for (uint8_t p = 0; p < common_emu_state.pause_frames + 1; p++)
            {
                static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
                while (dma_state == last_dma_state)
                {
#ifdef GW_EMU_DEBUG_OVERLAY
                    __NOP();
#else
                    cpumon_sleep();
#endif
                }
                last_dma_state = dma_state;
            }
        }

        /* get how cycles have been spent inside this loop */
        loop_cycles = get_dwt_cycles();

    } // end of loop
}

#endif
