#include "build/config.h"

#if defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0

#include <string.h>
#include <ctype.h>
#include "gw_buttons.h"
#include "gw_lcd.h"
#include "gw_linker.h"
#include "common.h"
#include "rom_manager.h"
#include "rg_i18n.h"
#include "lz4_depack.h"
#include <assert.h>
#include  "miniz.h"
#include "lzma.h"
#include "appid.h"
#include "fceu.h"
#include "fceu-state.h"
#include "fceu-cart.h"
#include "fds.h"
#include "driver.h"
#include "video.h"
#include "gw_malloc.h"
#include "nes_memory_stream.h"

#define NES_WIDTH  256
#define NES_HEIGHT 240

extern uint32_t ram_start;

extern CartInfo iNESCart;

static uint8_t nes_framebuffer[(NES_WIDTH+16)*NES_HEIGHT];
static bool crop_overscan_v;
static bool crop_overscan_h;

static char palette_values_text[50];
static char sprite_limit_text[5];
static char crop_overscan_v_text[5];
static char crop_overscan_h_text[5];
static char next_disk_text[32];
static char eject_insert_text[32];
static char overclocking_text[32];
static uint8_t palette_index = 0;
static uint8_t overclocking_type = 0;
static uint8_t allow_swap_disk = 0;
static bool disable_sprite_limit = false;

uint8_t *UNIFchrrama = 0;

unsigned overclock_enabled = -1;
unsigned overclocked = 0;
unsigned skip_7bit_overclocking = 1; /* 7-bit samples have priority over overclocking */
unsigned totalscanlines = 240;
unsigned normal_scanlines = 240;
unsigned extrascanlines = 0;
unsigned vblankscanlines = 0;

#define NES_FREQUENCY_18K 18000 // 18 kHz to limit cpu usage
#define NES_FREQUENCY_48K 48000
static uint samplesPerFrame;

static int32_t *sound = 0;

static uint32_t fceu_joystick; /* player input data, 1 byte per player (1-4) */

static bool crop_overscan_v_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat);
static bool crop_overscan_h_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat);
static bool fds_eject_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat);
static bool fds_side_swap_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat);
static bool overclocking_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat);

static SFORMAT gnw_save_data[] = {
	{ &crop_overscan_v, 1, "HCRO" },
	{ &crop_overscan_h, 1, "VCRO" },
	{ &overclocking_type, 1, "OCPR" },
	{ &disable_sprite_limit, 1, "SPLI" },
    { &palette_index, 1, "NPAL"},
	{ 0 }
};

/* table for currently loaded palette */
static uint8_t base_palette[192];

struct st_palettes {
   char name[21];
   unsigned int data[64];
};

