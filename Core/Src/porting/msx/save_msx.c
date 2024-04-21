#include "build/config.h"
#ifdef ENABLE_EMULATOR_MSX

#include <odroid_system.h>

#include "main.h"
#include "appid.h"

#include "common.h"
#include "gw_linker.h"
#include "gw_flash.h"
#include "gw_lcd.h"
#include "main_msx.h"
#include "save_msx.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Board.h"

static char *headerString = "bMSX0000";

extern BoardInfo boardInfo;
static SaveState *msxSaveState = NULL;  // This is really a fs_file_t

/* Savestate functions */
UInt32 saveMsxState(char *pathName) {
    msxSaveState = fs_open(pathName, FS_WRITE, FS_COMPRESS);
    fs_write(msxSaveState, (unsigned char *)headerString, 8);
    boardSaveState("mem0",0);
    save_gnw_msx_data();
    fs_close(msxSaveState);
    return 0;
}

void saveStateCreateForWrite(const char* fileName)
{
    // Nothing to do, file is previously opened in saveMsxState()
}

void saveStateSet(SaveState* state, const char* tagName, UInt32 value)
{
    wdog_refresh();
    fs_write(state, (unsigned char *)&value, 4);
}

void saveStateSetBuffer(SaveState* state, const char* tagName, void* buffer, UInt32 length)
{
    wdog_refresh();
    fs_write(state, buffer, length);
}

SaveState* saveStateOpenForWrite(const char* fileName)
{
    // Nothing to do, all "files" are serialized in... serial
    return msxSaveState;
}

void saveStateDestroy(void)
{
    // Nothing To Do; saveMsxState will handle closing the file
}

void saveStateClose(SaveState* state)
{
    // Nothing to do, all "files" are serialized in... serial
}

/* Loadstate functions */
UInt32 loadMsxState(char *pathName) {
    char header[8];
    msxSaveState = fs_open(pathName, FS_READ, FS_COMPRESS);
    fs_read(msxSaveState, (unsigned char *)header, sizeof(header));
    if (memcmp(headerString, header, 8) == 0) {
        boardInfo.loadState();
        load_gnw_msx_data();
    }
    fs_close(msxSaveState);
    return 0;
}

SaveState* saveStateOpenForRead(const char* fileName)
{
    // Nothing to do, all "files" are serialized in... serial
    return msxSaveState;
}

UInt32 saveStateGet(SaveState* state, const char* tagName, UInt32 defValue)
{
    UInt32 value;
    wdog_refresh();
    fs_read(state, (unsigned char *)&value, 4);
    return value;
}

void saveStateGetBuffer(SaveState* state, const char* tagName, void* buffer, UInt32 length)
{
    wdog_refresh();
    fs_read(state, buffer, length);
}

void saveStateCreateForRead(const char* fileName)
{
    // Nothing to do, file is previously opened in loadMsxState()
}

#endif
