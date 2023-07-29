#include "build/config.h"
#ifdef ENABLE_EMULATOR_MSX

#include <odroid_system.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>


#include "main.h"
#include "appid.h"

#include "stm32h7xx_hal.h"

#include "common.h"
#include "rom_manager.h"
#include "gw_lcd.h"
#include "rg_i18n.h"

#include <assert.h>
#include "lzma.h"

#include "MSX.h"
#include "Properties.h"
#include "ArchFile.h"
#include "VideoRender.h"
#include "AudioMixer.h"
#include "SlotManager.h"
#include "Casette.h"
#include "PrinterIO.h"
#include "UartIO.h"
#include "MidiIO.h"
#include "Machine.h"
#include "Board.h"
#include "Emulator.h"
#include "FileHistory.h"
#include "Actions.h"
#include "Language.h"
#include "LaunchFile.h"
#include "ArchEvent.h"
#include "ArchSound.h"
#include "ArchNotifications.h"
#include "JoystickPort.h"
#include "InputEvent.h"
#include "R800.h"
#include "save_msx.h"
#include "gw_malloc.h"
#include "gw_linker.h"
#include "main_msx.h"

extern BoardInfo boardInfo;
static Properties* properties;
static Machine *msxMachine;
static Mixer* mixer;

enum{
   FREQUENCY_VDP_AUTO = 0,
   FREQUENCY_VDP_50HZ,
   FREQUENCY_VDP_60HZ
};

enum{
   MSX_GAME_ROM = 0,
   MSX_GAME_DISK,
   MSX_GAME_HDIDE
};

static int msx_game_type = MSX_GAME_ROM;
// Default is MSX2+
static int selected_msx_index = 2;
// Default is Automatic
static int selected_frequency_index = FREQUENCY_VDP_AUTO;

static odroid_gamepad_state_t previous_joystick_state;
int msx_button_left_key = EC_LEFT;
int msx_button_right_key = EC_RIGHT;
int msx_button_up_key = EC_UP;
int msx_button_down_key = EC_DOWN;
int msx_button_a_key = EC_SPACE;
int msx_button_b_key = EC_CTRL;
int msx_button_game_key = EC_RETURN;
int msx_button_time_key = EC_CTRL;
int msx_button_start_key = EC_RETURN;
int msx_button_select_key = EC_CTRL;

static bool show_disk_icon = false;
static int selected_disk_index = 0;
#define MSX_DISK_EXTENSION "cdk"

static int selected_key_index = 0;

/* strings for options */
static char disk_name[128];
static char msx_name[11];
static char key_name[10];
static char frequency_name[10];
static char a_button_name[10];
static char b_button_name[10];

/* Volume management */
static int8_t currentVolume = -1;
static const uint8_t volume_table[ODROID_AUDIO_VOLUME_MAX + 1] = {
    0,
    6,
    12,
    19,
    25,
    35,
    46,
    60,
    80,
    100,
};

/* Compression buffer */
static uint8_t rom_decompress_buffer[136*1024];

/* Framebuffer management */
uint8_t msx_framebuffer[272*240];
static unsigned image_buffer_base_width;
static unsigned image_buffer_current_width;
static unsigned image_buffer_height;
static unsigned width = 272;
static unsigned height = 240;
static int double_width;
static bool use_overscan = true;
static int msx2_dif = 0;
static uint16_t palette565[256];

#define FPS_NTSC  60
#define FPS_PAL   50
static int8_t msx_fps = FPS_PAL;

#define AUDIO_MSX_SAMPLE_RATE 18000

static const uint8_t IMG_DISKETTE[] = {
    0x00, 0x00, 0x00, 0x3F, 0xFF, 0xE0, 0x7C, 0x00, 0x70, 0x7C, 0x03, 0x78,
    0x7C, 0x03, 0x7C, 0x7C, 0x03, 0x7E, 0x7C, 0x00, 0x7E, 0x7F, 0xFF, 0xFE,
    0x7F, 0xFF, 0xFE, 0x7F, 0xFF, 0xFE, 0x7F, 0xFF, 0xFE, 0x7F, 0xFF, 0xFE,
    0x7F, 0xFF, 0xFE, 0x7E, 0x00, 0x7E, 0x7C, 0x00, 0x3E, 0x7C, 0x00, 0x3E,
    0x7D, 0xFF, 0xBE, 0x7C, 0x00, 0x3E, 0x7C, 0x00, 0x3E, 0x7D, 0xFF, 0xBE,
    0x7C, 0x00, 0x3E, 0x7C, 0x00, 0x3E, 0x3F, 0xFF, 0xFC, 0x00, 0x00, 0x00,
};

static Int32 soundWrite(void* dummy, Int16 *buffer, UInt32 count);
static void createMsxMachine(int msxType);
static void setPropertiesMsx(Machine *machine, int msxType);
static void setupEmulatorRessources(int msxType);
static void createProperties();

void msxLedSetFdd1(int state) {
    show_disk_icon = state;
}

static bool msx_system_LoadState(char *pathName)
{
    loadMsxState(pathName);  // internally calls load_gnw_msx_data
    return true;
}

static bool msx_system_SaveState(char *pathName)
{
    // Show disk icon when saving state
    uint16_t *dest = lcd_get_inactive_buffer();
    uint16_t idx = 0;
    for (uint8_t i = 0; i < 24; i++) {
        for (uint8_t j = 0; j < 24; j++) {
        if (IMG_DISKETTE[idx / 8] & (1 << (7 - idx % 8))) {
            dest[274 + j + GW_LCD_WIDTH * (2 + i)] = 0xFFFF;
        }
        idx++;
        }
    }
    saveMsxState(pathName); // internally calls save_gnw_msx_data
    return true;
}

void save_gnw_msx_data() {
    SaveState* state;
    state = saveStateOpenForWrite("main_msx");
    saveStateSet(state, "selected_msx_index", selected_msx_index);
    saveStateSet(state, "selected_disk_index", selected_disk_index);
    saveStateSet(state, "msx_button_a_key", msx_button_a_key);
    saveStateSet(state, "msx_button_b_key", msx_button_b_key);
    saveStateSet(state, "selected_frequency_index", selected_frequency_index);
    saveStateSet(state, "selected_key_index", selected_key_index);
    saveStateSet(state, "msx_fps", msx_fps);
    saveStateClose(state);
}

void load_gnw_msx_data() {
    SaveState* state;
    state = saveStateOpenForRead("main_msx");
    selected_msx_index = saveStateGet(state, "selected_msx_index", 0);
    selected_disk_index = saveStateGet(state, "selected_disk_index", 0);
    msx_button_a_key = saveStateGet(state, "msx_button_a_key", 0);
    msx_button_b_key = saveStateGet(state, "msx_button_b_key", 0);
    selected_frequency_index = saveStateGet(state, "selected_frequency_index", 0);
    selected_key_index = saveStateGet(state, "selected_key_index", 0);
    msx_fps = saveStateGet(state, "msx_fps", 0);
    saveStateClose(state);
}

/* Core stubs */
void frameBufferDataDestroy(FrameBufferData* frameData){}
void frameBufferSetActive(FrameBufferData* frameData){}
void frameBufferSetMixMode(FrameBufferMixMode mode, FrameBufferMixMode mask){}
void frameBufferClearDeinterlace(){}
void archTrap(UInt8 value){}
void videoUpdateAll(Video* video, Properties* properties){}

/* framebuffer */

static void update_fb_info() {
    width  = use_overscan ? 272 : (272 - 16);
    height = use_overscan ? 240 : (240 - 48 + (msx2_dif * 2));
}

Pixel* frameBufferGetLine(FrameBuffer* frameBuffer, int y)
{
   return (msx_framebuffer +  (y * image_buffer_current_width));
}

Pixel16* frameBufferGetLine16(FrameBuffer* frameBuffer, int y)
{
   return (lcd_get_active_buffer() + sizeof(Pixel16) * (y * GW_LCD_WIDTH + 24));
}

FrameBuffer* frameBufferGetDrawFrame(void)
{
   return (void*)msx_framebuffer;
}

FrameBuffer* frameBufferFlipDrawFrame(void)
{
   return (void*)msx_framebuffer;
}

static int fbScanLine = 0;

void frameBufferSetScanline(int scanline)
{
   fbScanLine = scanline;
}

int frameBufferGetScanline(void)
{
   return fbScanLine;
}