static struct st_palettes palettes[] __attribute__((section(".extflash_emu_data"))) = {
   { "asqrealc", /*"AspiringSquire's Real palette",*/
      { 0x6c6c6c, 0x00268e, 0x0000a8, 0x400094,
         0x700070, 0x780040, 0x700000, 0x621600,
         0x442400, 0x343400, 0x005000, 0x004444,
         0x004060, 0x000000, 0x101010, 0x101010,
         0xbababa, 0x205cdc, 0x3838ff, 0x8020f0,
         0xc000c0, 0xd01474, 0xd02020, 0xac4014,
         0x7c5400, 0x586400, 0x008800, 0x007468,
         0x00749c, 0x202020, 0x101010, 0x101010,
         0xffffff, 0x4ca0ff, 0x8888ff, 0xc06cff,
         0xff50ff, 0xff64b8, 0xff7878, 0xff9638,
         0xdbab00, 0xa2ca20, 0x4adc4a, 0x2ccca4,
         0x1cc2ea, 0x585858, 0x101010, 0x101010,
         0xffffff, 0xb0d4ff, 0xc4c4ff, 0xe8b8ff,
         0xffb0ff, 0xffb8e8, 0xffc4c4, 0xffd4a8,
         0xffe890, 0xf0f4a4, 0xc0ffc0, 0xacf4f0,
         0xa0e8ff, 0xc2c2c2, 0x202020, 0x101010 }
   },
   { "nintendo-vc", /*"Virtual Console palette",*/
      { 0x494949, 0x00006a, 0x090063, 0x290059,
         0x42004a, 0x490000, 0x420000, 0x291100,
         0x182700, 0x003010, 0x003000, 0x002910,
         0x012043, 0x000000, 0x000000, 0x000000,
         0x747174, 0x003084, 0x3101ac, 0x4b0194,
         0x64007b, 0x6b0039, 0x6b2101, 0x5a2f00,
         0x424900, 0x185901, 0x105901, 0x015932,
         0x01495a, 0x101010, 0x000000, 0x000000,
         0xadadad, 0x4a71b6, 0x6458d5, 0x8450e6,
         0xa451ad, 0xad4984, 0xb5624a, 0x947132,
         0x7b722a, 0x5a8601, 0x388e31, 0x318e5a,
         0x398e8d, 0x383838, 0x000000, 0x000000,
         0xb6b6b6, 0x8c9db5, 0x8d8eae, 0x9c8ebc,
         0xa687bc, 0xad8d9d, 0xae968c, 0x9c8f7c,
         0x9c9e72, 0x94a67c, 0x84a77b, 0x7c9d84,
         0x73968d, 0xdedede, 0x000000, 0x000000 }
   },
   { "rgb", /*"Nintendo RGB PPU palette",*/
      { 0x6D6D6D, 0x002492, 0x0000DB, 0x6D49DB,
         0x92006D, 0xB6006D, 0xB62400, 0x924900,
         0x6D4900, 0x244900, 0x006D24, 0x009200,
         0x004949, 0x000000, 0x000000, 0x000000,
         0xB6B6B6, 0x006DDB, 0x0049FF, 0x9200FF,
         0xB600FF, 0xFF0092, 0xFF0000, 0xDB6D00,
         0x926D00, 0x249200, 0x009200, 0x00B66D,
         0x009292, 0x242424, 0x000000, 0x000000,
         0xFFFFFF, 0x6DB6FF, 0x9292FF, 0xDB6DFF,
         0xFF00FF, 0xFF6DFF, 0xFF9200, 0xFFB600,
         0xDBDB00, 0x6DDB00, 0x00FF00, 0x49FFDB,
         0x00FFFF, 0x494949, 0x000000, 0x000000,
         0xFFFFFF, 0xB6DBFF, 0xDBB6FF, 0xFFB6FF,
         0xFF92FF, 0xFFB6B6, 0xFFDB92, 0xFFFF49,
         0xFFFF6D, 0xB6FF49, 0x92FF6D, 0x49FFDB,
         0x92DBFF, 0x929292, 0x000000, 0x000000 }
   },
   { "yuv-v3", /*"FBX's YUV-V3 palette",*/
      { 0x666666, 0x002A88, 0x1412A7, 0x3B00A4,
         0x5C007E, 0x6E0040, 0x6C0700, 0x561D00,
         0x333500, 0x0C4800, 0x005200, 0x004C18,
         0x003E5B, 0x000000, 0x000000, 0x000000,
         0xADADAD, 0x155FD9, 0x4240FF, 0x7527FE,
         0xA01ACC, 0xB71E7B, 0xB53120, 0x994E00,
         0x6B6D00, 0x388700, 0x0D9300, 0x008C47,
         0x007AA0, 0x000000, 0x000000, 0x000000,
         0xFFFFFF, 0x64B0FF, 0x9290FF, 0xC676FF,
         0xF26AFF, 0xFF6ECC, 0xFF8170, 0xEA9E22,
         0xBCBE00, 0x88D800, 0x5CE430, 0x45E082,
         0x48CDDE, 0x4F4F4F, 0x000000, 0x000000,
         0xFFFFFF, 0xC0DFFF, 0xD3D2FF, 0xE8C8FF,
         0xFAC2FF, 0xFFC4EA, 0xFFCCC5, 0xF7D8A5,
         0xE4E594, 0xCFEF96, 0xBDF4AB, 0xB3F3CC,
         0xB5EBF2, 0xB8B8B8, 0x000000, 0x000000 }
   },
   { "unsaturated-final", /*"FBX's Unsaturated-Final palette",*/
      { 0x676767, 0x001F8E, 0x23069E, 0x40008E,
         0x600067, 0x67001C, 0x5B1000, 0x432500,
         0x313400, 0x074800, 0x004F00, 0x004622,
         0x003A61, 0x000000, 0x000000, 0x000000,
         0xB3B3B3, 0x205ADF, 0x5138FB, 0x7A27EE,
         0xA520C2, 0xB0226B, 0xAD3702, 0x8D5600,
         0x6E7000, 0x2E8A00, 0x069200, 0x008A47,
         0x037B9B, 0x101010, 0x000000, 0x000000,
         0xFFFFFF, 0x62AEFF, 0x918BFF, 0xBC78FF,
         0xE96EFF, 0xFC6CCD, 0xFA8267, 0xE29B26,
         0xC0B901, 0x84D200, 0x58DE38, 0x46D97D,
         0x49CED2, 0x494949, 0x000000, 0x000000,
         0xFFFFFF, 0xC1E3FF, 0xD5D4FF, 0xE7CCFF,
         0xFBC9FF, 0xFFC7F0, 0xFFD0C5, 0xF8DAAA,
         0xEBE69A, 0xD1F19A, 0xBEF7AF, 0xB6F4CD,
         0xB7F0EF, 0xB2B2B2, 0x000000, 0x000000 }
   },
   { "sony-cxa2025as-us", /*"Sony CXA2025AS US palette",*/
      { 0x585858, 0x00238C, 0x00139B, 0x2D0585,
         0x5D0052, 0x7A0017, 0x7A0800, 0x5F1800,
         0x352A00, 0x093900, 0x003F00, 0x003C22,
         0x00325D, 0x000000, 0x000000, 0x000000,
         0xA1A1A1, 0x0053EE, 0x153CFE, 0x6028E4,
         0xA91D98, 0xD41E41, 0xD22C00, 0xAA4400,
         0x6C5E00, 0x2D7300, 0x007D06, 0x007852,
         0x0069A9, 0x000000, 0x000000, 0x000000,
         0xFFFFFF, 0x1FA5FE, 0x5E89FE, 0xB572FE,
         0xFE65F6, 0xFE6790, 0xFE773C, 0xFE9308,
         0xC4B200, 0x79CA10, 0x3AD54A, 0x11D1A4,
         0x06BFFE, 0x424242, 0x000000, 0x000000,
         0xFFFFFF, 0xA0D9FE, 0xBDCCFE, 0xE1C2FE,
         0xFEBCFB, 0xFEBDD0, 0xFEC5A9, 0xFED18E,
         0xE9DE86, 0xC7E992, 0xA8EEB0, 0x95ECD9,
         0x91E4FE, 0xACACAC, 0x000000, 0x000000 }
   },
   { "pal", /*"PAL palette",*/
      { 0x808080, 0x0000BA, 0x3700BF, 0x8400A6,
         0xBB006A, 0xB7001E, 0xB30000, 0x912600,
         0x7B2B00, 0x003E00, 0x00480D, 0x003C22,
         0x002F66, 0x000000, 0x050505, 0x050505,
         0xC8C8C8, 0x0059FF, 0x443CFF, 0xB733CC,
         0xFE33AA, 0xFE375E, 0xFE371A, 0xD54B00,
         0xC46200, 0x3C7B00, 0x1D8415, 0x009566,
         0x0084C4, 0x111111, 0x090909, 0x090909,
         0xFEFEFE, 0x0095FF, 0x6F84FF, 0xD56FFF,
         0xFE77CC, 0xFE6F99, 0xFE7B59, 0xFE915F,
         0xFEA233, 0xA6BF00, 0x51D96A, 0x4DD5AE,
         0x00D9FF, 0x666666, 0x0D0D0D, 0x0D0D0D,
         0xFEFEFE, 0x84BFFF, 0xBBBBFF, 0xD0BBFF,
         0xFEBFEA, 0xFEBFCC, 0xFEC4B7, 0xFECCAE,
         0xFED9A2, 0xCCE199, 0xAEEEB7, 0xAAF8EE,
         0xB3EEFF, 0xDDDDDD, 0x111111, 0x111111 }
   },
   { "bmf-final2", /*"BMF's Final 2 palette",*/
      { 0x525252, 0x000080, 0x08008A, 0x2C007E,
         0x4A004E, 0x500006, 0x440000, 0x260800,
         0x0A2000, 0x002E00, 0x003200, 0x00260A,
         0x001C48, 0x000000, 0x000000, 0x000000,
         0xA4A4A4, 0x0038CE, 0x3416EC, 0x5E04DC,
         0x8C00B0, 0x9A004C, 0x901800, 0x703600,
         0x4C5400, 0x0E6C00, 0x007400, 0x006C2C,
         0x005E84, 0x000000, 0x000000, 0x000000,
         0xFFFFFF, 0x4C9CFF, 0x7C78FF, 0xA664FF,
         0xDA5AFF, 0xF054C0, 0xF06A56, 0xD68610,
         0xBAA400, 0x76C000, 0x46CC1A, 0x2EC866,
         0x34C2BE, 0x3A3A3A, 0x000000, 0x000000,
         0xFFFFFF, 0xB6DAFF, 0xC8CAFF, 0xDAC2FF,
         0xF0BEFF, 0xFCBCEE, 0xFAC2C0, 0xF2CCA2,
         0xE6DA92, 0xCCE68E, 0xB8EEA2, 0xAEEABE,
         0xAEE8E2, 0xB0B0B0, 0x000000, 0x000000 }
   },
   { "bmf-final3", /*"BMF's Final 3 palette",*/
      { 0x686868, 0x001299, 0x1A08AA, 0x51029A,
         0x7E0069, 0x8E001C, 0x7E0301, 0x511800,
         0x1F3700, 0x014E00, 0x005A00, 0x00501C,
         0x004061, 0x000000, 0x000000, 0x000000,
         0xB9B9B9, 0x0C5CD7, 0x5035F0, 0x8919E0,
         0xBB0CB3, 0xCE0C61, 0xC02B0E, 0x954D01,
         0x616F00, 0x1F8B00, 0x01980C, 0x00934B,
         0x00819B, 0x000000, 0x000000, 0x000000,
         0xFFFFFF, 0x63B4FF, 0x9B91FF, 0xD377FF,
         0xEF6AFF, 0xF968C0, 0xF97D6C, 0xED9B2D,
         0xBDBD16, 0x7CDA1C, 0x4BE847, 0x35E591,
         0x3FD9DD, 0x606060, 0x000000, 0x000000,
         0xFFFFFF, 0xACE7FF, 0xD5CDFF, 0xEDBAFF,
         0xF8B0FF, 0xFEB0EC, 0xFDBDB5, 0xF9D28E,
         0xE8EB7C, 0xBBF382, 0x99F7A2, 0x8AF5D0,
         0x92F4F1, 0xBEBEBE, 0x000000, 0x000000 }
   },
   { "smooth-fbx", /*"FBX's Smooth palette",*/
      { 0x6A6D6A, 0x001380, 0x1E008A, 0x39007A,
         0x550056, 0x5A0018, 0x4F1000, 0x3D1C00,
         0x253200, 0x003D00, 0x004000, 0x003924,
         0x002E55, 0x000000, 0x000000, 0x000000,
         0xB9BCB9, 0x1850C7, 0x4B30E3, 0x7322D6,
         0x951FA9, 0x9D285C, 0x983700, 0x7F4C00,
         0x5E6400, 0x227700, 0x027E02, 0x007645,
         0x006E8A, 0x000000, 0x000000, 0x000000,
         0xFFFFFF, 0x68A6FF, 0x8C9CFF, 0xB586FF,
         0xD975FD, 0xE377B9, 0xE58D68, 0xD49D29,
         0xB3AF0C, 0x7BC211, 0x55CA47, 0x46CB81,
         0x47C1C5, 0x4A4D4A, 0x000000, 0x000000,
         0xFFFFFF, 0xCCEAFF, 0xDDDEFF, 0xECDAFF,
         0xF8D7FE, 0xFCD6F5, 0xFDDBCF, 0xF9E7B5,
         0xF1F0AA, 0xDAFAA9, 0xC9FFBC, 0xC3FBD7,
         0xC4F6F6, 0xBEC1BE, 0x000000, 0x000000 }
   },
   { "composite-direct-fbx", /*"FBX's Composite Direct palette",*/
      { 0x656565, 0x00127D, 0x18008E, 0x360082,
         0x56005D, 0x5A0018, 0x4F0500, 0x381900,
         0x1D3100, 0x003D00, 0x004100, 0x003B17,
         0x002E55, 0x000000, 0x000000, 0x000000,
         0xAFAFAF, 0x194EC8, 0x472FE3, 0x6B1FD7,
         0x931BAE, 0x9E1A5E, 0x993200, 0x7B4B00,
         0x5B6700, 0x267A00, 0x008200, 0x007A3E,
         0x006E8A, 0x000000, 0x000000, 0x000000,
         0xFFFFFF, 0x64A9FF, 0x8E89FF, 0xB676FF,
         0xE06FFF, 0xEF6CC4, 0xF0806A, 0xD8982C,
         0xB9B40A, 0x83CB0C, 0x5BD63F, 0x4AD17E,
         0x4DC7CB, 0x4C4C4C, 0x000000, 0x000000,
         0xFFFFFF, 0xC7E5FF, 0xD9D9FF, 0xE9D1FF,
         0xF9CEFF, 0xFFCCF1, 0xFFD4CB, 0xF8DFB1,
         0xEDEAA4, 0xD6F4A4, 0xC5F8B8, 0xBEF6D3,
         0xBFF1F1, 0xB9B9B9, 0x000000, 0x000000 }
   },
   { "pvm-style-d93-fbx", /*"FBX's PVM Style D93 palette",*/
      { 0x696B63, 0x001774, 0x1E0087, 0x340073,
         0x560057, 0x5E0013, 0x531A00, 0x3B2400,
         0x243000, 0x063A00, 0x003F00, 0x003B1E,
         0x00334E, 0x000000, 0x000000, 0x000000,
         0xB9BBB3, 0x1453B9, 0x4D2CDA, 0x671EDE,
         0x98189C, 0x9D2344, 0xA03E00, 0x8D5500,
         0x656D00, 0x2C7900, 0x008100, 0x007D42,
         0x00788A, 0x000000, 0x000000, 0x000000,
         0xFFFFFF, 0x69A8FF, 0x9691FF, 0xB28AFA,
         0xEA7DFA, 0xF37BC7, 0xF28E59, 0xE6AD27,
         0xD7C805, 0x90DF07, 0x64E53C, 0x45E27D,
         0x48D5D9, 0x4E5048, 0x000000, 0x000000,
         0xFFFFFF, 0xD2EAFF, 0xE2E2FF, 0xE9D8FF,
         0xF5D2FF, 0xF8D9EA, 0xFADEB9, 0xF9E89B,
         0xF3F28C, 0xD3FA91, 0xB8FCA8, 0xAEFACA,
         0xCAF3F3, 0xBEC0B8, 0x000000, 0x000000 }
   },
   { "ntsc-hardware-fbx", /*"FBX's NTSC Hardware palette",*/
      { 0x6A6D6A, 0x001380, 0x1E008A, 0x39007A,
         0x550056, 0x5A0018, 0x4F1000, 0x382100,
         0x213300, 0x003D00, 0x004000, 0x003924,
         0x002E55, 0x000000, 0x000000, 0x000000,
         0xB9BCB9, 0x1850C7, 0x4B30E3, 0x7322D6,
         0x951FA9, 0x9D285C, 0x963C00, 0x7A5100,
         0x5B6700, 0x227700, 0x027E02, 0x007645,
         0x006E8A, 0x000000, 0x000000, 0x000000,
         0xFFFFFF, 0x68A6FF, 0x9299FF, 0xB085FF,
         0xD975FD, 0xE377B9, 0xE58D68, 0xCFA22C,
         0xB3AF0C, 0x7BC211, 0x55CA47, 0x46CB81,
         0x47C1C5, 0x4A4D4A, 0x000000, 0x000000,
         0xFFFFFF, 0xCCEAFF, 0xDDDEFF, 0xECDAFF,
         0xF8D7FE, 0xFCD6F5, 0xFDDBCF, 0xF9E7B5,
         0xF1F0AA, 0xDAFAA9, 0xC9FFBC, 0xC3FBD7,
         0xC4F6F6, 0xBEC1BE, 0x000000, 0x000000 }
   },
   { "nes-classic-fbx-fs", /*"FBX's NES-Classic FS palette",*/
      { 0x60615F, 0x000083, 0x1D0195, 0x340875,
         0x51055E, 0x56000F, 0x4C0700, 0x372308,
         0x203A0B, 0x0F4B0E, 0x194C16, 0x02421E,
         0x023154, 0x000000, 0x000000, 0x000000,
         0xA9AAA8, 0x104BBF, 0x4712D8, 0x6300CA,
         0x8800A9, 0x930B46, 0x8A2D04, 0x6F5206,
         0x5C7114, 0x1B8D12, 0x199509, 0x178448,
         0x206B8E, 0x000000, 0x000000, 0x000000,
         0xFBFBFB, 0x6699F8, 0x8974F9, 0xAB58F8,
         0xD557EF, 0xDE5FA9, 0xDC7F59, 0xC7A224,
         0xA7BE03, 0x75D703, 0x60E34F, 0x3CD68D,
         0x56C9CC, 0x414240, 0x000000, 0x000000,
         0xFBFBFB, 0xBED4FA, 0xC9C7F9, 0xD7BEFA,
         0xE8B8F9, 0xF5BAE5, 0xF3CAC2, 0xDFCDA7,
         0xD9E09C, 0xC9EB9E, 0xC0EDB8, 0xB5F4C7,
         0xB9EAE9, 0xABABAB, 0x000000, 0x000000 }
   },
   { "nescap", /*"RGBSource's NESCAP palette",*/
      { 0x646365, 0x001580, 0x1D0090, 0x380082,
         0x56005D, 0x5A001A, 0x4F0900, 0x381B00,
         0x1E3100, 0x003D00, 0x004100, 0x003A1B,
         0x002F55, 0x000000, 0x000000, 0x000000,
         0xAFADAF, 0x164BCA, 0x472AE7, 0x6B1BDB,
         0x9617B0, 0x9F185B, 0x963001, 0x7B4800,
         0x5A6600, 0x237800, 0x017F00, 0x00783D,
         0x006C8C, 0x000000, 0x000000, 0x000000,
         0xFFFFFF, 0x60A6FF, 0x8F84FF, 0xB473FF,
         0xE26CFF, 0xF268C3, 0xEF7E61, 0xD89527,
         0xBAB307, 0x81C807, 0x57D43D, 0x47CF7E,
         0x4BC5CD, 0x4C4B4D, 0x000000, 0x000000,
         0xFFFFFF, 0xC2E0FF, 0xD5D2FF, 0xE3CBFF,
         0xF7C8FF, 0xFEC6EE, 0xFECEC6, 0xF6D7AE,
         0xE9E49F, 0xD3ED9D, 0xC0F2B2, 0xB9F1CC,
         0xBAEDED, 0xBAB9BB, 0x000000, 0x000000 }
   },
   { "wavebeam", /*"nakedarthur's Wavebeam palette",*/
      { 0X6B6B6B, 0X001B88, 0X21009A, 0X40008C,
         0X600067, 0X64001E, 0X590800, 0X481600,
         0X283600, 0X004500, 0X004908, 0X00421D,
         0X003659, 0X000000, 0X000000, 0X000000,
         0XB4B4B4, 0X1555D3, 0X4337EF, 0X7425DF,
         0X9C19B9, 0XAC0F64, 0XAA2C00, 0X8A4B00,
         0X666B00, 0X218300, 0X008A00, 0X008144,
         0X007691, 0X000000, 0X000000, 0X000000,
         0XFFFFFF, 0X63B2FF, 0X7C9CFF, 0XC07DFE,
         0XE977FF, 0XF572CD, 0XF4886B, 0XDDA029,
         0XBDBD0A, 0X89D20E, 0X5CDE3E, 0X4BD886,
         0X4DCFD2, 0X525252, 0X000000, 0X000000,
         0XFFFFFF, 0XBCDFFF, 0XD2D2FF, 0XE1C8FF,
         0XEFC7FF, 0XFFC3E1, 0XFFCAC6, 0XF2DAAD,
         0XEBE3A0, 0XD2EDA2, 0XBCF4B4, 0XB5F1CE,
         0XB6ECF1, 0XBFBFBF, 0X000000, 0X000000 }
   },
   { "digital-prime-fbx", /*"FBX's Digital Prime palette",*/
      { 0x616161, 0x000088, 0x1F0D99, 0x371379,
         0x561260, 0x5D0010, 0x520E00, 0x3A2308,
         0x21350C, 0x0D410E, 0x174417, 0x003A1F,
         0x002F57, 0x000000, 0x000000, 0x000000,
         0xAAAAAA, 0x0D4DC4, 0x4B24DE, 0x6912CF,
         0x9014AD, 0x9D1C48, 0x923404, 0x735005,
         0x5D6913, 0x167A11, 0x138008, 0x127649,
         0x1C6691, 0x000000, 0x000000, 0x000000,
         0xFCFCFC, 0x639AFC, 0x8A7EFC, 0xB06AFC,
         0xDD6DF2, 0xE771AB, 0xE38658, 0xCC9E22,
         0xA8B100, 0x72C100, 0x5ACD4E, 0x34C28E,
         0x4FBECE, 0x424242, 0x000000, 0x000000,
         0xFCFCFC, 0xBED4FC, 0xCACAFC, 0xD9C4FC,
         0xECC1FC, 0xFAC3E7, 0xF7CEC3, 0xE2CDA7,
         0xDADB9C, 0xC8E39E, 0xBFE5B8, 0xB2EBC8,
         0xB7E5EB, 0xACACAC, 0x000000, 0x000000 }
   },
   { "magnum-fbx", /*"FBX's Magnum palette",*/
      { 0x696969, 0x00148F, 0x1E029B, 0x3F008A,
         0x600060, 0x660017, 0x570D00, 0x451B00,
         0x243400, 0x004200, 0x004500, 0x003C1F,
         0x00315C, 0x000000, 0x000000, 0x000000,
         0xAFAFAF, 0x0F51DD, 0x442FF3, 0x7220E2,
         0xA319B3, 0xAE1C51, 0xA43400, 0x884D00,
         0x676D00, 0x208000, 0x008B00, 0x007F42,
         0x006C97, 0x010101, 0x000000, 0x000000,
         0xFFFFFF, 0x65AAFF, 0x8C96FF, 0xB983FF,
         0xDD6FFF, 0xEA6FBD, 0xEB8466, 0xDCA21F,
         0xBAB403, 0x7ECB07, 0x54D33E, 0x3CD284,
         0x3EC7CC, 0x4B4B4B, 0x000000, 0x000000,
         0xFFFFFF, 0xBDE2FF, 0xCECFFF, 0xE6C2FF,
         0xF6BCFF, 0xF9C2ED, 0xFACFC6, 0xF8DEAC,
         0xEEE9A1, 0xD0F59F, 0xBBF5AF, 0xB3F5CD,
         0xB9EDF0, 0xB9B9B9, 0x000000, 0x000000 }
   },
   { "smooth-v2-fbx", /*"FBX's Smooth V2 palette",*/
      { 0x6A6A6A, 0x00148F, 0x1E029B, 0x3F008A,
         0x600060, 0x660017, 0x570D00, 0x3C1F00,
         0x1B3300, 0x004200, 0x004500, 0x003C1F,
         0x00315C, 0x000000, 0x000000, 0x000000,
         0xB9B9B9, 0x0F4BD4, 0x412DEB, 0x6C1DD9,
         0x9C17AB, 0xA71A4D, 0x993200, 0x7C4A00,
         0x546400, 0x1A7800, 0x007F00, 0x00763E,
         0x00678F, 0x010101, 0x000000, 0x000000,
         0xFFFFFF, 0x68A6FF, 0x8C9CFF, 0xB586FF,
         0xD975FD, 0xE377B9, 0xE58D68, 0xD49D29,
         0xB3AF0C, 0x7BC211, 0x55CA47, 0x46CB81,
         0x47C1C5, 0x4A4A4A, 0x000000, 0x000000,
         0xFFFFFF, 0xCCEAFF, 0xDDDEFF, 0xECDAFF,
         0xF8D7FE, 0xFCD6F5, 0xFDDBCF, 0xF9E7B5,
         0xF1F0AA, 0xDAFAA9, 0xC9FFBC, 0xC3FBD7,
         0xC4F6F6, 0xBEBEBE, 0x000000, 0x000000 }
   },
   { "nes-classic-fbx", /*"FBX's NES Classic palette",*/
      { 0x616161, 0x000088, 0x1F0D99, 0x371379,
         0x561260, 0x5D0010, 0x520E00, 0x3A2308,
         0x21350C, 0x0D410E, 0x174417, 0x003A1F,
         0x002F57, 0x000000, 0x000000, 0x000000,
         0xAAAAAA, 0x0D4DC4, 0x4B24DE, 0x6912CF,
         0x9014AD, 0x9D1C48, 0x923404, 0x735005,
         0x5D6913, 0x167A11, 0x138008, 0x127649,
         0x1C6691, 0x000000, 0x000000, 0x000000,
         0xFCFCFC, 0x639AFC, 0x8A7EFC, 0xB06AFC,
         0xDD6DF2, 0xE771AB, 0xE38658, 0xCC9E22,
         0xA8B100, 0x72C100, 0x5ACD4E, 0x34C28E,
         0x4FBECE, 0x424242, 0x000000, 0x000000,
         0xFCFCFC, 0xBED4FC, 0xCACAFC, 0xD9C4FC,
         0xECC1FC, 0xFAC3E7, 0xF7CEC3, 0xE2CDA7,
         0xDADB9C, 0xC8E39E, 0xBFE5B8, 0xB2EBC8,
         0xB7E5EB, 0xACACAC, 0x000000, 0x000000 }
   }
};

