#include <string.h>
#include <stdint.h>

#include "rom_manager.h"
#include "rg_emulators.h"
#include "utils.h"

const unsigned char *ROM_DATA = NULL;
unsigned ROM_DATA_LENGTH;
const char *ROM_EXT = NULL;
retro_emulator_file_t *ACTIVE_FILE = NULL;

#include "gb_roms.c"
#include "nes_roms.c"
#include "nes_bios.c"
#include "sms_roms.c"
#include "gg_roms.c"
#include "col_roms.c"
#include "sg1000_roms.c"
#include "pce_roms.c"
#include "gw_roms.c"
#include "msx_roms.c"
#include "msx_bios.c"
#include "wsv_roms.c"
#include "md_roms.c"
#include "a7800_roms.c"
#include "amstrad_roms.c"
#include "zelda3_roms.c"
#include "smw_roms.c"

const rom_system_t *systems[] = {
    &nes_system,
    &nes_bios,
    &gb_system,
    &sms_system,
    &gg_system,
    &col_system,
    &sg1000_system,
    &pce_system,
    &gw_system,
    &msx_system,
    &msx_bios,
    &wsv_system,
    &md_system,
    &a7800_system,
    &amstrad_system,
    &zelda3_system,
    &smw_system,
};

const rom_manager_t rom_mgr = {
    .systems = systems,
    .systems_count = ARRAY_SIZE(systems),
};

const rom_system_t *rom_manager_system(const rom_manager_t *mgr, char *name) {
    for(int i=0; i < mgr->systems_count; i++) {
        if(strcmp(mgr->systems[i]->system_name, name) == 0) {
            return mgr->systems[i];
        }
    }
    return NULL;
}

int rom_get_ext_count(const rom_system_t *system, char *ext) {
    int count = 0;
    for(int i=0; i < system->roms_count; i++) {
        if(strcmp(system->roms[i].ext, ext) == 0) {
            count++;
        }
    }
    return count;
}

const retro_emulator_file_t *rom_get_ext_file_at_index(const rom_system_t *system, char *ext, int index) {
    int count = 0;
    for(int i=0; i < system->roms_count; i++) {
        if(strcmp(system->roms[i].ext, ext) == 0) {
            if (count == index) {
                return &system->roms[i];
            }
            count++;
        }
    }
    return NULL;
}

int rom_get_index_for_file_ext(const rom_system_t *system, retro_emulator_file_t *file) {
    int index = 0;
    for(int i=0; i < system->roms_count; i++) {
        if(strcmp(system->roms[i].ext, file->ext) == 0) {
            if (strcmp(system->roms[i].name, file->name) == 0) {
                return index;
            }
            index++;
        }
    }
    return 0;
}

void rom_manager_set_active_file(retro_emulator_file_t *file)
{
    ACTIVE_FILE = file;
    ROM_DATA = file->address;
    ROM_EXT = file->ext;
    ROM_DATA_LENGTH = file->size;
}

const retro_emulator_file_t *rom_manager_get_file(const rom_system_t *system, const char *name)
{
    for(int i=0; i < system->roms_count; i++) {
        if (strlen(name) == (strlen(system->roms[i].name) + strlen(system->roms[i].ext) + 1)) {
            if((strncmp(system->roms[i].name, name,strlen(system->roms[i].name)) == 0) &&
               (name[strlen(system->roms[i].name)] == '.') &&
               (strncmp(system->roms[i].ext, name+strlen(system->roms[i].name)+1,strlen(system->roms[i].ext)) == 0)) {
                return &system->roms[i];
            }
        }
    }
    return NULL;
}