FrameBufferData* frameBufferDataCreate(int maxWidth, int maxHeight, int defaultHorizZoom)
{
   return (void*)msx_framebuffer;
}

FrameBufferData* frameBufferGetActive()
{
    return (void*)msx_framebuffer;
}

void frameBufferSetLineCount(FrameBuffer* frameBuffer, int val)
{
    image_buffer_height = val;
}

int frameBufferGetLineCount(FrameBuffer* frameBuffer) {
    return image_buffer_height;
}

int frameBufferGetMaxWidth(FrameBuffer* frameBuffer)
{
    return FB_MAX_LINE_WIDTH;
}

int frameBufferGetDoubleWidth(FrameBuffer* frameBuffer, int y)
{
    return double_width;
}

void frameBufferSetDoubleWidth(FrameBuffer* frameBuffer, int y, int val)
{
    double_width = val;
}

/** GuessROM() ***********************************************/
/** Guess MegaROM mapper of a ROM.                          **/
/*************************************************************/
int GuessROM(const uint8_t *buf,int size)
{
    int i;
    int counters[6] = { 0, 0, 0, 0, 0, 0 };

    int mapper;

    /* No result yet */
    mapper = ROM_UNKNOWN;

    if (size <= 0x10000) {
        if (size == 0x10000) {
            if (buf[0x4000] == 'A' && buf[0x4001] == 'B') mapper = ROM_PLAIN;
            else mapper = ROM_ASCII16;
            return mapper;
        } 
        
        if (size <= 0x4000 && buf[0] == 'A' && buf[1] == 'B') {
            UInt16 text = buf[8] + 256 * buf[9];
            if ((text & 0xc000) == 0x8000) {
                return ROM_BASIC;
            }
        }
        return ROM_PLAIN;
    }

    /* Count occurences of characteristic addresses */
    for (i = 0; i < size - 3; i++) {
        if (buf[i] == 0x32) {
            UInt32 value = buf[i + 1] + ((UInt32)buf[i + 2] << 8);

            switch(value) {
            case 0x4000: 
            case 0x8000: 
            case 0xa000: 
                counters[3]++;
                break;

            case 0x5000: 
            case 0x9000: 
            case 0xb000: 
                counters[2]++;
                break;

            case 0x6000: 
                counters[3]++;
                counters[4]++;
                counters[5]++;
                break;

            case 0x6800: 
            case 0x7800: 
                counters[4]++;
                break;

            case 0x7000: 
                counters[2]++;
                counters[4]++;
                counters[5]++;
                break;

            case 0x77ff: 
                counters[5]++;
                break;
            }
        }
    }

    /* Find which mapper type got more hits */
    mapper = 0;

    counters[4] -= counters[4] ? 1 : 0;

    for (i = 0; i <= 5; i++) {
        if (counters[i] > 0 && counters[i] >= counters[mapper]) {
            mapper = i;
        }
    }

    if (mapper == 5 && counters[0] == counters[5]) {
        mapper = 0;
    }

    switch (mapper) {
        default:
        case 0: mapper = ROM_STANDARD; break;
        case 1: mapper = ROM_MSXDOS2; break;
        case 2: mapper = ROM_KONAMI5; break;
        case 3: mapper = ROM_KONAMI4; break;
        case 4: mapper = ROM_ASCII8; break;
        case 5: mapper = ROM_ASCII16; break;
    }

    /* Return the most likely mapper type */
    return(mapper);
}

static bool update_disk_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    char game_name[PROP_MAXPATH];
    int disk_count = 0;
    int max_index = 0;
    retro_emulator_file_t *disk_file = NULL;
    const rom_system_t *msx_system = rom_manager_system(&rom_mgr, "MSX");
    disk_count = rom_get_ext_count(msx_system,MSX_DISK_EXTENSION);
    if (disk_count > 0) {
        max_index = disk_count - 1;
    } else {
        max_index = 0;
    }

    if (event == ODROID_DIALOG_PREV) {
        selected_disk_index = selected_disk_index > 0 ? selected_disk_index - 1 : max_index;
    }
    if (event == ODROID_DIALOG_NEXT) {
        selected_disk_index = selected_disk_index < max_index ? selected_disk_index + 1 : 0;
    }

    disk_file = (retro_emulator_file_t *)rom_get_ext_file_at_index(msx_system,MSX_DISK_EXTENSION,selected_disk_index);
    if (disk_count > 0) {
        sprintf(game_name,"%s.%s",disk_file->name,disk_file->ext);
        emulatorSuspend();
        insertDiskette(properties, 0, game_name, NULL, -1);
        emulatorResume();
    }
    strcpy(option->value, disk_file->name);
    return event == ODROID_DIALOG_ENTER;
}

static bool update_frequency_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int max_index = 2;

    if (event == ODROID_DIALOG_PREV) {
        selected_frequency_index = selected_frequency_index > 0 ? selected_frequency_index - 1 : max_index;
    }
    if (event == ODROID_DIALOG_NEXT) {
        selected_frequency_index = selected_frequency_index < max_index ? selected_frequency_index + 1 : 0;
    }

    switch (selected_frequency_index) {
        case FREQUENCY_VDP_AUTO:
            strcpy(option->value, curr_lang->s_msx_Freq_Auto);
            break;
        case FREQUENCY_VDP_50HZ: // Force 50Hz
            strcpy(option->value, curr_lang->s_msx_Freq_50);
            break;
        case FREQUENCY_VDP_60HZ: // Force 60Hz
            strcpy(option->value, curr_lang->s_msx_Freq_60);
            break;
    }

    if (event == ODROID_DIALOG_ENTER) {
        switch (selected_frequency_index) {
            case FREQUENCY_VDP_AUTO:
                // Frequency update will be done at next loop if needed
                vdpSetSyncMode(VDP_SYNC_AUTO);
                break;
            case FREQUENCY_VDP_50HZ: // Force 50Hz;
                msx_fps = 50;
                common_emu_state.frame_time_10us = (uint16_t)(100000 / FPS_PAL + 0.5f);
                memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
                HAL_SAI_DMAStop(&hsai_BlockA1);
                HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, (2 * AUDIO_MSX_SAMPLE_RATE / msx_fps));
                emulatorRestartSound();
                vdpSetSyncMode(VDP_SYNC_50HZ);
                break;
            case FREQUENCY_VDP_60HZ: // Force 60Hz;
                msx_fps = 60;
                common_emu_state.frame_time_10us = (uint16_t)(100000 / FPS_NTSC + 0.5f);
                memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
                HAL_SAI_DMAStop(&hsai_BlockA1);
                HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, (2 * AUDIO_MSX_SAMPLE_RATE / msx_fps));
                emulatorRestartSound();
                vdpSetSyncMode(VDP_SYNC_60HZ);
                break;
        }
    }
    return event == ODROID_DIALOG_ENTER;
}

static bool update_msx_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int max_index = 2;

    if (event == ODROID_DIALOG_PREV) {
        selected_msx_index = selected_msx_index > 0 ? selected_msx_index - 1 : max_index;
    }
    if (event == ODROID_DIALOG_NEXT) {
        selected_msx_index = selected_msx_index < max_index ? selected_msx_index + 1 : 0;
    }

    switch (selected_msx_index) {
        case 0: // MSX1;
            msx2_dif = 0;
            strcpy(option->value, curr_lang->s_msx_MSX1_EUR);
            break;
        case 1: // MSX2;
            msx2_dif = 10;
            strcpy(option->value, curr_lang->s_msx_MSX2_EUR);
            break;
        case 2: // MSX2+;
            msx2_dif = 10;
            strcpy(option->value, curr_lang->s_msx_MSX2_JP);
            break;
    }

    if (event == ODROID_DIALOG_ENTER) {
        boardInfo.destroy();
        boardDestroy();
        ahb_init();
        itc_init();
        setupEmulatorRessources(selected_msx_index);
    }
    return event == ODROID_DIALOG_ENTER;
}

struct msx_key_info {
    int  key_id;
    const char *name;
    bool auto_release;
};

