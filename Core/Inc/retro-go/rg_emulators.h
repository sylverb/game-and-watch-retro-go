#pragma once

#include <odroid_sdcard.h>
#include <stdint.h>
#include <stdbool.h>

#if !defined(COVERFLOW)
#define COVERFLOW 0
#endif /* COVERFLOW */
#if !defined (CHEAT_CODES)
#define CHEAT_CODES 0
#endif

typedef enum
{
    REGION_NTSC = 0,
    REGION_PAL
} rom_region_t;

typedef struct rom_system_t rom_system_t;

typedef struct {
#if CHEAT_CODES == 1
    uint32_t id;
#endif
    const char *name;
    const char *ext;
    // char folder[32];
    const uint8_t *address;
    uint32_t size;
	#if COVERFLOW != 0
    const uint8_t *img_address;
    //max 64kb image file data
    uint16_t img_size;
	#endif
    //size_t crc_offset;
    //uint32_t checksum;
    //bool missing_cover;
    const uint8_t *extra;
    rom_region_t region;
    const rom_system_t *system;
#if CHEAT_CODES == 1
    const char** cheat_codes; // Cheat codes to choose from
    const char** cheat_descs; // Cheat codes descriptions
    int cheat_count;
#endif
} retro_emulator_file_t;

typedef struct {
    char system_name[32];
    //char dirname[16];
    char ext[4];
    uint16_t crc_offset;
    uint16_t partition;
	#if COVERFLOW != 0
    size_t cover_width;
    size_t cover_height;
	#endif
    struct {
        const retro_emulator_file_t *files;
        int count;
    } roms;
    bool initialized;
    const rom_system_t *system;
} retro_emulator_t;


extern const unsigned int extflash_magic_sign;
extern const unsigned int intflash_magic_sign;

void emulators_init();
void emulator_init(retro_emulator_t *emu);
void emulator_start(retro_emulator_file_t *file, bool load_state, bool start_paused, int8_t save_slot);
bool emulator_show_file_menu(retro_emulator_file_t *file);
void emulator_show_file_info(retro_emulator_file_t *file);
void emulator_crc32_file(retro_emulator_file_t *file);
bool emulator_build_file_object(const char *path, retro_emulator_file_t *out_file);
const char *emu_get_file_path(retro_emulator_file_t *file);
retro_emulator_t *file_to_emu(retro_emulator_file_t *file);
bool emulator_is_file_valid(retro_emulator_file_t *file);