void setCustomPalette(uint8_t palette_idx) {
      unsigned *palette_data = palettes[palette_idx].data;
      for (int i = 0; i < 64; i++ )
      {
         unsigned data = palette_data[i];
         base_palette[ i * 3 + 0 ] = ( data >> 16 ) & 0xff; /* red */
         base_palette[ i * 3 + 1 ] = ( data >>  8 ) & 0xff; /* green */
         base_palette[ i * 3 + 2 ] = ( data >>  0 ) & 0xff; /* blue */
      }
      FCEUI_SetPaletteArray( base_palette );
}

void FCEUD_PrintError(char *c)
{
    printf("%s", c);
}

void FCEUD_DispMessage(enum retro_log_level level, unsigned duration, const char *str)
{
    printf("%s", str);
}

void FCEUD_Message(char *s)
{
    printf("%s", s);
}

static bool SaveState(char *pathName)
{
    if (ACTIVE_FILE->save_size > 0) {
#if OFF_SAVESTATE==1
        if (strcmp(pathName,"1") == 0) {
            // Save in common save slot (during a power off)
            memstream_set_buffer((uint8_t*)&__OFFSAVEFLASH_START__, (uint64_t)ACTIVE_FILE->save_size);
            FCEUSS_Save_Mem();
        } else {
#endif
            memstream_set_buffer((uint8_t*)ACTIVE_FILE->save_address, (uint64_t)ACTIVE_FILE->save_size);
            FCEUSS_Save_Mem();
#if OFF_SAVESTATE==1
        }
#endif
    }
    return 0;
}