struct msx_key_info msx_keyboard[] = {
    {EC_F1,"F1",true}, // Index 0
    {EC_F2,"F2",true},
    {EC_F3,"F3",true},
    {EC_F4,"F4",true},
    {EC_F5,"F5",true},
    {EC_SPACE,"Space",true},
    {EC_LSHIFT,"Shift",false},
    {EC_CTRL,"Control",false},
    {EC_GRAPH,"Graph",true},
    {EC_BKSPACE,"BS",true},
    {EC_TAB,"Tab",true}, // Index 10
    {EC_CAPS,"CapsLock",true},
    {EC_CODE,"Code",true},
    {EC_SELECT,"Select",true},
    {EC_RETURN,"Return",true},
    {EC_CLS,"Home",true},
    {EC_DEL,"Delete",true},
    {EC_INS,"Insert",true},
    {EC_STOP,"Stop",true},
    {EC_ESC,"Esc",true},
    {EC_1,"1/!",true},
    {EC_2,"2/@",true}, // Index 20
    {EC_3,"3/#",true},
    {EC_4,"4/$",true},
    {EC_5,"5/\%",true},
    {EC_6,"6/^",true},
    {EC_7,"7/&",true},
    {EC_8,"8/*",true},
    {EC_9,"9/(",true},
    {EC_0,"0/)",true},
    {EC_NUM0,"0",true},
    {EC_NUM1,"1",true}, // Index 30
    {EC_NUM2,"2",true},
    {EC_NUM3,"3",true},
    {EC_NUM4,"4",true},
    {EC_NUM5,"5",true},
    {EC_NUM6,"6",true},
    {EC_NUM7,"7",true},
    {EC_NUM8,"8",true},
    {EC_NUM9,"9",true},
    {EC_A,"a",true},
    {EC_B,"b",true}, // Index 40
    {EC_C,"c",true},
    {EC_D,"d",true},
    {EC_E,"e",true},
    {EC_F,"f",true},
    {EC_G,"g",true},
    {EC_H,"h",true},
    {EC_I,"i",true},
    {EC_J,"j",true},
    {EC_K,"k",true},
    {EC_L,"l",true}, // Index 50
    {EC_M,"m",true},
    {EC_N,"n",true},
    {EC_O,"o",true},
    {EC_P,"p",true},
    {EC_Q,"q",true},
    {EC_R,"r",true},
    {EC_S,"s",true},
    {EC_T,"t",true},
    {EC_U,"u",true},
    {EC_V,"v",true}, // Index 60
    {EC_W,"w",true},
    {EC_X,"x",true},
    {EC_Y,"y",true},
    {EC_Z,"z",true},
    {EC_COLON,":",true},
    {EC_UNDSCRE,"_",true},
    {EC_DIV,"/",true},
    {EC_LBRACK,"[",true},
    {EC_RBRACK,"]",true}, // Index 70
    {EC_COMMA,",",true},
    {EC_PERIOD,".",true},
    {EC_CLS,"CLS",true},
    {EC_NEG,"-",true},
    {EC_CIRCFLX,"^",true},
    {EC_BKSLASH,"\\",true},
    {EC_AT,"AT",true},
    {EC_TORIKE,"Torike",true},
    {EC_JIKKOU,"Jikkou",true},
    {EC_UP,"UP",true},
    {EC_DOWN,"DOWN",true},
    {EC_LEFT,"LEFT",true},
    {EC_RIGHT,"RIGHT",true},
    {EC_JOY1_UP,"JOY1 UP"},
    {EC_JOY1_DOWN,"JOY1 DOWN"},
    {EC_JOY1_LEFT,"JOY1 LEFT"},
    {EC_JOY1_RIGHT,"JOY1 RIGHT"},
    {EC_JOY1_BUTTON1,"JOY1 A"},
    {EC_JOY1_BUTTON2,"JOY1 B"},
};

#define RELEASE_KEY_DELAY 5
static struct msx_key_info *pressed_key = NULL;
static struct msx_key_info *release_key = NULL;
static int release_key_delay = RELEASE_KEY_DELAY;
static bool update_keyboard_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int max_index = sizeof(msx_keyboard)/sizeof(msx_keyboard[0])-1;

    if (event == ODROID_DIALOG_PREV) {
        selected_key_index = selected_key_index > 0 ? selected_key_index - 1 : max_index;
    }
    if (event == ODROID_DIALOG_NEXT) {
        selected_key_index = selected_key_index < max_index ? selected_key_index + 1 : 0;
    }

    if (eventMap[msx_keyboard[selected_key_index].key_id]) {
        // If key is pressed, add a * in front of key name
        option->value[0] = '*';
        strcpy(option->value+1, msx_keyboard[selected_key_index].name);
    } else {
        strcpy(option->value, msx_keyboard[selected_key_index].name);
    }

    if (event == ODROID_DIALOG_ENTER) {
        pressed_key = &msx_keyboard[selected_key_index];
    }
    return event == ODROID_DIALOG_ENTER;
}

static bool update_a_button_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int max_index = sizeof(msx_keyboard)/sizeof(msx_keyboard[0])-1;
    int msx_button_key_index = 0;

    // find index for current key
    for (int i = 0; i <= max_index; i++) {
        if (msx_keyboard[i].key_id == msx_button_a_key) {
            msx_button_key_index = i;
            break;
        }
    }

    if (event == ODROID_DIALOG_PREV) {
        msx_button_key_index = msx_button_key_index > 0 ? msx_button_key_index - 1 : max_index;
    }
    if (event == ODROID_DIALOG_NEXT) {
        msx_button_key_index = msx_button_key_index < max_index ? msx_button_key_index + 1 : 0;
    }

    msx_button_a_key = msx_keyboard[msx_button_key_index].key_id;
    strcpy(option->value, msx_keyboard[msx_button_key_index].name);
    return event == ODROID_DIALOG_ENTER;
}

static bool update_b_button_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int max_index = sizeof(msx_keyboard)/sizeof(msx_keyboard[0])-1;
    int msx_button_key_index = 0;

    // find index for current key
    for (int i = 0; i <= max_index; i++) {
        if (msx_keyboard[i].key_id == msx_button_b_key) {
            msx_button_key_index = i;
            break;
        }
    }

    if (event == ODROID_DIALOG_PREV) {
        msx_button_key_index = msx_button_key_index > 0 ? msx_button_key_index - 1 : max_index;
    }
    if (event == ODROID_DIALOG_NEXT) {
        msx_button_key_index = msx_button_key_index < max_index ? msx_button_key_index + 1 : 0;
    }

    msx_button_b_key = msx_keyboard[msx_button_key_index].key_id;
    strcpy(option->value, msx_keyboard[msx_button_key_index].name);
    return event == ODROID_DIALOG_ENTER;
}

