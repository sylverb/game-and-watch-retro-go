#pragma once

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
    const uint8_t *save_address;
    uint32_t save_size;
    //size_t crc_offset;
    //uint32_t checksum;
    //bool missing_cover;
    uint16_t mapper;
    const rom_system_t *system;
    uint16_t game_config;
#if CHEAT_CODES == 1
    const char** cheat_codes; // Cheat codes to choose from
    const char** cheat_descs; // Cheat codes descriptions
    int cheat_count;
#endif
} retro_emulator_file_t;

struct rom_system_t {
    char *system_name;
    const retro_emulator_file_t *roms;
    char *extension;
	#if COVERFLOW != 0
    size_t cover_width;
    size_t cover_height;
	#endif    
    uint32_t roms_count;
};
typedef struct {
    const rom_system_t **systems;
    uint32_t systems_count;
} rom_manager_t;

extern const rom_manager_t rom_mgr;

const rom_system_t *rom_manager_system(const rom_manager_t *mgr, char *name);
const retro_emulator_file_t *rom_manager_get_file(const rom_system_t *system, const char *name);