// TODO: Expose properly
extern int nes_state_load(uint8_t* flash_ptr, size_t size);

static bool LoadState(char *pathName)
{
    memstream_set_buffer((uint8_t*)ACTIVE_FILE->save_address, ACTIVE_FILE->save_size);
    FCEUSS_Load_Mem();
    return true;
}

// TODO: Move to lcd.c/h
extern LTDC_HandleTypeDef hltdc;
unsigned dendy = 0;

static uint16_t palette565[256];
static uint32_t palette_spaced_565[256];

#define RED_SHIFT 11
#define GREEN_SHIFT 5
#define BLUE_SHIFT 0
#define RED_EXPAND 3
#define GREEN_EXPAND 2
#define BLUE_EXPAND 3
#define BUILD_PIXEL_RGB565(R,G,B) (((int) ((R)&0x1f) << RED_SHIFT) | ((int) ((G)&0x3f) << GREEN_SHIFT) | ((int) ((B)&0x1f) << BLUE_SHIFT))

void FCEUD_SetPalette(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t color_565 = BUILD_PIXEL_RGB565(r >> RED_EXPAND, g >> GREEN_EXPAND, b >> BLUE_EXPAND);
    palette565[index] = color_565;
    uint32_t sc = ((0b1111100000000000&color_565)<<10) | ((0b0000011111100000&color_565)<<5) | ((0b0000000000011111&color_565));
    palette_spaced_565[index] = sc;
}

