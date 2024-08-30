#include "build/config.h"

#ifdef ENABLE_EMULATOR_A2600
extern "C"
{
#include <odroid_system.h>
#include <string.h>
#include <assert.h>

#include "main.h"
#include "bilinear.h"
#include "gw_lcd.h"
#include "gw_linker.h"
#include "rg_i18n.h"
#include "gw_buttons.h"
#include "common.h"
#include "rom_manager.h"
#include "appid.h"
#include "gw_malloc.h"
#include "lzma.h"

extern void __libc_init_array(void);
}

#include "Console.hxx"
#include "Cart.hxx"
#include "MD5.hxx"
#include "Paddles.hxx"
#include "Sound.hxx"
#include "Switches.hxx"
#include "StateManager.hxx"
#include "TIA.hxx"
#include "M6532.hxx"
#include "Version.hxx"

#include "Stubs.hxx"

static Console *console = 0;
static Cartridge *cartridge = 0;
static Settings *settings = 0;
static OSystem osystem;
static StateManager stateManager(&osystem);

static int videoWidth, videoHeight;

static const uint32_t *currentPalette32 = NULL;
static uint16_t currentPalette16[256] = {0};

static int16_t sampleBuffer[2048];

static int paddle_digital_sensitivity = 50;

#define AUDIO_A2600_SAMPLE_RATE 31400

static bool LoadState(char *savePathName, char *sramPathName, int slot)
{
    return 0;
}

static bool SaveState(char *savePathName, char *sramPathName, int slot)
{
    return 0;
}

uint8_t a2600_y_offset = 0;
uint16_t a2600_height = 0x00;
char a2600_control[15];
bool a2600_control_swap = false;
bool a2600_swap_paddle = false;
char a2600_display_mode[10];
uint8_t a2600_difficulty;
bool a2600_fastscbios = false;

#define ROM_BUFF_LENGTH 131072 // 128kB
// Memory to handle compressed roms
static uint8_t rom_memory[ROM_BUFF_LENGTH];

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

void fill_stella_config()
{
    /* ACTIVE_FILE->extra description
     * Byte 0 : Mapper
     * Byte 1 : Y offset
     * Byte 2 : Height
     * Byte 3 : Control type
     * Byte 4 : Control swap
     * Byte 5 : difficulty
     */
    a2600_y_offset = ACTIVE_FILE->extra[1];
    if (ACTIVE_FILE->extra[2] > 0)
    {
        a2600_height = ACTIVE_FILE->extra[2] + 213;
    }
    switch (ACTIVE_FILE->extra[3])
    {
    case 0:
        strcpy(a2600_control, ""); // Joystick
        a2600_control_swap = ACTIVE_FILE->extra[4];
        break;
    case 1:
        strcpy(a2600_control, "KEYBOARD");
        a2600_control_swap = ACTIVE_FILE->extra[4];
        break;
    case 2:
        strcpy(a2600_control, "PADDLES");
        a2600_swap_paddle = ACTIVE_FILE->extra[4];
        break;
    case 3:
        strcpy(a2600_control, "GENESIS");
        a2600_control_swap = ACTIVE_FILE->extra[4];
        break;
    case 4:
        strcpy(a2600_control, "DRIVING");
        a2600_control_swap = ACTIVE_FILE->extra[4];
        break;
    case 5:
        strcpy(a2600_control, "BOOSTERGRIP");
        a2600_control_swap = ACTIVE_FILE->extra[4];
        break;
    case 6:
        strcpy(a2600_control, "AMIGAMOUSE");
        a2600_control_swap = ACTIVE_FILE->extra[4];
        break;
    case 7:
        strcpy(a2600_control, "PADDLES_IAXDR");
        a2600_swap_paddle = ACTIVE_FILE->extra[4];
        break;
    case 8:
        strcpy(a2600_control, "COMPUMATE");
        a2600_control_swap = ACTIVE_FILE->extra[4];
        break;
    case 9:
        strcpy(a2600_control, "PADDLES_IAXIS");
        a2600_swap_paddle = ACTIVE_FILE->extra[4];
        break;
    case 10:
        strcpy(a2600_control, "TRACKBALL22");
        a2600_control_swap = ACTIVE_FILE->extra[4];
        break;
    case 11:
        strcpy(a2600_control, "TRACKBALL80");
        a2600_control_swap = ACTIVE_FILE->extra[4];
        break;
    case 12:
        strcpy(a2600_control, "MINDLINK");
        a2600_control_swap = ACTIVE_FILE->extra[4];
        break;
    }
    switch (ACTIVE_FILE->region)
    {
    case REGION_NTSC:
        strcpy(a2600_display_mode, "NTSC");
        break;
    case REGION_PAL:
        strcpy(a2600_display_mode, "PAL");
        break;
    case REGION_SECAM:
        strcpy(a2600_display_mode, "SECAM");
        break;
    case REGION_NTSC50:
        strcpy(a2600_display_mode, "NTSC50");
        break;
    case REGION_PAL60:
        strcpy(a2600_display_mode, "PAL60");
        break;
    case REGION_AUTO:
        strcpy(a2600_display_mode, "AUTO");
        break;
    }
    a2600_difficulty = ACTIVE_FILE->extra[5];
}
void update_joystick(odroid_gamepad_state_t *joystick)
{
    Event &ev = osystem.eventHandler().event();
    ev.set(Event::Type(Event::JoystickZeroUp), joystick->values[ODROID_INPUT_UP] ? 1 : 0);
    ev.set(Event::Type(Event::JoystickZeroDown), joystick->values[ODROID_INPUT_DOWN] ? 1 : 0);
    ev.set(Event::Type(Event::JoystickZeroLeft), joystick->values[ODROID_INPUT_LEFT] ? 1 : 0);
    ev.set(Event::Type(Event::JoystickZeroRight), joystick->values[ODROID_INPUT_RIGHT] ? 1 : 0);
    ev.set(Event::Type(Event::JoystickZeroFire), joystick->values[ODROID_INPUT_A] ? 1 : 0);
    /*    ev.set(Event::Type(Event::ConsoleLeftDiffA),  joystick_state & (1 << RETRO_DEVICE_ID_JOYPAD_L));
        ev.set(Event::Type(Event::ConsoleLeftDiffB),  joystick_state & (1 << RETRO_DEVICE_ID_JOYPAD_L2));
        ev.set(Event::Type(Event::ConsoleColor),      joystick_state & (1 << RETRO_DEVICE_ID_JOYPAD_L3));
        ev.set(Event::Type(Event::ConsoleRightDiffA), joystick_state & (1 << RETRO_DEVICE_ID_JOYPAD_R));
        ev.set(Event::Type(Event::ConsoleRightDiffB), joystick_state & (1 << RETRO_DEVICE_ID_JOYPAD_R2));
        ev.set(Event::Type(Event::ConsoleBlackWhite), joystick_state & (1 << RETRO_DEVICE_ID_JOYPAD_R3));*/
    ev.set(Event::Type(Event::ConsoleSelect), joystick->values[ODROID_INPUT_SELECT] || joystick->values[ODROID_INPUT_Y]);
    ev.set(Event::Type(Event::ConsoleReset), joystick->values[ODROID_INPUT_START] || joystick->values[ODROID_INPUT_X]);

    console->controller(Controller::Left).update();
    console->switches().update();
}

