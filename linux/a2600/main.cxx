#pragma GCC optimize("O0")

extern "C"
{

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gw_lcd.h"

#include <SDL2/SDL.h>

#include "odroid_system.h"
#include "porting.h"
#include "crc32.h"

#include "gw_malloc.h"

    // ROM Data
    extern const unsigned char ROM_DATA[];
    extern unsigned int ROM_DATA_LENGTH;
}

#include <cstdio>
#include <cstddef>
#include <cassert>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <algorithm>
#include <cmath>

#include <streams/file_stream.h>

#include "Console.hxx"
#include "Cart.hxx"
#include "MD5.hxx"
#include "Sound.hxx"
#include "SerialPort.hxx"
#include "TIA.hxx"
#include "Switches.hxx"
#include "StateManager.hxx"
#include "Sound.hxx"
#include "M6532.hxx"
#include "Version.hxx"
#include "Paddles.hxx"

#include "Stubs.hxx"

static Controller::Type left_controller_type = Controller::Joystick;

#define APP_ID 20

/*
 */
uint8_t a2600_y_offset = 0x24;
uint16_t a2600_height = 250;
char a2600_control[15] = ""; //"PADDLES";
bool a2600_control_swap = false;
bool a2600_swap_paddle = false;
bool a2600_fastscbios = false;
char a2600_display_mode[10] = "AUTO";
bool a2600_difficulty = 0;

#define AUDIO_A2600_SAMPLE_RATE 31400

static Console *console = 0;
static Cartridge *cartridge = 0;
static Settings *settings = 0;
static OSystem osystem;
static StateManager stateManager(&osystem);

static int videoWidth, videoHeight;

static int16_t sampleBuffer[2048];

#define A2600_WIDTH 160
#define A2600_HEIGHT 256
#define WIDTH 320
#define HEIGHT 240
#define FRAME_BUFFER_SIZE (WIDTH * HEIGHT * sizeof(uint16_t))
#define BPP 2
#define SCALE 2

#define VIDEO_PITCH (160)
#define VIDEO_REFRESH_RATE 60
static uint8_t frameBuffer[FRAME_BUFFER_SIZE];

//static uint8_t *frameBuffer = NULL;
static uint8_t *frameBufferPrev = NULL;
static uint8_t framePixelBytes = 2;
static const uint32_t *currentPalette32 = NULL;
static uint16_t currentPalette16[256] = {0};

static float stelladaptor_analog_sensitivity = 1.0f;
static float stelladaptor_analog_center = 0.0f;

static int paddle_digital_sensitivity = 500;

char index_palette = 0;

#define AUDIO_SAMPLE_RATE (48000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 60)
uint16_t audio_buffer[5000];

// Use 60Hz for GB
#define AUDIO_BUFFER_LENGTH_GB (AUDIO_SAMPLE_RATE / 60)
#define AUDIO_BUFFER_LENGTH_DMA_GB ((2 * AUDIO_SAMPLE_RATE) / 60)

static uint skipFrames = 0;

// Resampling
/* There are 35112 stereo sound samples in a video frame */
#define SOUND_SAMPLES_PER_FRAME 35112
/* We request 2064 samples from each call of GB::runFor() */
#define SOUND_SAMPLES_PER_RUN 2064
/* Native GB/GBC hardware audio sample rate (~2 MHz) */
#define SOUND_SAMPLE_RATE_NATIVE (VIDEO_REFRESH_RATE * (double)SOUND_SAMPLES_PER_FRAME)

#define SOUND_SAMPLE_RATE_CC (SOUND_SAMPLE_RATE_NATIVE / CC_DECIMATION_RATE) /* 65835Hz */

#define SOUND_BUFF_SIZE (SOUND_SAMPLES_PER_RUN + 2064)

TIA *tia = nullptr;

// SDL
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *fb_texture;

SDL_AudioSpec wanted;

uint32_t allocated_ram = 0;
uint32_t ahb_allocated_ram = 0;
uint32_t itc_allocated_ram = 0;

void *itc_calloc(size_t count, size_t size)
{
    itc_allocated_ram += size * count;
    printf("itc_calloc %zu bytes (new total = %u)\n", size * count, itc_allocated_ram);
    void *ret = calloc(count, size);
    return ret;
}