static void msxInputUpdate(odroid_gamepad_state_t *joystick)
{
    if ((joystick->values[ODROID_INPUT_LEFT]) && !previous_joystick_state.values[ODROID_INPUT_LEFT]) {
        eventMap[msx_button_left_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_LEFT]) && previous_joystick_state.values[ODROID_INPUT_LEFT]) {
        eventMap[msx_button_left_key]  = 0;
    }
    if ((joystick->values[ODROID_INPUT_RIGHT]) && !previous_joystick_state.values[ODROID_INPUT_RIGHT]) {
        eventMap[msx_button_right_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_RIGHT]) && previous_joystick_state.values[ODROID_INPUT_RIGHT]) {
        eventMap[msx_button_right_key]  = 0;
    }
    if ((joystick->values[ODROID_INPUT_UP]) && !previous_joystick_state.values[ODROID_INPUT_UP]) {
        eventMap[msx_button_up_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_UP]) && previous_joystick_state.values[ODROID_INPUT_UP]) {
        eventMap[msx_button_up_key]  = 0;
    }
    if ((joystick->values[ODROID_INPUT_DOWN]) && !previous_joystick_state.values[ODROID_INPUT_DOWN]) {
        eventMap[msx_button_down_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_DOWN]) && previous_joystick_state.values[ODROID_INPUT_DOWN]) {
        eventMap[msx_button_down_key]  = 0;
    }
    if ((joystick->values[ODROID_INPUT_A]) && !previous_joystick_state.values[ODROID_INPUT_A]) {
        eventMap[msx_button_a_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_A]) && previous_joystick_state.values[ODROID_INPUT_A]) {
        eventMap[msx_button_a_key]  = 0;
    }
    if ((joystick->values[ODROID_INPUT_B]) && !previous_joystick_state.values[ODROID_INPUT_B]) {
        eventMap[msx_button_b_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_B]) && previous_joystick_state.values[ODROID_INPUT_B]) {
        eventMap[msx_button_b_key]  = 0;
    }
    // Game button on G&W
    if ((joystick->values[ODROID_INPUT_START]) && !previous_joystick_state.values[ODROID_INPUT_START]) {
        eventMap[msx_button_game_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_START]) && previous_joystick_state.values[ODROID_INPUT_START]) {
        eventMap[msx_button_game_key]  = 0;
    }
    // Time button on G&W
    if ((joystick->values[ODROID_INPUT_SELECT]) && !previous_joystick_state.values[ODROID_INPUT_SELECT]) {
        eventMap[msx_button_time_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_SELECT]) && previous_joystick_state.values[ODROID_INPUT_SELECT]) {
        eventMap[msx_button_time_key]  = 0;
    }
    // Start button on Zelda G&W
    if ((joystick->values[ODROID_INPUT_X]) && !previous_joystick_state.values[ODROID_INPUT_X]) {
        eventMap[msx_button_start_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_X]) && previous_joystick_state.values[ODROID_INPUT_X]) {
        eventMap[msx_button_start_key]  = 0;
    }
    // Select button on Zelda G&W
    if ((joystick->values[ODROID_INPUT_Y]) && !previous_joystick_state.values[ODROID_INPUT_Y]) {
        eventMap[msx_button_select_key]  = 1;
    } else if (!(joystick->values[ODROID_INPUT_Y]) && previous_joystick_state.values[ODROID_INPUT_Y]) {
        eventMap[msx_button_select_key]  = 0;
    }

    // Handle keyboard emulation
    if (pressed_key != NULL) {
        eventMap[pressed_key->key_id] = (eventMap[pressed_key->key_id] + 1) % 2;
        if (pressed_key->auto_release) {
            release_key = pressed_key;
        }
        pressed_key = NULL;
    } else if (release_key != NULL) {
        if (release_key_delay == 0) {
            eventMap[release_key->key_id] = 0;
            release_key = NULL;
            release_key_delay = RELEASE_KEY_DELAY;
        } else {
            release_key_delay--;
        }
    }

    memcpy(&previous_joystick_state,joystick,sizeof(odroid_gamepad_state_t));
}

static void createOptionMenu(odroid_dialog_choice_t *options) {
    int index=0;
    if (msx_game_type == MSX_GAME_DISK) {
        options[index].id = 100;
        options[index].label = curr_lang->s_msx_Change_Dsk;
        options[index].value = disk_name;
        options[index].enabled = 1;
        options[index].update_cb = &update_disk_cb;
        index++;
    }
    options[index].id = 100;
    options[index].label = curr_lang->s_msx_Select_MSX;
    options[index].value = msx_name;
    options[index].enabled = 1;
    options[index].update_cb = &update_msx_cb;
    index++;
    options[index].id = 100;
    options[index].label = curr_lang->s_msx_Frequency;
    options[index].value = frequency_name;
    options[index].enabled = 1;
    options[index].update_cb = &update_frequency_cb;
    index++;
    options[index].id = 100;
    options[index].label = curr_lang->s_msx_A_Button;
    options[index].value = a_button_name;
    options[index].enabled = 1;
    options[index].update_cb = &update_a_button_cb;
    index++;
    options[index].id = 100;
    options[index].label = curr_lang->s_msx_B_Button;
    options[index].value = b_button_name;
    options[index].enabled = 1;
    options[index].update_cb = &update_b_button_cb;
    index++;
    options[index].id = 100;
    options[index].label = curr_lang->s_msx_Press_Key;
    options[index].value = key_name;
    options[index].enabled = 1;
    options[index].update_cb = &update_keyboard_cb;
    index++;
    options[index].id = 0x0F0F0F0F;
    options[index].label = "LAST";
    options[index].value = "LAST";
    options[index].enabled = 0xFFFF;
    options[index].update_cb = NULL;
}

static void setPropertiesMsx(Machine *machine, int msxType) {
    int i = 0;

    msx2_dif = 0;
    switch(msxType) {
        case 0: // MSX1
            machine->board.type = BOARD_MSX;
            machine->video.vdpVersion = VDP_TMS9929A;
            machine->video.vramSize = 16 * 1024;
            machine->cmos.enable = 0;

            machine->slot[0].subslotted = 0;
            machine->slot[1].subslotted = 0;
            machine->slot[2].subslotted = 0;
            machine->slot[3].subslotted = 1;
            machine->cart[0].slot = 1;
            machine->cart[0].subslot = 0;
            machine->cart[1].slot = 2;
            machine->cart[1].subslot = 0;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 8; // 64kB of RAM
            machine->slotInfo[i].romType = RAM_NORMAL;
            strcpy(machine->slotInfo[i].name, "");
            i++;

            machine->slotInfo[i].slot = 0;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 4;
            machine->slotInfo[i].romType = ROM_CASPATCH;
            strcpy(machine->slotInfo[i].name, "MSX.rom");
            i++;

            if (msx_game_type == MSX_GAME_DISK) {
                machine->slotInfo[i].slot = 3;
                machine->slotInfo[i].subslot = 1;
                machine->slotInfo[i].startPage = 2;
                machine->slotInfo[i].pageCount = 4;
                machine->slotInfo[i].romType = ROM_TC8566AF;
                strcpy(machine->slotInfo[i].name, "PANASONICDISK.rom");
                i++;
            }

            machine->slotInfoCount = i;
            break;

        case 1: // MSX2
            msx2_dif = 10;

            machine->board.type = BOARD_MSX_S3527;
            machine->video.vdpVersion = VDP_V9938;
            machine->video.vramSize = 128 * 1024;
            machine->cmos.enable = 1;

            machine->slot[0].subslotted = 0;
            machine->slot[1].subslotted = 0;
            machine->slot[2].subslotted = 1;
            machine->slot[3].subslotted = 1;
            machine->cart[0].slot = 1;
            machine->cart[0].subslot = 0;
            machine->cart[1].slot = 2;
            machine->cart[1].subslot = 0;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 2;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 16; // 128kB of RAM
            machine->slotInfo[i].romType = RAM_MAPPER;
            strcpy(machine->slotInfo[i].name, "");
            i++;

            machine->slotInfo[i].slot = 0;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 4;
            machine->slotInfo[i].romType = ROM_CASPATCH;
            strcpy(machine->slotInfo[i].name, "MSX2.rom");
            i++;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 1;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 2;
            machine->slotInfo[i].romType = ROM_NORMAL;
            strcpy(machine->slotInfo[i].name, "MSX2EXT.rom");
            i++;

            if (msx_game_type == MSX_GAME_DISK) {
                machine->slotInfo[i].slot = 3;
                machine->slotInfo[i].subslot = 1;
                machine->slotInfo[i].startPage = 2;
                machine->slotInfo[i].pageCount = 4;
                machine->slotInfo[i].romType = ROM_TC8566AF;
                strcpy(machine->slotInfo[i].name, "PANASONICDISK.rom");
                i++;
            } else if (msx_game_type == MSX_GAME_HDIDE) {
                machine->slotInfo[i].slot = 1;
                machine->slotInfo[i].subslot = 0;
                machine->slotInfo[i].startPage = 0;
                machine->slotInfo[i].pageCount = 16;
                machine->slotInfo[i].romType = ROM_MSXDOS2;
                strcpy(machine->slotInfo[i].name, "MSXDOS23.ROM");
                i++;
            }

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 2;
            machine->slotInfo[i].pageCount = 2;
            machine->slotInfo[i].romType = ROM_MSXMUSIC; // FMPAC
            strcpy(machine->slotInfo[i].name, "MSX2PMUS.rom");
            i++;

            machine->slotInfoCount = i;
            break;

        case 2: // MSX2+
            msx2_dif = 10;

            machine->board.type = BOARD_MSX_T9769B;
            machine->video.vdpVersion = VDP_V9958;
            machine->video.vramSize = 128 * 1024;
            machine->cmos.enable = 1;

            machine->slot[0].subslotted = 1;
            machine->slot[1].subslotted = 0;
            machine->slot[2].subslotted = 1;
            machine->slot[3].subslotted = 1;
            machine->cart[0].slot = 1;
            machine->cart[0].subslot = 0;
            machine->cart[1].slot = 2;
            machine->cart[1].subslot = 0;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 16; // 128kB of RAM
            machine->slotInfo[i].romType = RAM_MAPPER;
            strcpy(machine->slotInfo[i].name, "");
            i++;

            machine->slotInfo[i].slot = 0;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 0;
            machine->slotInfo[i].romType = ROM_F4DEVICE; //ROM_F4INVERTED;
            strcpy(machine->slotInfo[i].name, "");
            i++;

            machine->slotInfo[i].slot = 0;
            machine->slotInfo[i].subslot = 0;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 4;
            machine->slotInfo[i].romType = ROM_CASPATCH;
            strcpy(machine->slotInfo[i].name, "MSX2P.rom");
            i++;

            machine->slotInfo[i].slot = 3;
            machine->slotInfo[i].subslot = 1;
            machine->slotInfo[i].startPage = 0;
            machine->slotInfo[i].pageCount = 2;
            machine->slotInfo[i].romType = ROM_NORMAL;
            strcpy(machine->slotInfo[i].name, "MSX2PEXT.rom");
            i++;

            if (msx_game_type == MSX_GAME_DISK) {
                machine->slotInfo[i].slot = 3;
                machine->slotInfo[i].subslot = 1;
                machine->slotInfo[i].startPage = 2;
                machine->slotInfo[i].pageCount = 4;
                machine->slotInfo[i].romType = ROM_TC8566AF;
                strcpy(machine->slotInfo[i].name, "PANASONICDISK.rom");
                i++;
            } else if (msx_game_type == MSX_GAME_HDIDE) {
                machine->slotInfo[i].slot = 1;
                machine->slotInfo[i].subslot = 0;
                machine->slotInfo[i].startPage = 0;
                machine->slotInfo[i].pageCount = 16;
                machine->slotInfo[i].romType = ROM_MSXDOS2;
                strcpy(machine->slotInfo[i].name, "MSXDOS23.ROM");
                i++;
            }

            machine->slotInfo[i].slot = 0;
            machine->slotInfo[i].subslot = 2;
            machine->slotInfo[i].startPage = 2;
            machine->slotInfo[i].pageCount = 2;
            machine->slotInfo[i].romType = ROM_MSXMUSIC; // FMPAC
            strcpy(machine->slotInfo[i].name, "MSX2PMUS.rom");
            i++;

            machine->slotInfoCount = i;
            break;
    }
}