static void nesInputUpdate(odroid_gamepad_state_t *joystick)
{
    uint8_t input_buf  = 0;
    if (joystick->values[ODROID_INPUT_LEFT]) {
        input_buf |= JOY_LEFT;
    }
    if (joystick->values[ODROID_INPUT_RIGHT]) {
        input_buf |= JOY_RIGHT;
    }
    if (joystick->values[ODROID_INPUT_UP]) {
        input_buf |= JOY_UP;
    }
    if (joystick->values[ODROID_INPUT_DOWN]) {
        input_buf |= JOY_DOWN;
    }
    if (joystick->values[ODROID_INPUT_A]) {
        input_buf |= JOY_A;
    }
    if (joystick->values[ODROID_INPUT_B]) {
        input_buf |= JOY_B;
    }
    // Game button on G&W
    if (joystick->values[ODROID_INPUT_START]) {
        input_buf |= JOY_START;
    }
    // Time button on G&W
    if (joystick->values[ODROID_INPUT_SELECT]) {
        input_buf |= JOY_SELECT;
    }
    // Start button on Zelda G&W
    if (joystick->values[ODROID_INPUT_X]) {
        input_buf |= JOY_START;
    }
    // Select button on Zelda G&W
    if (joystick->values[ODROID_INPUT_Y]) {
        input_buf |= JOY_SELECT;
    }
    fceu_joystick = input_buf;
}

// No scaling
__attribute__((optimize("unroll-loops")))
static inline void blit_normal(uint8_t *src, uint16_t *framebuffer) {
    uint32_t x, y;
    uint8_t incr   = 0;
    uint16_t width  = NES_WIDTH;
    uint16_t height = NES_HEIGHT;
    uint8_t offset_x  = (GW_LCD_WIDTH - width) / 2;
    uint8_t offset_y  = 0;

    incr     += (crop_overscan_h ? 16 : 0);
    width    -= (crop_overscan_h ? 16 : 0);
    height   -= (crop_overscan_v ? 16 : 0);
    src      += (crop_overscan_v ? ((crop_overscan_h ? 8 : 0) + NES_WIDTH * 8) : (crop_overscan_h ? 8 : 0));
    offset_x += (crop_overscan_h ? 8 : 0);
    offset_y  = (crop_overscan_v ? 8 : 0);

    for (y = 0; y < height; y++, src += incr) {
        for (x = 0; x < width; x++, src++) {
            framebuffer[(y+offset_y) * GW_LCD_WIDTH + x + offset_x] = palette565[*src];
        }
    }
}

