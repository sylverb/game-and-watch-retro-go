#include "build/config.h"
#ifdef ENABLE_EMULATOR_AMSTRAD

#include <odroid_system.h>

#include "main.h"
#include "appid.h"

#include "common.h"
#include "gw_linker.h"
#include "gw_flash.h"
#include "gw_lcd.h"
#include "main_amstrad.h"
#include "save_amstrad.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "filesystem.h"

static char *headerString = "AMST0000";

#define WORK_BLOCK_SIZE (256)
static int flashBlockOffset = 0;
static bool isLastFlashWrite = 0;

extern int cap32_save_state(fs_file_t *file);
extern int cap32_load_state(fs_file_t *file);

// This function fills 4kB blocks and writes them in flash when full
static void SaveFlashSaveData(uint8_t *dest, uint8_t *src, int size) {
    int blockNumber = 0;
    for (int i = 0; i < size; i++) {
        // We use amstrad_framebuffer as a temporary buffer
        amstrad_framebuffer[flashBlockOffset] = src[i];
        flashBlockOffset++;
        if ((flashBlockOffset == WORK_BLOCK_SIZE) || (isLastFlashWrite && (i == size-1))) {
            // Write block in flash
            int intDest = (int)dest+blockNumber*WORK_BLOCK_SIZE;
            intDest = intDest & ~(WORK_BLOCK_SIZE-1);
            unsigned char *newDest = (unsigned char *)intDest;
            OSPI_DisableMemoryMappedMode();
            OSPI_Program((uint32_t)newDest,(const uint8_t *)amstrad_framebuffer,WORK_BLOCK_SIZE);
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
        OSPI_Program((uint32_t)newDest,(const uint8_t *)amstrad_framebuffer,WORK_BLOCK_SIZE);
        OSPI_EnableMemoryMappedMode();
        flashBlockOffset = 0;
        blockNumber++;
    }
}

static SaveState amstradSaveState;

static uint32_t tagFromName(const char* tagName)
{
    uint32_t tag = 0;
    uint32_t mod = 1;

    while (*tagName) {
        mod *= 19219;
        tag += mod * *tagName++;
    }

    return tag;
}

/* Savestate functions */
uint32_t saveAmstradState(const char *pathName) {
    fs_file_t *file;
    file = fs_open(pathName, FS_WRITE, FS_COMPRESS);
    fs_write(file, headerString, 8);
    cap32_save_state(file);
    save_amstrad_data(file);
    fs_close(file);
    return 0;
}

void amstradSaveStateSet(SaveState* state, const char* tagName, uint32_t value)
{
    SaveFlashSaveData(state->buffer+state->offset,(uint8_t *)&value,sizeof(uint32_t));
    state->offset+=sizeof(uint32_t);
}

void amstradSaveStateSetBuffer(SaveState* state, const char* tagName, void* buffer, uint32_t length)
{
    SaveFlashSaveData(state->buffer + state->offset, (uint8_t *)buffer, length);
    state->offset += length;
}

SaveState* amstradSaveStateOpenForWrite(const char* fileName)
{
    // Update section
    amstradSaveState.sections[amstradSaveState.section].tag = tagFromName(fileName);
    amstradSaveState.sections[amstradSaveState.section].offset = amstradSaveState.offset;
    amstradSaveState.section++;
    return &amstradSaveState;
}

/* Loadstate functions */
bool initLoadAmstradState(uint8_t *srcBuffer) {
    amstradSaveState.offset = 0;
    // Check for header
    if (memcmp(headerString,srcBuffer,8) == 0) {
        amstradSaveState.buffer = srcBuffer;
        // Copy sections header in structure
        memcpy(amstradSaveState.sections,amstradSaveState.buffer+8,sizeof(amstradSaveState.sections[0])*MAX_SECTIONS);
        return true;
    }
    return false;
}

uint32_t loadAmstradState(const char *pathName) {
    fs_file_t *file;
    file = fs_open(pathName, FS_READ, FS_COMPRESS);
    char readin_header[9] = {0};
    fs_read(file, readin_header, 8);

    // Check for header
    if (memcmp(headerString, readin_header, sizeof(readin_header)) == 0) { 
        cap32_load_state(file);
        load_amstrad_data(file);
    }
    fs_close(file);
    return 0;
}

SaveState* amstradSaveStateOpenForRead(const char* fileName)
{
    // find offset
    uint32_t tag = tagFromName(fileName);
    for (int i = 0; i<MAX_SECTIONS; i++) {
        if (amstradSaveState.sections[i].tag == tag) {
            // Found tag
            amstradSaveState.offset = amstradSaveState.sections[i].offset;
            return &amstradSaveState;
        }
    }

    return NULL;
}

uint32_t amstradSaveStateGet(SaveState* state, const char* tagName)
{
    uint32_t value;

    memcpy(&value, state->buffer + state->offset, sizeof(uint32_t));
    state->offset+=sizeof(uint32_t);
    return value;
}

void amstradSaveStateGetBuffer(SaveState* state, const char* tagName, void* buffer, uint32_t length)
{
    memcpy(buffer, state->buffer + state->offset, length);
    state->offset += length;
}

#endif