static void createMsxMachine(int msxType) {
    msxMachine = ahb_calloc(1,sizeof(Machine));

    msxMachine->cpu.freqZ80 = 3579545;
    msxMachine->cpu.freqR800 = 7159090;
    msxMachine->fdc.count = 1;
    msxMachine->cmos.batteryBacked = 1;
    msxMachine->audio.psgstereo = 0;
    msxMachine->audio.psgpan[0] = 0;
    msxMachine->audio.psgpan[1] = -1;
    msxMachine->audio.psgpan[2] = 1;

    msxMachine->cpu.hasR800 = 0;
    msxMachine->fdc.enabled = 1;

    // We need to know which kind of media we will load to
    // load correct configuration
    if (0 == strcmp(ACTIVE_FILE->ext,MSX_DISK_EXTENSION)) {
        // Find if file is disk image or IDE HDD image
        const uint8_t *diskData = ACTIVE_FILE->address;
        uint32_t payload_offset = diskData[4]+(diskData[5]<<8)+(diskData[6]<<16)+(diskData[7]<<24);
        if (payload_offset <= 0x288) {
            msx_game_type = MSX_GAME_DISK;
        } else {
            msx_game_type = MSX_GAME_HDIDE;
        }
    } else {
        msx_game_type = MSX_GAME_ROM;
    }
    setPropertiesMsx(msxMachine,msxType);
}