__attribute__((optimize("unroll-loops")))
static inline void screen_blit_nn(uint8_t *src, uint16_t *framebuffer)
{
    uint16_t w1 = NES_WIDTH - (crop_overscan_h ? 16 : 0);
    uint16_t h1 = NES_HEIGHT - (crop_overscan_v ? 16 : 0);
    uint16_t w2 = GW_LCD_WIDTH;
    uint16_t h2 = GW_LCD_HEIGHT;
    uint8_t src_x_offset = (crop_overscan_h ? 8 : 0);
    uint8_t src_y_offset = (crop_overscan_v ? 8 : 0);
    int x_ratio = (int)((w1<<16)/w2) +1;
    int y_ratio = (int)((h1<<16)/h2) +1;

    int x2;
    int y2;

    for (int i=0;i<h2;i++) {
        for (int j=0;j<w2;j++) {
            x2 = ((j*x_ratio)>>16) ;
            y2 = ((i*y_ratio)>>16) ;
            uint8_t b2 = src[((y2+src_y_offset)*NES_WIDTH)+x2+src_x_offset];
            framebuffer[(i*w2)+j] = palette565[b2];
        }
    }
}

__attribute__((optimize("unroll-loops")))
static inline void blit_nearest(uint8_t *src, uint16_t *framebuffer)
{
    int w1 = NES_WIDTH - (crop_overscan_h ? 16 : 0);
    int w2 = GW_LCD_WIDTH;
    int h2 = GW_LCD_HEIGHT - (crop_overscan_v ? 16 : 0);
    int src_x_offset = (crop_overscan_h ? 8 : 0);
    int dst_x_offset = (crop_overscan_h ? 10 : 0);
    uint8_t y_offset = (crop_overscan_v ? 8 : 0);
    // duplicate one column every 3 lines -> x1.25
    int scale_ctr = 3;

    for (int y = y_offset; y < h2; y++) {
        int ctr = 0;
        uint8_t  *src_row  = &src[y*NES_WIDTH+src_x_offset];
        uint16_t *dest_row = &framebuffer[y * w2 + dst_x_offset];
        int x2 = 0;
        for (int x = 0; x < w1; x++) {
            uint16_t b2 = palette565[src_row[x]];
            dest_row[x2++] = b2;
            if (ctr++ == scale_ctr) {
                ctr = 0;
                dest_row[x2++] = b2;
            }
        }
    }
}

#define CONV(_b0) ((0b11111000000000000000000000&_b0)>>10) | ((0b000001111110000000000&_b0)>>5) | ((0b0000000000011111&_b0));

__attribute__((optimize("unroll-loops")))
static void blit_4to5(uint8_t *src, uint16_t *framebuffer) {
    int w1 = NES_WIDTH - (crop_overscan_h ? 16 : 0);
    int w2 = GW_LCD_WIDTH;
    int h2 = GW_LCD_HEIGHT - (crop_overscan_v ? 16 : 0);

    int src_x_offset = (crop_overscan_h ? 8 : 0);
    int dst_x_offset = (crop_overscan_h ? 10 : 0);
    uint8_t y_offset = (crop_overscan_v ? 8 : 0);

    for (int y = y_offset; y < h2; y++) {
        uint8_t  *src_row  = &src[y*NES_WIDTH+src_x_offset];
        uint16_t *dest_row = &framebuffer[y * w2 + dst_x_offset];
        for (int x_src = 0, x_dst=0; x_src < w1; x_src+=4, x_dst+=5) {
            uint32_t b0 = palette_spaced_565[src_row[x_src]];
            uint32_t b1 = palette_spaced_565[src_row[x_src+1]];
            uint32_t b2 = palette_spaced_565[src_row[x_src+2]];
            uint32_t b3 = palette_spaced_565[src_row[x_src+3]];

            dest_row[x_dst]   = CONV(b0);
            dest_row[x_dst+1] = CONV((b0+b0+b0+b1)>>2);
            dest_row[x_dst+2] = CONV((b1+b2)>>1);
            dest_row[x_dst+3] = CONV((b2+b2+b2+b3)>>2);
            dest_row[x_dst+4] = CONV(b3);
        }
    }
}


__attribute__((optimize("unroll-loops")))
static void blit_5to6(uint8_t *src, uint16_t *framebuffer) {
    int w1_adjusted = NES_WIDTH - 4 - (crop_overscan_h ? 16 : 0);
    int w2 = WIDTH;
    int h2 = GW_LCD_HEIGHT - (crop_overscan_v ? 16 : 0);
    int dst_x_offset = (WIDTH - 307) / 2 + (crop_overscan_h ? 9 : 0);

    int src_x_offset = (crop_overscan_h ? 8 : 0);
    uint8_t y_offset = (crop_overscan_v ? 8 : 0);

    // x 1.2
    for (int y = y_offset; y < h2; y++) {
        uint8_t  *src_row  = &src[y*NES_WIDTH+src_x_offset];
        uint16_t *dest_row = &framebuffer[y * w2 + dst_x_offset];
        int x_src = 0;
        int x_dst = 0;
        for (; x_src < w1_adjusted; x_src+=5, x_dst+=6) {
            uint32_t b0 = palette_spaced_565[src_row[x_src]];
            uint32_t b1 = palette_spaced_565[src_row[x_src+1]];
            uint32_t b2 = palette_spaced_565[src_row[x_src+2]];
            uint32_t b3 = palette_spaced_565[src_row[x_src+3]];
            uint32_t b4 = palette_spaced_565[src_row[x_src+4]];

            dest_row[x_dst]   = CONV(b0);
            dest_row[x_dst+1] = CONV((b0+b1+b1+b1)>>2);
            dest_row[x_dst+2] = CONV((b1+b2)>>1);
            dest_row[x_dst+3] = CONV((b2+b3)>>1);
            dest_row[x_dst+4] = CONV((b3+b3+b3+b4)>>2);
            dest_row[x_dst+5] = CONV(b4);
        }
        // Last column, x_src=255
        dest_row[x_dst] = palette565[src_row[x_src]];
    }
}

static void blit(uint8_t *src, uint16_t *framebuffer)
{
    odroid_display_scaling_t scaling = odroid_display_get_scaling_mode();
    odroid_display_filter_t filtering = odroid_display_get_filter_mode();

    switch (scaling) {
    case ODROID_DISPLAY_SCALING_OFF:
        // Full height, borders on the side
        blit_normal(src, framebuffer);
        break;
    case ODROID_DISPLAY_SCALING_FIT:
        // Full height and width, with cropping removal
        screen_blit_nn(src, framebuffer);
        break;
    case ODROID_DISPLAY_SCALING_FULL:
        // full height, full width
        if (filtering == ODROID_DISPLAY_FILTER_OFF) {
            blit_nearest(src, framebuffer);
        } else {
            blit_4to5(src, framebuffer);
        }
        break;
    case ODROID_DISPLAY_SCALING_CUSTOM:
        // full height, almost full width
        blit_5to6(src, framebuffer);
        break;
    default:
        printf("Unknown scaling mode %d\n", scaling);
        assert(!"Unknown scaling mode");
        break;
    }
    common_ingame_overlay();
}

static void update_sound_nes(int32_t *sound, uint16_t size) {
    if (common_emu_sound_loop_is_muted()) {
        return;
    }

    int32_t factor = common_emu_sound_get_volume();
    int16_t* sound_buffer = audio_get_active_buffer();
    uint16_t sound_buffer_length = audio_get_buffer_length();

    // Write to DMA buffer and lower the volume accordingly
    for (int i = 0; i < sound_buffer_length; i++) {
        int32_t sample = sound[i];
        sound_buffer[i] = ((sample * factor) >> 8) & 0xFFFF;
    }
}