void *itc_malloc(size_t size)
{
    itc_allocated_ram += size;
    printf("itc_malloc %zu bytes (new total = %u)\n", size, itc_allocated_ram);
    void *ret = malloc(size);
    return ret;
}

#if 0
void * operator new(std::size_t n) throw(std::bad_alloc)
{
   printf("new %d\n",n);
   return malloc(n);
  //...
}
void operator delete(void * p) throw()
{
   printf("delete\n");
}
#endif

int init_window(int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO /*| SDL_INIT_AUDIO*/) != 0)
        return 0;

    window = SDL_CreateWindow("G&W Stella2014",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              width * SCALE, height * SCALE,
                              0);
    if (!window)
        return 0;

    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
        return 0;

    fb_texture = SDL_CreateTexture(renderer,
                                   SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING,
                                   width, height);
    if (!fb_texture)
        return 0;
#if 0
    /* Set the audio format */
    wanted.freq = SOUND_SAMPLE_RATE_CC;// AUDIO_SAMPLE_RATE;
    wanted.format = AUDIO_S16;
    wanted.channels = 2; /* 1 = mono, 2 = stereo */
    wanted.samples = 256;
    wanted.callback = fillaudio;
    wanted.userdata = NULL;

    BufferSize = soundbufsize * AUDIO_SAMPLE_RATE / 1000;

    BufferSize -= wanted.samples * 2;		/* SDL uses at least double-buffering, so
                            multiply by 2. */

    if(BufferSize < wanted.samples) BufferSize = wanted.samples;

    Buffer = (volatile int *)malloc(sizeof(int) * BufferSize);
    BufferRead = BufferWrite = BufferIn = 0;

    /* Open the audio device, forcing the desired format */
    if (SDL_OpenAudio(&wanted, NULL) < 0) {
        fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
        return(-1);
    }

    SDL_PauseAudio(0);
#endif
    return 0;
}

static bool SaveState(char *savePathName, char *sramPathName, int slot)
{
    return 0;
}

static bool LoadState(char *savePathName, char *sramPathName, int slot)
{
    return true;
}

static uint32_t joystick_state = 0; /* player input data, 1 byte per player (1-4) */

static bool run_loop = true;

int input_read_gamepad()
{
    SDL_Event event;
    uint8_t input_buf = joystick_state;

    if (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_x:
                input_buf |= 1 << 0;
                break;
            case SDLK_z:
                input_buf |= 1 << 1;
                break;
            case SDLK_LSHIFT:
                input_buf |= 1 << 3;
                break;
            case SDLK_LCTRL:
                input_buf |= 1 << 2;
                break;
            case SDLK_UP:
                input_buf |= 1 << 5;
                break;
            case SDLK_DOWN:
                input_buf |= 1 << 4;
                break;
            case SDLK_LEFT:
                input_buf |= 1 << 6;
                break;
            case SDLK_RIGHT:
                input_buf |= 1 << 7;
                break;
            case SDLK_ESCAPE:
                run_loop = false;
                break;
            case SDLK_F3:
                SaveState(NULL, NULL, 0);
                break;
            case SDLK_F4:
                LoadState(NULL, NULL, 0);
                break;
            case SDLK_p:
                index_palette++;
                break;
            default:
                break;
            }
        }
        if (event.type == SDL_KEYUP)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_x:
                input_buf &= ~(1 << 0);
                break;
            case SDLK_z:
                input_buf &= ~(1 << 1);
                break;
            case SDLK_LSHIFT:
                input_buf &= ~(1 << 3);
                break;
            case SDLK_LCTRL:
                input_buf &= ~(1 << 2);
                break;
            case SDLK_UP:
                input_buf &= ~(1 << 5);
                break;
            case SDLK_DOWN:
                input_buf &= ~(1 << 4);
                break;
            case SDLK_LEFT:
                input_buf &= ~(1 << 6);
                break;
            case SDLK_RIGHT:
                input_buf &= ~(1 << 7);
                break;
            case SDLK_ESCAPE:
                run_loop = false;
                break;
            default:
                break;
            }
        }
    }

    joystick_state = input_buf;
    return input_buf;
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
    if (width > 320)
        width = 320;
    if (height > 240)
        height = 240;

    const uint32_t *palette32 = console->getPalette(0);
    uint16_t *palette16 = currentPalette16;
    uInt8 *in = stella_fb;
    uint16_t *out = (uint16_t *)frameBuffer;
    int x, y;
    int yoffset = (240 - height) / 2;

    /* If palette has changed, re-cache converted
     * RGB565 values */
    if (palette32 != currentPalette32)
    {
        currentPalette32 = palette32;
        convert_palette(palette32, palette16);
    }