static void insertGame() {
    char game_name[PROP_MAXPATH];
    bool controls_found = true;
    sprintf(game_name,"%s.%s",ACTIVE_FILE->name,ACTIVE_FILE->ext);

    // default config
    msx_button_right_key = EC_RIGHT;
    msx_button_left_key = EC_LEFT;
    msx_button_up_key = EC_UP;
    msx_button_down_key = EC_DOWN;
    msx_button_a_key = EC_SPACE;
    msx_button_b_key = EC_CTRL;
    msx_button_game_key = EC_RETURN;
    msx_button_time_key = EC_CTRL;
    msx_button_start_key = EC_RETURN;
    msx_button_select_key = EC_CTRL;
    uint8_t controls_profile = ACTIVE_FILE->game_config&0xFF;

    switch (controls_profile) {
        case 0: // Default configuration
        break;
        case 1: // Konami
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_N;
            msx_button_game_key = EC_F4;
            msx_button_time_key = EC_F3;
            msx_button_start_key = EC_F1;
            msx_button_select_key = EC_F2;
        break;
        case 2: // Compile
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_LSHIFT;
            msx_button_game_key = EC_STOP;
            msx_button_time_key = EC_Z;
            msx_button_start_key = EC_STOP;
            msx_button_select_key = EC_Z;
        break;
        case 3: // YS I
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_RETURN;
            msx_button_game_key = EC_S; // Status
            msx_button_time_key = EC_I; // Inventory
            msx_button_start_key = EC_S; // Return
            msx_button_select_key = EC_I; // Inventory
        break;
        case 4: // YS II
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_RETURN;
            msx_button_game_key = EC_E; // Equipment
            msx_button_time_key = EC_I; // Inventory
            msx_button_start_key = EC_RETURN;
            msx_button_select_key = EC_S; // Status
        break;
        case 5: // YS III
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_X;
            msx_button_game_key = EC_RETURN;
            msx_button_time_key = EC_I; // Inventory
            msx_button_start_key = EC_RETURN;
            msx_button_select_key = EC_S; // Status
        break;
        case 6: // H.E.R.O.
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_SPACE;
            msx_button_game_key = EC_1; // Start level 1
            msx_button_time_key = EC_2; // Start level 5
            msx_button_start_key = EC_1; // Start level 1
            msx_button_select_key = EC_2; // Start level 5
        break;
        case 7: // SD Snatcher, Arsene Lupin 3rd, ...
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_GRAPH;
            msx_button_game_key = EC_F1; // Pause
            msx_button_time_key = EC_F1; // Pause
            msx_button_start_key = EC_F1; // Pause
            msx_button_select_key = EC_F1; // Pause
        break;
        case 8: // Konami key 2 = Keyboard
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_N;
            msx_button_game_key = EC_2;
            msx_button_time_key = EC_STOP;
            msx_button_start_key = EC_2;
            msx_button_select_key = EC_STOP;
        break;
        case 9: // Konami key 3 = Keyboard
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_N;
            msx_button_game_key = EC_3;
            msx_button_time_key = EC_STOP;
            msx_button_start_key = EC_3;
            msx_button_select_key = EC_STOP;
        break;
        case 10: // Konami Green Beret
            msx_button_a_key = EC_LSHIFT;
            msx_button_b_key = EC_LSHIFT;
            msx_button_game_key = EC_STOP;
            msx_button_time_key = EC_STOP;
            msx_button_start_key = EC_STOP;
            msx_button_select_key = EC_STOP;
        break;
        case 11: // Dragon Slayer 4
            msx_button_a_key = EC_LSHIFT; // Jump
            msx_button_b_key = EC_Z; // Magic
            msx_button_game_key = EC_RETURN; // Menu selection
            msx_button_time_key = EC_ESC; // Inventory
            msx_button_start_key = EC_RETURN;
            msx_button_select_key = EC_ESC;
        break;
        case 12: // Dragon Slayer 6
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_LSHIFT;
            msx_button_game_key = EC_RETURN;
            msx_button_time_key = EC_ESC;
            msx_button_start_key = EC_RETURN;
            msx_button_select_key = EC_ESC;
        break;
        case 13: // Dunk Shot
            msx_button_a_key = EC_LBRACK;
            msx_button_b_key = EC_RETURN;
            msx_button_game_key = EC_SPACE;
            msx_button_time_key = EC_SPACE;
            msx_button_start_key = EC_SPACE;
            msx_button_select_key = EC_SPACE;
        break;
        case 14: //Eggerland Mystery
            msx_button_a_key = EC_SPACE;
            msx_button_a_key = EC_SPACE;
            msx_button_game_key = EC_SPACE;
            msx_button_time_key = EC_STOP; // Suicide
            msx_button_start_key = EC_SPACE;
            msx_button_select_key = EC_STOP;
        break;
        case 15: // Famicle Parodic, 1942
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_GRAPH;
            msx_button_game_key = EC_STOP; // Pause
            msx_button_time_key = EC_STOP; // Pause
            msx_button_start_key = EC_STOP; // Pause
            msx_button_select_key = EC_STOP; // Pause
        break;
        case 16: // Laydock 2
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_LSHIFT;
            msx_button_game_key = EC_RETURN;
            msx_button_time_key = EC_STOP;
            msx_button_start_key = EC_RETURN;
            msx_button_select_key = EC_STOP;
        break;
        case 17: // Fray - In Magical Adventure
            msx_button_a_key = EC_C;
            msx_button_b_key = EC_X;
            msx_button_game_key = EC_LSHIFT;
            msx_button_time_key = EC_SPACE;
            msx_button_start_key = EC_LSHIFT;
            msx_button_select_key = EC_SPACE;
        break;
        case 18: // XAK 1
            // Note : If you press Space + Return, you enter main menu
            // which is allowing to go to Equipment/Items/System menu
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_RETURN;
            msx_button_game_key = EC_F2; // Equipment
            msx_button_time_key = EC_F3; // Items
            msx_button_start_key = EC_RETURN;
            msx_button_select_key = EC_F4; // System Menu
        break;
        case 19: // XAK 2/3
            // Note : If you press Space + Return, you enter main menu
            // which is allowing to go to Equipment/Items/System menu
            msx_button_a_key = EC_C;
            msx_button_b_key = EC_X;
            msx_button_game_key = EC_F2; // System Menu
            msx_button_time_key = EC_F3; // Speed Menu
            msx_button_start_key = EC_SPACE;
            msx_button_select_key = EC_RETURN;
        break;
        case 20: // Ghost
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_GRAPH;
            msx_button_game_key = EC_F5; // Continue
            msx_button_time_key = EC_F1; // Menu
            msx_button_start_key = EC_F5; // Continue
            msx_button_select_key = EC_F1; // Menu
        break;
        case 21: // Golvellius II
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_LSHIFT;
            msx_button_game_key = EC_STOP; // Continue
            msx_button_time_key = EC_STOP; // Menu
            msx_button_start_key = EC_STOP; // Continue
            msx_button_select_key = EC_STOP; // Menu
        break;
        case 22: // R-TYPE
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_GRAPH;
            msx_button_game_key = EC_ESC;
            msx_button_time_key = EC_ESC;
            msx_button_start_key = EC_ESC;
            msx_button_select_key = EC_ESC;
        break;
        case 23: // Super Lode Runner
            msx_button_a_key = EC_UNDSCRE;
            msx_button_b_key = EC_DIV;
            msx_button_game_key = EC_SPACE;
            msx_button_time_key = EC_STOP;
            msx_button_start_key = EC_SPACE;
            msx_button_select_key = EC_STOP;
        break;
        case 24: // Lode Runner 1 & 2
            msx_button_a_key = EC_X;
            msx_button_b_key = EC_Z;
            msx_button_game_key = EC_SPACE;
            msx_button_time_key = EC_STOP;
            msx_button_start_key = EC_SPACE;
            msx_button_select_key = EC_STOP;
        break;
        case 25: // Dynamite Go Go
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_C;
            msx_button_game_key = EC_STOP;
            msx_button_time_key = EC_STOP;
            msx_button_start_key = EC_STOP;
            msx_button_select_key = EC_STOP;
        break;
        case 26: // Moon Patrol
            msx_button_a_key = EC_SPACE; // Fire
            msx_button_b_key = EC_RIGHT; // Jump
            msx_button_game_key = EC_1;
            msx_button_time_key = EC_STOP;
            msx_button_start_key = EC_1;
            msx_button_select_key = EC_STOP;
            msx_button_right_key = EC_X;
        break;
        case 27: // Pyro-Man
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_LSHIFT;
            msx_button_game_key = EC_1;
            msx_button_time_key = EC_STOP;
            msx_button_start_key = EC_1;
            msx_button_select_key = EC_STOP;
        break;
        case 28: // Roller Ball
            msx_button_a_key = EC_RETURN;
            msx_button_b_key = EC_RETURN;
            msx_button_game_key = EC_STOP;
            msx_button_time_key = EC_STOP;
            msx_button_start_key = EC_STOP;
            msx_button_select_key = EC_STOP;
            msx_button_left_key = EC_ESC;
        break;
        case 29: // Robo Rumble
#if GNW_TARGET_MARIO != 0
            msx_button_a_key = EC_P; // Right Magnet UP
            msx_button_b_key = EC_L; // Right Magnet Down
            msx_button_game_key = EC_SPACE; // Start game
            msx_button_time_key = EC_SPACE; // Start game
            msx_button_up_key = EC_Q; // Left Magnet UP
            msx_button_down_key = EC_A; // Left Magnet Down
            msx_button_right_key = EC_Q; // Left Magnet UP
            msx_button_left_key = EC_A; // Left Magnet Down
#else
            msx_button_a_key = EC_L; // Right Magnet Down
            msx_button_b_key = EC_L; // Right Magnet Down
            msx_button_game_key = EC_SPACE; // Start game
            msx_button_time_key = EC_SPACE; // Start game
            msx_button_start_key = EC_P; // Right Magnet UP
            msx_button_select_key = EC_P; // Right Magnet UP
            msx_button_up_key = EC_Q; // Left Magnet UP
            msx_button_down_key = EC_A; // Left Magnet Down
#endif
        break;
        case 30: // Brunilda
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_SPACE;
            msx_button_game_key = EC_SPACE;
            msx_button_time_key = EC_R;
            msx_button_start_key = EC_SPACE;
            msx_button_select_key = EC_R;
        break;
        case 31: // Castle Excellent
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_GRAPH;
            msx_button_game_key = EC_F5;
            msx_button_time_key = EC_F1;
            msx_button_start_key = EC_F5;
            msx_button_select_key = EC_F1;
        break;
        case 32: // Doki Doki Penguin Land
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_GRAPH;
            msx_button_game_key = EC_1;
            msx_button_time_key = EC_STOP;
            msx_button_start_key = EC_1;
            msx_button_select_key = EC_STOP;
        break;
        case 33: // Angelic Warrior Deva
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_UP;
            msx_button_game_key = EC_F1; // Pause
            msx_button_time_key = EC_F1; // Pause
            msx_button_start_key = EC_F1; // Pause
            msx_button_select_key = EC_F1; // Pause
        break;
        case 34: // Konami MSX1 Game Collection
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_N;
            msx_button_game_key = EC_3;
            msx_button_time_key = EC_2;
            msx_button_start_key = EC_3;
            msx_button_select_key = EC_2;
        break;
        case 35: // Penguin-Kun Wars 2
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_SPACE;
            msx_button_game_key = EC_F1;
            msx_button_time_key = EC_F1;
            msx_button_start_key = EC_F1;
            msx_button_select_key = EC_F1;
        break;
        case 36: // Stevedore
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_SPACE;
            msx_button_game_key = EC_STOP;
            msx_button_time_key = EC_STOP;
            msx_button_start_key = EC_STOP;
            msx_button_select_key = EC_STOP;
        break;
        case 37: // The Castle
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_CTRL;
            msx_button_game_key = EC_F5;
            msx_button_time_key = EC_F1;
            msx_button_start_key = EC_F5;
            msx_button_select_key = EC_F1;
        break;
        case 38: // Youkai Yashiki
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_GRAPH;
            msx_button_game_key = EC_STOP;
            msx_button_time_key = EC_F1;
            msx_button_start_key = EC_STOP;
            msx_button_select_key = EC_F1;
        break;
        case 39: // Xevious
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_Z;
            msx_button_game_key = EC_STOP;
            msx_button_time_key = EC_LSHIFT;
            msx_button_start_key = EC_STOP;
            msx_button_select_key = EC_LSHIFT;
        break;
        case 40: // Undeadline
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_LSHIFT;
            msx_button_game_key = EC_ESC;
            msx_button_time_key = EC_ESC;
            msx_button_start_key = EC_ESC;
            msx_button_select_key = EC_ESC;
        break;
        case 41: // Valis 2
            msx_button_a_key = EC_X;
            msx_button_b_key = EC_Z;
            msx_button_game_key = EC_RETURN;
            msx_button_time_key = EC_F1;
            msx_button_start_key = EC_RETURN;
            msx_button_select_key = EC_F1;
        break;
        case 42: // Mr Ghost
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_LSHIFT;
            msx_button_game_key = EC_F1;
            msx_button_time_key = EC_F1;
            msx_button_start_key = EC_F1;
            msx_button_select_key = EC_F1;
        break;
        case 43: // Firebird
            msx_button_a_key = EC_SPACE;
            msx_button_b_key = EC_N;
            msx_button_game_key = EC_F1;
            msx_button_time_key = EC_F5;
            msx_button_start_key = EC_F1;
            msx_button_select_key = EC_F5;
        break;
        case 44: // Super Mario World
#if GNW_TARGET_MARIO != 0
            msx_button_a_key = EC_X; // Jump
            msx_button_b_key = EC_C; // Run/Grab/Fire
            msx_button_game_key = EC_SPACE; // Start
            msx_button_time_key = EC_Z; // Spin Jump
#else
            msx_button_a_key = EC_X; // Jump
            msx_button_b_key = EC_C; // Run/Grab/Fire
            msx_button_game_key = EC_ESC; // Menu
            msx_button_time_key = EC_SPACE; // Pause
            msx_button_start_key = EC_Z; // Spin Jump
            msx_button_select_key = EC_V; // Drop power reserve
#endif
        break;
        default:
            controls_found = false;
        break;
    }
    switch (msx_game_type) {
        case MSX_GAME_ROM:
        {
            printf("Rom Mapper %d\n",ACTIVE_FILE->mapper);
            uint16_t mapper = ACTIVE_FILE->mapper;
            if (mapper == ROM_UNKNOWN) {
                uint32_t rom_size;
                uint8_t *rom_data;
                rom_size = msx_getromdata(&rom_data,(uint8_t *)ROM_DATA,ROM_DATA_LENGTH,ROM_EXT);
                mapper = GuessROM(rom_data,rom_size);
            }
            if (!controls_found) {
                // If game is using konami mapper, we setup a Konami key mapping
                // elseway we use joystick control
                switch (mapper)
                {
                    case ROM_KONAMI5:
                    case ROM_KONAMI4:
                    case ROM_KONAMI4NF:
                        msx_button_a_key = EC_SPACE;
                        msx_button_b_key = EC_N;
                        msx_button_game_key = EC_F4;
                        msx_button_time_key = EC_F3;
                        msx_button_start_key = EC_F1;
                        msx_button_select_key = EC_F2;
                        break;
                    default:
                        msx_button_up_key = EC_JOY1_UP;
                        msx_button_down_key = EC_JOY1_DOWN;
                        msx_button_right_key = EC_JOY1_RIGHT;
                        msx_button_left_key = EC_JOY1_LEFT;
                        msx_button_a_key = EC_JOY1_BUTTON1;
                        msx_button_b_key = EC_JOY1_BUTTON2;
                        break;
                }
            }
            printf("insertCartridge msx mapper %d\n",mapper);
            insertCartridge(properties, 0, game_name, NULL, mapper, -1);
            break;
        }
        case MSX_GAME_DISK:
        {
            if (selected_disk_index == -1) {
                const rom_system_t *msx_system = rom_manager_system(&rom_mgr, "MSX");
                selected_disk_index = rom_get_index_for_file_ext(msx_system,ACTIVE_FILE);

                insertDiskette(properties, 0, game_name, NULL, -1);
            } else {
                retro_emulator_file_t *disk_file = NULL;
                const rom_system_t *msx_system = rom_manager_system(&rom_mgr, "MSX");
                disk_file = (retro_emulator_file_t *)rom_get_ext_file_at_index(msx_system,MSX_DISK_EXTENSION,selected_disk_index);
                sprintf(game_name,"%s.%s",disk_file->name,disk_file->ext);
                insertDiskette(properties, 0, game_name, NULL, -1);
            }
            // We load SCC-I cartridge for disk games requiring it
            insertCartridge(properties, 0, CARTNAME_SNATCHER, NULL, ROM_SNATCHER, -1);
            if (!controls_found) {
                // If game name contains konami, we setup a Konami key mapping
                if (strcasestr(ACTIVE_FILE->name,"konami")) {
                    msx_button_a_key = EC_SPACE;
                    msx_button_b_key = EC_N;
                    msx_button_game_key = EC_F4;
                    msx_button_time_key = EC_F3;
                    msx_button_start_key = EC_F1;
                    msx_button_select_key = EC_F2;
                } else {
                    msx_button_up_key = EC_JOY1_UP;
                    msx_button_down_key = EC_JOY1_DOWN;
                    msx_button_right_key = EC_JOY1_RIGHT;
                    msx_button_left_key = EC_JOY1_LEFT;
                    msx_button_a_key = EC_JOY1_BUTTON1;
                    msx_button_b_key = EC_JOY1_BUTTON2;
                }
            }
            break;
        }
        case MSX_GAME_HDIDE:
        {
            insertCartridge(properties, 0, CARTNAME_SUNRISEIDE, NULL, ROM_SUNRISEIDE, -1);
            insertCartridge(properties, 1, CARTNAME_SNATCHER, NULL, ROM_SNATCHER, -1);
            insertDiskette(properties, 1, game_name, NULL, -1);
            break;
        }
    }
}