static size_t nes_getromdata(unsigned char **data)
{
    /* src pointer to the ROM data in the external flash (raw or LZ4) */
    const unsigned char *src = ROM_DATA;
    unsigned char *dest = (unsigned char *)&_NES_FCEU_ROM_UNPACK_BUFFER;
    uint32_t available_size = (uint32_t)&_NES_FCEU_ROM_UNPACK_BUFFER_SIZE;

    wdog_refresh();
    if(strcmp(ROM_EXT, "lzma") == 0){
        size_t n_decomp_bytes;
        n_decomp_bytes = lzma_inflate(dest, available_size, src, ROM_DATA_LENGTH);
        *data = dest;
        ram_start = (uint32_t)dest + n_decomp_bytes;
        return n_decomp_bytes;
    }
    else
    {
#ifdef FCEU_LOW_RAM
        // FDS disks has to be stored in ram for games
        // that wants to write to the disk
        if (ROM_DATA_LENGTH <= 262000) {
            memcpy(dest, ROM_DATA, ROM_DATA_LENGTH);
            *data = (unsigned char *)dest;
            ram_start = (uint32_t)dest + ROM_DATA_LENGTH;
        } else 
#endif
        {
            *data = (unsigned char *)ROM_DATA;
            ram_start = (uint32_t)dest;
        }

        return ROM_DATA_LENGTH;
    }
}

static bool palette_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int max = sizeof(palettes) / sizeof(palettes[0]) - 1;

    if (event == ODROID_DIALOG_PREV) palette_index = palette_index > 0 ? palette_index - 1 : max;
    if (event == ODROID_DIALOG_NEXT) palette_index = palette_index < max ? palette_index + 1 : 0;

    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) {
        setCustomPalette(palette_index);
    }
    sprintf(option->value, "%10s", palettes[palette_index].name);
    return event == ODROID_DIALOG_ENTER;
}

static bool sprite_limit_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    if ((event == ODROID_DIALOG_NEXT) || (event == ODROID_DIALOG_PREV)) {
        disable_sprite_limit = !disable_sprite_limit;
    }
    sprintf(option->value,"%s",disable_sprite_limit?curr_lang->s_Yes:curr_lang->s_No);

    FCEUI_DisableSpriteLimitation(disable_sprite_limit);

    return event == ODROID_DIALOG_ENTER;
}

static void update_overclocking(uint8_t oc_profile)
{
    switch (oc_profile) {
        case 0: // No overclocking
            skip_7bit_overclocking = 1;
            extrascanlines         = 0;
            vblankscanlines        = 0;
            overclock_enabled      = 0;
            break;
        case 1: // 2x-Postrender
            skip_7bit_overclocking = 1;
            extrascanlines         = 266;
            vblankscanlines        = 0;
            overclock_enabled      = 1;
            break;
        case 2: // 2x-VBlank
            skip_7bit_overclocking = 1;
            extrascanlines         = 0;
            vblankscanlines        = 266;
            overclock_enabled      = 1;
            break;
    }
}

static bool overclocking_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    uint8_t max_index = 2;

    if (event == ODROID_DIALOG_NEXT) {
        overclocking_type = overclocking_type < max_index ? overclocking_type + 1 : 0;
    }
    if (event == ODROID_DIALOG_PREV) {
        overclocking_type = overclocking_type > 0 ? overclocking_type - 1 : max_index;
    }
    switch (overclocking_type) {
        case 0: // No overclocking
            skip_7bit_overclocking = 1;
            extrascanlines         = 0;
            vblankscanlines        = 0;
            overclock_enabled      = 0;
            sprintf(option->value,"%s",curr_lang->s_No);
            break;
        case 1: // 2x-Postrender
            skip_7bit_overclocking = 1;
            extrascanlines         = 266;
            vblankscanlines        = 0;
            overclock_enabled      = 1;
            sprintf(option->value,"%s","2x-Postrender");
            break;
        case 2: // 2x-VBlank
            skip_7bit_overclocking = 1;
            extrascanlines         = 0;
            vblankscanlines        = 266;
            overclock_enabled      = 1;
            sprintf(option->value,"%s","2x-VBlank");
            break;
    }
    return event == ODROID_DIALOG_ENTER;
}

static bool crop_overscan_v_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    if ((event == ODROID_DIALOG_NEXT) || (event == ODROID_DIALOG_PREV)) {
        crop_overscan_v = (crop_overscan_v+1)%2;
    }
    sprintf(option->value,"%s",crop_overscan_v?curr_lang->s_Yes:curr_lang->s_No);
    return event == ODROID_DIALOG_ENTER;
}

static bool crop_overscan_h_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    if ((event == ODROID_DIALOG_NEXT) || (event == ODROID_DIALOG_PREV)) {
        crop_overscan_h = (crop_overscan_h+1)%2;
    }
    sprintf(option->value,"%s",crop_overscan_h?curr_lang->s_Yes:curr_lang->s_No);
    return event == ODROID_DIALOG_ENTER;
}

static bool reset_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    if (event == ODROID_DIALOG_ENTER) {
        FCEUI_ResetNES();
    }
    return event == ODROID_DIALOG_ENTER;
}

static bool fds_side_swap_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    if (event == ODROID_DIALOG_NEXT) {
        FCEU_FDSSelect();          /* Swap FDisk side */
    }
    if (event == ODROID_DIALOG_PREV) {
        FCEU_FDSSelect_previous(); /* Swap FDisk side */
    }
    int8 diskinfo = FCEU_FDSCurrentSideDisk();
    sprintf(option->value,curr_lang->s_NES_FDS_Side_Format,1+((diskinfo&2)>>1),(diskinfo&1)?"B":"A");
    if (event == ODROID_DIALOG_ENTER) {
        allow_swap_disk = false;
        FCEU_FDSInsert(-1);        /* Insert the disk */
    }
    return event == ODROID_DIALOG_ENTER;
}

static bool fds_eject_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    bool inserted = FCEU_FDSIsDiskInserted();
    if (event == ODROID_DIALOG_ENTER) {
        if (inserted) {
            allow_swap_disk = true;
        } else {
            allow_swap_disk = false;
        }
        FCEU_FDSInsert(-1);        /* Insert or eject the disk */
    }
    sprintf(option->value,inserted?curr_lang->s_NES_Eject_FDS:curr_lang->s_NES_Insert_FDS);
    return event == ODROID_DIALOG_ENTER;
}

#if CHEAT_CODES == 1
static int checkGG(char c)
{
   static const char lets[16] = { 'A', 'P', 'Z', 'L', 'G', 'I', 'T', 'Y', 'E', 'O', 'X', 'U', 'K', 'S', 'V', 'N' };
   int x;

   for (x = 0; x < 16; x++)
      if (lets[x] == toupper(c))
         return 1;
   return 0;
}

static int GGisvalid(const char *code)
{
   size_t len = strlen(code);
   uint32 i;

   if (len != 6 && len != 8)
      return 0;

   for (i = 0; i < len; i++)
      if (!checkGG(code[i]))
         return 0;
   return 1;
}