static void convert_palette(const uint32_t *palette32, uint16_t *palette16)
{
    size_t i;
    for (i = 0; i < 256; i++)
    {
        uint32_t color32 = *(palette32 + i);
        *(palette16 + i) = ((color32 & 0xF80000) >> 8) |
                           ((color32 & 0x00F800) >> 5) |
                           ((color32 & 0x0000F8) >> 3);
    }
}

static void blend_frames_16(uInt8 *stella_fb, int width, int height)
{
    printf("width %d height %d\n", width, height);
    if (width > 320)
        width = 320;
    if (height > 240)
        height = 240;

    const uint32_t *palette32 = console->getPalette(0);
    uint16_t *palette16 = currentPalette16;
    uInt8 *in = stella_fb;
    uint16_t *out = (uint16_t *)lcd_get_active_buffer();
    int x, y;
//    int yoffset = (240 - height) / 2;

    /* If palette has changed, re-cache converted
     * RGB565 values */
    if (palette32 != currentPalette32)
    {
        currentPalette32 = palette32;
        convert_palette(palette32, palette16);
    }

    /*   for (y=0; y < height; y++) {
        for (x = 0; x < width; x++) {
          *(out+y*320+x) = *(palette16 + *(in++));
        }
       }*/

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            *(out + y * 320 + 2 * x) = *(palette16 + *(in++));
            *(out + y * 320 + 2 * x + 1) = *(out + y * 320 + 2 * x);
        }
    }
    /*   for (y=yoffset; y < height+yoffset; y++) {
        for (x = 0; x < width; x++) {
          *(out+y*320+2*x) = *(palette16 + *(in++));
          *(out+y*320+2*x+1) = *(out+y*320+2*x);
        }
       }*/
}

static void sound_store(int16_t *audio_out_buf, uint16_t length)
{
    // MUST shift with at least 1 place, or it will brownout.
    uint8_t volume = odroid_audio_volume_get();
    int32_t factor = volume_tbl[volume];
    int16_t *audio_in_buf = sampleBuffer;

    if (audio_mute || (volume == ODROID_AUDIO_VOLUME_MIN))
    {
        // mute
        for (int i = 0; i < length; i++)
        {
            audio_out_buf[i] = 0;
        }
        return;
    }

    // Write to DMA buffer and lower the volume accordingly
    for (int i = 0; i < length; i++)
    {
        int32_t sample = *audio_in_buf++;
        audio_out_buf[i] = (sample * factor) >> 8;
    }
}