static void createProperties() {
    properties = propCreate(1, EMU_LANG_ENGLISH, P_KBD_EUROPEAN, P_EMU_SYNCNONE, "");
    properties->sound.stereo = 0;
    if (selected_frequency_index == FREQUENCY_VDP_AUTO) {
        properties->emulation.vdpSyncMode = P_VDP_SYNCAUTO;
    } else if (selected_frequency_index == FREQUENCY_VDP_60HZ) {
        properties->emulation.vdpSyncMode = P_VDP_SYNC60HZ;
    } else {
        properties->emulation.vdpSyncMode = P_VDP_SYNC50HZ;
    }
    properties->emulation.enableFdcTiming = 0;
    properties->emulation.noSpriteLimits = 0;
    properties->sound.masterVolume = 0;

    currentVolume = -1;
    // Default : enable SCC and disable MSX-MUSIC
    // This will be changed dynamically if the game use MSX-MUSIC
    properties->sound.mixerChannel[MIXER_CHANNEL_SCC].enable = 1;
    properties->sound.mixerChannel[MIXER_CHANNEL_MSXMUSIC].enable = 1;
    properties->sound.mixerChannel[MIXER_CHANNEL_PSG].pan = 0;
    properties->sound.mixerChannel[MIXER_CHANNEL_MSXMUSIC].pan = 0;
    properties->sound.mixerChannel[MIXER_CHANNEL_SCC].pan = 0;

    // Joystick Configuration
    properties->joy1.typeId = JOYSTICK_PORT_JOYSTICK;
}

static void setupEmulatorRessources(int msxType)
{
    int i;
    mixer = mixerCreate();
    createProperties();
    createMsxMachine(msxType);
    emulatorInit(properties, mixer);
    insertGame();
    emulatorRestartSound();

    for (i = 0; i < MIXER_CHANNEL_TYPE_COUNT; i++)
    {
        mixerSetChannelTypeVolume(mixer, i, properties->sound.mixerChannel[i].volume);
        mixerSetChannelTypePan(mixer, i, properties->sound.mixerChannel[i].pan);
        mixerEnableChannelType(mixer, i, properties->sound.mixerChannel[i].enable);
    }

    mixerSetMasterVolume(mixer, properties->sound.masterVolume);
    mixerEnableMaster(mixer, properties->sound.masterEnable);

    boardSetFdcTimingEnable(properties->emulation.enableFdcTiming);
    boardSetY8950Enable(0/*properties->sound.chip.enableY8950*/);
    boardSetYm2413Enable(1/*properties->sound.chip.enableYM2413*/);
    boardSetMoonsoundEnable(0/*properties->sound.chip.enableMoonsound*/);
    boardSetVideoAutodetect(1/*properties->video.detectActiveMonitor*/);

    emulatorStartMachine(NULL, msxMachine);
    // Enable SCC and disable MSX-MUSIC as G&W is not powerfull enough to handle both at same time
    // If a game wants to play MSX-MUSIC sound, the mapper will detect it and it will disable SCC
    // and enable MSX-MUSIC
    mixerEnableChannelType(boardGetMixer(), MIXER_CHANNEL_SCC, 1);
    mixerEnableChannelType(boardGetMixer(), MIXER_CHANNEL_MSXMUSIC, 0);
}