//    for (i = 0; i < width * height; i++)
//        *(out++) = *(palette16 + *(in++));

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            *(out + y * 320 + 2 * x) = *(palette16 + *(in++));
            *(out + y * 320 + 2 * x + 1) = *(out + y * 320 + 2 * x);
        }
    }

    SDL_UpdateTexture(fb_texture, NULL, frameBuffer, WIDTH * BPP);
    SDL_RenderCopy(renderer, fb_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

int main()
{
    string cartMD5 = "";

    // Load the cart
    string cartType = "AUTO";

    string cartId;
    settings = new Settings(&osystem);
    //   settings->setValue("romloadcount", false);
    cartridge = Cartridge::create((const uInt8 *)ROM_DATA, (uInt32)ROM_DATA_LENGTH, cartMD5, cartType, cartId, osystem, *settings);

    if (cartridge == 0)
    {
        printf("Stella: Failed to load cartridge.\n");
        return -1;
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

    // SDL
    unsigned flags = 0;
    printf("stella2014-go\n");

    // sets framebuffer1 as active buffer
    odroid_system_init(APP_ID, AUDIO_SAMPLE_RATE);
    odroid_system_emu_init(&LoadState, &SaveState, NULL);

    init_window(WIDTH, HEIGHT);
    // SDL

    /* Set initial digital sensitivity */
    Paddles::setDigitalSensitivity(paddle_digital_sensitivity);

    Event &ev = osystem.eventHandler().event();

    while (run_loop)
    {
        input_read_gamepad();
        tia.update();
        blend_frames_16(tia.currentFrameBuffer(), videoWidth, videoHeight);
        osystem.sound().processFragment(sampleBuffer, tiaSamplesPerFrame);

        #if 0
        // Tick before submitting audio/syncing
//        odroid_system_tick(!drawFrame, fullFrame, get_elapsed_time_since(startTime));

        // Paddle 0
        ev.set(Event::Type(Event::SALeftAxis0Value), paddle_a);
        ev.set(Event::Type(Event::PaddleZeroFire), joystick_state & (1 << 0));

        // Paddle 1
//        ev.set(Event::Type(Event::SALeftAxis1Value), paddle_b);
//        ev.set(Event::Type(Event::PaddleOneFire), joystick_state & (1 << 0));
#else

        // Generic inputs
        ev.set(Event::Type(Event::JoystickZeroUp), joystick_state & (1 << 5));
        ev.set(Event::Type(Event::JoystickZeroDown), joystick_state & (1 << 4));
        ev.set(Event::Type(Event::JoystickZeroLeft), joystick_state & (1 << 6));
        ev.set(Event::Type(Event::JoystickZeroRight), joystick_state & (1 << 7));
        ev.set(Event::Type(Event::JoystickZeroFire), joystick_state & (1 << 0));
        //    ev.set(Event::Type(Event::ConsoleLeftDiffA),  joystick_state & (1 << RETRO_DEVICE_ID_JOYPAD_L));
        //    ev.set(Event::Type(Event::ConsoleLeftDiffB),  joystick_state & (1 << RETRO_DEVICE_ID_JOYPAD_L2));
        //    ev.set(Event::Type(Event::ConsoleColor),      joystick_state & (1 << RETRO_DEVICE_ID_JOYPAD_L3));
        //    ev.set(Event::Type(Event::ConsoleRightDiffA), joystick_state & (1 << RETRO_DEVICE_ID_JOYPAD_R));
        //    ev.set(Event::Type(Event::ConsoleRightDiffB), joystick_state & (1 << RETRO_DEVICE_ID_JOYPAD_R2));
        //    ev.set(Event::Type(Event::ConsoleBlackWhite), joystick_state & (1 << RETRO_DEVICE_ID_JOYPAD_R3));
#endif

        ev.set(Event::Type(Event::ConsoleSelect), joystick_state & (1 << 3));
        ev.set(Event::Type(Event::ConsoleReset), joystick_state & (1 << 2));
        console->controller(Controller::Left).update();
        console->controller(Controller::Right).update();
        console->switches().update();
    }

    SDL_Quit();

    return 0;
}