void apply_cheat_code(const char *cheatcode) {
    uint16 a;
    uint8  v;
    int    c;
    int    type = 1;
    char temp[256];
    char *codepart;

    strcpy(temp, cheatcode);
    codepart = strtok(temp, "+,;._ ");

    while (codepart)
    {
        size_t codepart_len = strlen(codepart);
        if ((codepart_len == 7) && (codepart[4]==':'))
        {
            /* raw code in xxxx:xx format */
            printf("Cheat code added: '%s' (Raw)\n", codepart);
            codepart[4] = '\0';
            a = strtoul(codepart, NULL, 16);
            v = strtoul(codepart + 5, NULL, 16);
            c = -1;
            /* Zero-page addressing modes don't go through the normal read/write handlers in FCEU, so
            * we must do the old hacky method of RAM cheats. */
            if (a < 0x0100) type = 0;
            FCEUI_AddCheat(NULL, a, v, c, type);
        }
        else if ((codepart_len == 10) && (codepart[4] == '?') && (codepart[7] == ':'))
        {
            /* raw code in xxxx?xx:xx */
            printf("Cheat code added: '%s' (Raw)\n", codepart);
            codepart[4] = '\0';
            codepart[7] = '\0';
            a = strtoul(codepart, NULL, 16);
            v = strtoul(codepart + 8, NULL, 16);
            c = strtoul(codepart + 5, NULL, 16);
            /* Zero-page addressing modes don't go through the normal read/write handlers in FCEU, so
            * we must do the old hacky method of RAM cheats. */
            if (a < 0x0100) type = 0;
            FCEUI_AddCheat(NULL, a, v, c, type);
        }
        else if (GGisvalid(codepart) && FCEUI_DecodeGG(codepart, &a, &v, &c))
        {
            FCEUI_AddCheat(NULL, a, v, c, type);
            printf("Cheat code added: '%s' (GG)\n", codepart);
        }
        else if (FCEUI_DecodePAR(codepart, &a, &v, &c, &type))
        {
            FCEUI_AddCheat(NULL, a, v, c, type);
            printf("Cheat code added: '%s' (PAR)\n", codepart);
        }
        codepart = strtok(NULL,"+,;._ ");
    }
}
#endif

int app_main_nes_fceu(uint8_t load_state, uint8_t start_paused, uint8_t save_slot)
{
    uint8_t *rom_data;
    uint32_t rom_size;
    bool drawFrame;
    uint8_t *gfx;
    int32_t ssize = 0;

    uint32_t sndsamplerate = NES_FREQUENCY_48K;
    odroid_gamepad_state_t joystick;

    crop_overscan_v = false;
    crop_overscan_h = false;

    lcd_clear_buffers();

    if (start_paused) {
        common_emu_state.pause_after_frames = 2;
    } else {
        common_emu_state.pause_after_frames = 0;
    }

    XBuf = nes_framebuffer;

    FCEUI_Initialize();

    rom_size = nes_getromdata(&rom_data);
    FCEUGI *gameInfo = FCEUI_LoadGame(ACTIVE_FILE->name, rom_data, rom_size,
                                     NULL);

    PowerNES();

    FCEUI_SetInput(0, SI_GAMEPAD, &fceu_joystick, 0);

    // If mapper is 85 (with YM2413 FM sound), we have to use lower
    // sample rate as STM32H7 CPU can't handle FM sound emulation at 48kHz
    if ((gameInfo->type == GIT_CART) && (iNESCart.mapper == 85)) {
        sndsamplerate = NES_FREQUENCY_18K;
    }
    FCEUI_Sound(sndsamplerate);
    FCEUI_SetSoundVolume(150);

    odroid_system_init(APPID_NES, sndsamplerate);
    odroid_system_emu_init(&LoadState, &SaveState, NULL);

    if (FSettings.PAL) {
        lcd_set_refresh_rate(50);
        common_emu_state.frame_time_10us = (uint16_t)(100000 / 50 + 0.5f);
        samplesPerFrame = sndsamplerate / 50;
    } else {
        lcd_set_refresh_rate(60);
        common_emu_state.frame_time_10us = (uint16_t)(100000 / 60 + 0.5f);
        samplesPerFrame = sndsamplerate / 60;
    }

    // Init Sound
    audio_start_playing(samplesPerFrame);

    AddExState(&gnw_save_data, ~0, 0, 0);

    if (load_state) {
#if OFF_SAVESTATE==1
        if (save_slot == 1) {
            // Load from common save slot if needed
            memstream_set_buffer((uint8_t*)&__OFFSAVEFLASH_START__, ACTIVE_FILE->save_size);
            FCEUSS_Load_Mem();
        } else {
#endif
        LoadState("");
#if OFF_SAVESTATE==1
        }
#endif
        // Update local settings
        setCustomPalette(palette_index);
        update_overclocking(overclocking_type);
        FCEUI_DisableSpriteLimitation(disable_sprite_limit);

        bool inserted = FCEU_FDSIsDiskInserted();
        if (inserted) {
            allow_swap_disk = false;
        } else {
            allow_swap_disk = true;
        }
    }

#if CHEAT_CODES == 1
    for(int i=0; i<MAX_CHEAT_CODES && i<ACTIVE_FILE->cheat_count; i++) {
        if (odroid_settings_ActiveGameGenieCodes_is_enabled(ACTIVE_FILE->id, i)) {
            apply_cheat_code(ACTIVE_FILE->cheat_codes[i]);
        }
    }
#endif

    void _blit()
    {
        blit(nes_framebuffer, lcd_get_active_buffer());
    }

    while(1) {
        odroid_dialog_choice_t options[] = {
            // {101, "More...", "", 1, &advanced_settings_cb},
            {302, curr_lang->s_Palette, palette_values_text, 1, &palette_update_cb},
            {302, curr_lang->s_Reset, NULL, 1, &reset_cb},
            {302, curr_lang->s_Crop_Vertical_Overscan,crop_overscan_v_text,1,&crop_overscan_v_cb},
            {302, curr_lang->s_Crop_Horizontal_Overscan,crop_overscan_h_text,1,&crop_overscan_h_cb},
            {302, curr_lang->s_Disable_Sprite_Limit,sprite_limit_text,1,&sprite_limit_cb},
            {302, curr_lang->s_NES_CPU_OC,overclocking_text,1,&overclocking_cb},
            {302, curr_lang->s_NES_Eject_Insert_FDS,eject_insert_text,GameInfo->type == GIT_FDS ? 1 : -1,&fds_eject_cb},
            {302, curr_lang->s_NES_Swap_Side_FDS,next_disk_text,allow_swap_disk ? 1 : -1,&fds_side_swap_cb},
            ODROID_DIALOG_CHOICE_LAST
        };

        wdog_refresh();

        drawFrame = common_emu_frame_loop();

        odroid_input_read_gamepad(&joystick);
        common_emu_input_loop(&joystick, options, &_blit);
        common_emu_input_loop_handle_turbo(&joystick);

        nesInputUpdate(&joystick);

        FCEUI_Emulate(&gfx, &sound, &ssize, !drawFrame);

        if (drawFrame)
        {
            _blit();
            lcd_swap();
        }

        update_sound_nes(sound,ssize);

        common_emu_sound_sync(false);
    }

    return 0;
}

#endif