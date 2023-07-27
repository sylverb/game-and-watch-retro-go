#include "build/config.h"
#ifdef ENABLE_EMULATOR_MD

#include <odroid_system.h>

#include "main.h"
#include "appid.h"

#include "common.h"
#include "gw_linker.h"
#include "gw_flash.h"
#include "gw_lcd.h"
#include "main_gwenesis.h"
#include "gwenesis_savestate.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char *headerString = "Gene0000";
static char saveBuffer[256];

#define WORK_BLOCK_SIZE (256)
static int flashBlockOffset = 0;
static bool isLastFlashWrite = 0;

// This function fills 4kB blocks and writes them in flash when full
static void SaveGwenesisFlashSaveData(unsigned char *dest, unsigned char *src, int size) {
    int blockNumber = 0;
    for (int i = 0; i < size; i++) {
        saveBuffer[flashBlockOffset] = src[i];
        flashBlockOffset++;
        if ((flashBlockOffset == WORK_BLOCK_SIZE) || (isLastFlashWrite && (i == size-1))) {
            // Write block in flash
            int intDest = (int)dest+blockNumber*WORK_BLOCK_SIZE;
            intDest = intDest & ~(WORK_BLOCK_SIZE-1);
            unsigned char *newDest = (unsigned char *)intDest;
            OSPI_DisableMemoryMappedMode();
            OSPI_Program((int)newDest,(const unsigned char *)saveBuffer,WORK_BLOCK_SIZE);
            OSPI_EnableMemoryMappedMode();
            flashBlockOffset = 0;
            blockNumber++;
        }
    }
    if (size == 0) {
        // force writing last data block
        int intDest = (int)dest+blockNumber*WORK_BLOCK_SIZE;
        intDest = intDest & ~(WORK_BLOCK_SIZE-1);
        unsigned char *newDest = (unsigned char *)intDest;
        OSPI_DisableMemoryMappedMode();
        OSPI_Program((int)newDest,(const unsigned char *)saveBuffer,WORK_BLOCK_SIZE);
        OSPI_EnableMemoryMappedMode();
        flashBlockOffset = 0;
        blockNumber++;
    }
}

struct SaveStateSection {
    int tag;
    int offset;
};

// We have 31*8 Bytes available for sections info
// Do not increase this value without reserving
// another 256 bytes block for header
#define MAX_SECTIONS 31

struct SaveState {
    struct SaveStateSection sections[MAX_SECTIONS];
    uint16_t section;
    int allocSize;
    int size;
    int offset;
    unsigned char *buffer;
    unsigned char  fileName[64];
};

static SaveState gwenesisSaveState;

static int tagFromName(const char* tagName)
{
    int tag = 0;
    int mod = 1;

    while (*tagName) {
        mod *= 19219;
        tag += mod * *tagName++;
    }

    return tag;
}

/* Savestate functions */
int saveGwenesisState(fs_file_t *file) {
    fs_write(file, headerString, 8);
    gwenesis_save_state(file);
    gwenesis_save_local_data(file);
    return 0;
}

/* Loadstate functions */
bool initLoadGwenesisState(unsigned char *srcBuffer) {
    gwenesisSaveState.offset = 0;
    // Check for header
    if (memcmp(headerString,srcBuffer,8) == 0) {
        gwenesisSaveState.buffer = srcBuffer;
        // Copy sections header in structure
        memcpy(gwenesisSaveState.sections,gwenesisSaveState.buffer+8,sizeof(gwenesisSaveState.sections[0])*MAX_SECTIONS);
        return true;
    }
    return false;
}

int loadGwenesisState(fs_file_t *file) {
    char header[8];
    fs_read(file, header, sizeof(header));

    // Check for header
    if (memcmp(headerString, header, 8) == 0) {
        gwenesis_load_state(file);
        gwenesis_load_local_data(file);
    }
    return 1;
}
#endif
