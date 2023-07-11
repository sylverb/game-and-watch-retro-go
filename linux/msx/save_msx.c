/*****************************************************************************
** $Source: /cvsroot/bluemsx/blueMSX/Src/Utils/SaveState.c,v $
**
** $Revision: 1.5 $
**
** $Date: 2008/06/25 22:26:17 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************
*/
#include <odroid_system.h>

#include "main.h"

#include "save_msx.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Board.h"

//#define SAVE_DEBUG

struct SaveStateSection {
    UInt32 tag;
    UInt32 offset;
};

#define MAX_SECTIONS 32

struct SaveState {
    struct SaveStateSection sections[MAX_SECTIONS];
    UInt16 section;
    UInt32 allocSize;
    UInt32 size;
    UInt32 offset;
    UInt8 *buffer;
    char   fileName[64];
};

extern BoardInfo boardInfo;

static SaveState msxSaveState;

static UInt32 tagFromName(const char* tagName)
{
    UInt32 tag = 0;
    UInt32 mod = 1;

    while (*tagName) {
        mod *= 19219;
        tag += mod * *tagName++;
    }

    return tag;
}

/* Savestate functions */
UInt32 saveMsxState(UInt8 *destBuffer, UInt32 save_size) {
    msxSaveState.buffer = (UInt8 *)destBuffer;
    msxSaveState.offset = sizeof(msxSaveState.sections);
    msxSaveState.section = 0;
    boardSaveState("mem0",0);
    memcpy(msxSaveState.buffer,msxSaveState.sections,sizeof(msxSaveState.sections));

    return msxSaveState.offset;
}

void saveStateCreateForWrite(const char* fileName)
{
#ifdef SAVE_DEBUG
    printf("saveStateCreateForWrite %s\n",fileName);
#endif
    // Nothing to do, no filename on the G&W
}

void saveStateSet(SaveState* state, const char* tagName, UInt32 value)
{
#ifdef SAVE_DEBUG
    UInt32 tag = tagFromName(tagName);
    UInt32 size = sizeof(UInt32);
    printf("Saving state %s = 0x%x\n",tagName,value);
    memcpy(state->buffer + state->offset, &tag, sizeof(UInt32));
    state->offset+=sizeof(UInt32);
    memcpy(state->buffer + state->offset, &size, sizeof(UInt32));
    state->offset+=sizeof(UInt32);
#endif
    memcpy(state->buffer + state->offset, &value, sizeof(UInt32));
    state->offset+=sizeof(UInt32);
}

void saveStateSetBuffer(SaveState* state, const char* tagName, void* buffer, UInt32 length)
{
#ifdef SAVE_DEBUG
    UInt32 tag = tagFromName(tagName);
    printf("Saving buffer %s length %d (0x%x)\n",tagName,length,length);
    memcpy(state->buffer + state->offset, &tag, sizeof(UInt32));
    state->offset+=sizeof(UInt32);
    memcpy(state->buffer + state->offset, &length, sizeof(UInt32));
    state->offset+=sizeof(UInt32);
#endif
    memcpy(state->buffer + state->offset, buffer, length);
    state->offset += length;
}

SaveState* saveStateOpenForWrite(const char* fileName)
{
#ifdef SAVE_DEBUG
    printf("saveStateOpenForWrite %s tag %x offset %x\n",fileName,tagFromName(fileName),msxSaveState.offset);
#endif
    // Update section
    msxSaveState.sections[msxSaveState.section].tag = tagFromName(fileName);
    msxSaveState.sections[msxSaveState.section].offset = msxSaveState.offset;
    msxSaveState.section++;
    return &msxSaveState;
}

void saveStateDestroy(void)
{
#ifdef SAVE_DEBUG
    printf("saveStateDestroy\n");
#endif
}

void saveStateClose(SaveState* state)
{
#ifdef SAVE_DEBUG
    printf("saveStateClose\n");
#endif
}

/* Loadstate functions */
void loadMsxState(UInt8 *srcBuffer, UInt32 save_size) {
    msxSaveState.buffer = (UInt8 *)srcBuffer;
    memcpy(msxSaveState.sections,msxSaveState.buffer,sizeof(msxSaveState.sections));
    boardInfo.loadState();
}

SaveState* saveStateOpenForRead(const char* fileName)
{
#ifdef SAVE_DEBUG
    printf("saveStateOpenForRead %s\n",fileName);
#endif
    // find offset
    UInt32 tag = tagFromName(fileName);
    for (int i = 0; i<MAX_SECTIONS; i++) {
#ifdef SAVE_DEBUG
    printf("tag %d tag[%d] %d\n",tag,i,msxSaveState.sections[i].tag);
#endif
        if (msxSaveState.sections[i].tag == tag) {
            // Found tag
#ifdef SAVE_DEBUG
    printf("saveStateOpenForRead %s Tag found %d\n",fileName,tag);
#endif
            msxSaveState.offset = msxSaveState.sections[i].offset;
            return &msxSaveState;
        }
    }
#ifdef SAVE_DEBUG
    printf("=======saveStateOpenForRead %s Tag not found\n",fileName);
#endif

    return NULL;
}

UInt32 saveStateGet(SaveState* state, const char* tagName, UInt32 defValue)
{
#ifdef SAVE_DEBUG
    UInt32 tag = tagFromName(tagName);
    UInt32 bufferTag;
    UInt32 length;
    UInt32 value;
    memcpy(&bufferTag, state->buffer + state->offset, sizeof(UInt32));
    state->offset+=sizeof(UInt32);
    memcpy(&length, state->buffer + state->offset, sizeof(UInt32));
    state->offset+=sizeof(UInt32);
    memcpy(&value, state->buffer + state->offset, sizeof(UInt32));
    state->offset+=sizeof(UInt32);

    printf("saveStateGet %s value = %x\n",tagName,value);
    if (tag != bufferTag) {
        printf("------------ error tag --------------");
    }
    if (0x04 != length) {
        printf("------------ error length --------------");
    }
    return value;
#else
    UInt32 value;
    memcpy(&value, state->buffer + state->offset, sizeof(UInt32));
    state->offset+=sizeof(UInt32);

    return value;
#endif
}

void saveStateGetBuffer(SaveState* state, const char* tagName, void* buffer, UInt32 length)
{
#ifdef SAVE_DEBUG
    UInt32 tag = tagFromName(tagName);
    UInt32 bufferTag;
    UInt32 bufferLength;
    memcpy(&bufferTag, state->buffer + state->offset, sizeof(UInt32));
    state->offset+=sizeof(UInt32);
    memcpy(&bufferLength, state->buffer + state->offset, sizeof(UInt32));
    state->offset+=sizeof(UInt32);
    printf("saveStateGetBuffer %s length = 0x%x\n",tagName,length);
    if (tag != bufferTag) {
        printf("------------ error tag --------------");
    }
    if (length != bufferLength) {
        printf("------------ error length --------------");
    }
#endif

    memcpy(buffer, state->buffer + state->offset, length);
    state->offset += length;
}

void saveStateCreateForRead(const char* fileName)
{
#ifdef SAVE_DEBUG
    printf("saveStateCreateForRead %s\n",fileName);
#endif
}