void blit()
{
}

void app_main_a2600_cpp(uint8_t load_state, uint8_t start_paused, int8_t save_slot)
{
    static dma_transfer_state_t last_dma_state = DMA_TRANSFER_STATE_HF;
    size_t offset;
    odroid_gamepad_state_t joystick;
    odroid_dialog_choice_t options[] = {
        ODROID_DIALOG_CHOICE_LAST};
    uint32_t rom_length = 0;
    uint8_t *rom_ptr = NULL;

    if (start_paused)
    {
        common_emu_state.pause_after_frames = 2;
    }
    else
    {
        common_emu_state.pause_after_frames = 0;
    }

    string cartMD5 = "";

    fill_stella_config();

    // Load the cart
    string cartType = "";
    switch (ACTIVE_FILE->extra[0])
    {
    case 0:
        cartType = "AUTO";
        break;
    case 1:
        cartType = "2IN1";
        break;
    case 2:
        cartType = "CM";
        break;
    case 3:
        cartType = "4IN1";
        break;
    case 4:
        cartType = "32IN1";
        break;
    case 5:
        cartType = "FE";
        break;
    case 6:
        cartType = "F6SC";
        break;
    case 7:
        cartType = "4K";
        break;
    case 8:
        cartType = "8IN1";
        break;
    case 9:
        cartType = "2K";
        break;
    case 10:
        cartType = "16IN1";
        break;
    }

    printf("cartType = %s\n", cartType.c_str());
    string cartId;
    settings = new Settings(&osystem);
    //   settings->setValue("romloadcount", false);
    rom_length = getromdata(&rom_ptr);

    cartridge = Cartridge::create((const uInt8 *)rom_ptr, (uInt32)rom_length, cartMD5, cartType, cartId, osystem, *settings);

    if (cartridge == 0)
    {
        printf("Stella: Failed to load cartridge.\n");
        return;
    }

    // Create the console
    console = new Console(&osystem, cartridge);
    osystem.myConsole = console;

    // Init sound and video
    console->initializeVideo();
    console->initializeAudio();

    // Get the ROM's width and height
    TIA &tia = console->tia();

    videoWidth = tia.width();
    videoHeight = tia.height();
    static uint32_t tiaSamplesPerFrame = (uint32_t)(AUDIO_A2600_SAMPLE_RATE / console->getFramerate());

    printf("videoWidth %d videoHeight %d\n", videoWidth, videoHeight);

    // G&W init
    common_emu_state.frame_time_10us = (uint16_t)(100000 / console->getFramerate() + 0.5f);

    // Black background
    memset(framebuffer1, 0, sizeof(framebuffer1));
    memset(framebuffer2, 0, sizeof(framebuffer2));

    odroid_system_init(APPID_A2600, AUDIO_A2600_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, NULL);

    // Init Sound
    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *)audiobuffer_dma, 2 * (tiaSamplesPerFrame));

    /* Set initial digital sensitivity */
    Paddles::setDigitalSensitivity(paddle_digital_sensitivity);

    while (1)
    {
        wdog_refresh();
        common_emu_frame_loop();
        odroid_input_read_gamepad(&joystick);
        common_emu_input_loop(&joystick, options, &blit);

        uint8_t turbo_buttons = odroid_settings_turbo_buttons_get();
        bool turbo_a = (joystick.values[ODROID_INPUT_A] && (turbo_buttons & 1));
        bool turbo_b = (joystick.values[ODROID_INPUT_B] && (turbo_buttons & 2));
        bool turbo_button = odroid_button_turbos();
        if (turbo_a)
            joystick.values[ODROID_INPUT_A] = turbo_button;
        if (turbo_b)
            joystick.values[ODROID_INPUT_B] = !turbo_button;

        update_joystick(&joystick);

        tia.update();

        blend_frames_16(tia.currentFrameBuffer(), videoWidth, videoHeight);
        common_ingame_overlay();
        osystem.sound().processFragment(sampleBuffer, tiaSamplesPerFrame);

        offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : tiaSamplesPerFrame;

        lcd_swap();
        sound_store(&audiobuffer_dma[offset], tiaSamplesPerFrame);

        if (!common_emu_state.skip_frames)
        {
            for (uint8_t p = 0; p < common_emu_state.pause_frames + 1; p++)
            {
                while (dma_state == last_dma_state)
                {
                    cpumon_sleep();
                }
                last_dma_state = dma_state;
            }
        }
    }
}

extern "C" int app_main_a2600(uint8_t load_state, uint8_t start_paused, int8_t save_slot)
{
    // Call static c++ constructors now, *after* OSPI and other memory is copied
    __libc_init_array();

    app_main_a2600_cpp(load_state, start_paused, save_slot);

    return 0;
}

#endif