#include "rg_emulators.h"

typedef struct {
    char name[64];
    char path[168];
    retro_emulator_file_t file;
} application_t;

void applications_init();