// No scaling
__attribute__((optimize("unroll-loops")))
static inline void blit_normal(uint8_t *msx_fb, uint16_t *framebuffer) {
    const int w1 = image_buffer_current_width;
    const int w2 = GW_LCD_WIDTH;
    const int h2 = GW_LCD_HEIGHT;
    const int hpad = 27;
    uint8_t  *src_row;
    uint16_t *dest_row;

    for (int y = 0; y < h2; y++) {
        src_row  = &msx_fb[y*w1];
        dest_row = &framebuffer[y * w2 + hpad];
        for (int x = 0; x < w1; x++) {
            dest_row[x] = palette565[src_row[x]];
        }
    }
}

__attribute__((optimize("unroll-loops")))
static inline void screen_blit_nn(uint8_t *msx_fb, uint16_t *framebuffer/*int32_t dest_width, int32_t dest_height*/)
{
    int w1 = width;
    int h1 = height;
    int w2 = GW_LCD_WIDTH;
    int h2 = GW_LCD_HEIGHT;
    int src_x_offset = 8;
    int src_y_offset = 24 - (msx2_dif);

    int x_ratio = (int)((w1<<16)/w2) +1;
    int y_ratio = (int)((h1<<16)/h2) +1;
    int hpad = 0;
    int wpad = 0;

    int x2;
    int y2;

    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            x2 = ((j*x_ratio)>>16) ;
            y2 = ((i*y_ratio)>>16) ;
            uint8_t b2 = msx_fb[((y2+src_y_offset)*image_buffer_current_width)+x2+src_x_offset];
            framebuffer[((i+wpad)*WIDTH)+j+hpad] = palette565[b2];
        }
    }
}

static void blit(uint8_t *msx_fb, uint16_t *framebuffer)
{
    odroid_display_scaling_t scaling = odroid_display_get_scaling_mode();
    uint16_t offset = 274;

    switch (scaling) {
    case ODROID_DISPLAY_SCALING_OFF:
        use_overscan = true;
        update_fb_info();
        blit_normal(msx_fb, framebuffer);
        break;
    // Full height, borders on the side
    case ODROID_DISPLAY_SCALING_FIT:
    case ODROID_DISPLAY_SCALING_FULL:
    case ODROID_DISPLAY_SCALING_CUSTOM:
        offset = GW_LCD_WIDTH-26;
        use_overscan = false;
        update_fb_info();
        screen_blit_nn(msx_fb, framebuffer);
        break;
    default:
        printf("Unsupported scaling mode %d\n", scaling);
        break;
    }
    if (show_disk_icon) {
        uint16_t *dest = lcd_get_active_buffer();
        uint16_t idx = 0;
        for (uint8_t i = 0; i < 24; i++) {
            for (uint8_t j = 0; j < 24; j++) {
            if (IMG_DISKETTE[idx / 8] & (1 << (7 - idx % 8))) {
                dest[offset + j + GW_LCD_WIDTH * (2 + i)] = 0xFFFF;
            }
            idx++;
            }
        }
    }
}

size_t msx_getromdata(uint8_t **data, uint8_t *src_data, size_t src_size, const char *ext)
{
    /* src pointer to the ROM data in the external flash (raw or LZ4) */
    unsigned char *dest = (unsigned char *)rom_decompress_buffer;
    uint32_t available_size = (uint32_t)sizeof(rom_decompress_buffer);

    wdog_refresh();
    if(strcmp(ext, "lzma") == 0){
        size_t n_decomp_bytes;
        n_decomp_bytes = lzma_inflate(dest, available_size, src_data, src_size);
        *data = dest;
        return n_decomp_bytes;
    }
    else
    {
        *data = (unsigned char *)src_data;

        return src_size;
    }
}

void app_main_msx(uint8_t load_state, uint8_t start_paused, int8_t save_slot)
{
    pixel_t *fb;
    odroid_dialog_choice_t options[10];
    bool drawFrame;
    dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;

    show_disk_icon = false;
    selected_disk_index = -1;

    // Create RGB8 to RGB565 table
    for (int i = 0; i < 256; i++)
    {
        // RGB 8bits to RGB 565 (RRR|GGG|BB -> RRRRR|GGGGGG|BBBBB)
        palette565[i] = (((i>>5)*31/7)<<11) |
                         ((((i&0x1C)>>2)*63/7)<<5) |
                         ((i&0x3)*31/3);
    }

    if (start_paused) {
        common_emu_state.pause_after_frames = 2;
    } else {
        common_emu_state.pause_after_frames = 0;
    }
    common_emu_state.frame_time_10us = (uint16_t)(100000 / msx_fps + 0.5f);

    odroid_system_init(APPID_MSX, AUDIO_MSX_SAMPLE_RATE);
    odroid_system_emu_init(&msx_system_LoadState, &msx_system_SaveState, NULL);

    image_buffer_base_width    =  272;
    image_buffer_current_width =  image_buffer_base_width;
    image_buffer_height        =  240;

    memset(lcd_get_active_buffer(), 0, sizeof(framebuffer1));
    memset(lcd_get_inactive_buffer(), 0, sizeof(framebuffer1));
    memset(msx_framebuffer, 0, sizeof(msx_framebuffer));
    
    memset(audiobuffer_dma, 0, 2*(AUDIO_MSX_SAMPLE_RATE/FPS_PAL)*sizeof(Int16));

    setupEmulatorRessources(selected_msx_index);

    createOptionMenu(options);

    if (load_state) {
        odroid_system_emu_load_state(save_slot);
    }

    while (1) {
        // Frequency change check if in automatic mode
        if ((selected_frequency_index == FREQUENCY_VDP_AUTO) && (msx_fps != boardInfo.getRefreshRate())) {
            // Update ressources to switch system frequency
            msx_fps = boardInfo.getRefreshRate();
            common_emu_state.frame_time_10us = (uint16_t)(100000 / msx_fps + 0.5f);
            memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
            HAL_SAI_DMAStop(&hsai_BlockA1);
            HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, (2 * AUDIO_MSX_SAMPLE_RATE / msx_fps));
            emulatorRestartSound();
        }
        wdog_refresh();
        drawFrame = common_emu_frame_loop();
        odroid_gamepad_state_t joystick;
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

        msxInputUpdate(&joystick);

        // Render 1 frame
        ((R800*)boardInfo.cpuRef)->terminate = 0;
        boardInfo.run(boardInfo.cpuRef);

        if (drawFrame) {
            fb = lcd_get_active_buffer();
            // If current MSX screen mode is 10 or 12, data has been directly written into
            // framebuffer (scaling is not possible for these screen modes), elseway apply
            // current scaling mode
            if ((vdpGetScreenMode() != 10) && (vdpGetScreenMode() != 12)) {
                blit(msx_framebuffer, fb);
            }
            common_ingame_overlay();
            lcd_swap();
        }

        // Render audio
        mixerSyncGNW(mixer,(AUDIO_MSX_SAMPLE_RATE/msx_fps));

        if(!common_emu_state.skip_frames) {
            for(uint8_t p = 0; p < common_emu_state.pause_frames + 1; p++) {
                while (dma_state == last_dma_state) {
                    cpumon_sleep();
                }
                last_dma_state = dma_state;
            }
        }
    }
}

static Int32 soundWrite(void* dummy, Int16 *buffer, UInt32 count)
{
    size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : (AUDIO_MSX_SAMPLE_RATE/msx_fps);
    uint8_t volume = odroid_audio_volume_get();
    if (volume != currentVolume) {
        if (volume == 0) {
            mixerSetEnable(mixer,0);
        } else {
            mixerSetEnable(mixer,1);
            mixerSetMasterVolume(mixer,volume_table[volume]);
        }
        currentVolume = volume;
    }

    memcpy(&audiobuffer_dma[offset],buffer,(AUDIO_MSX_SAMPLE_RATE/msx_fps)*sizeof(Int16));
    return 0;
}

void archSoundCreate(Mixer* mixer, UInt32 sampleRate, UInt32 bufferSize, Int16 channels) {
    // Init Sound
    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));

    HAL_SAI_DMAStop(&hsai_BlockA1);
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, 2*(AUDIO_MSX_SAMPLE_RATE / msx_fps));

    mixerSetStereo(mixer, 0);
    mixerSetWriteCallback(mixer, soundWrite, NULL, (AUDIO_MSX_SAMPLE_RATE/msx_fps));
}

void archSoundDestroy(void) {}

#if CHEAT_CODES == 1
void update_cheats_msx() {
    msxUpdateCheatInfo();
}
#endif

#endif
