#ifndef AMSTRAD_SAVE_STATE_H
#define AMSTRAD_SAVE_STATE_H

struct SaveStateSection {
    uint32_t tag;
    uint32_t offset;
};

// We have 31*8 Bytes available for sections info
// Do not increase this value without reserving
// another 256 bytes block for header
#define MAX_SECTIONS 31

typedef struct  {
    struct SaveStateSection sections[MAX_SECTIONS];
    uint16_t section;
    uint32_t allocSize;
    uint32_t size;
    uint32_t offset;
    uint8_t *buffer;
    char   fileName[64];
} SaveState;

bool initLoadAmstradState(uint8_t *srcBuffer);
uint32_t saveAmstradState(const char *pathName);
uint32_t loadAmstradState(const char *pathName);

SaveState* amstradSaveStateOpenForRead(const char* fileName);
SaveState* amstradSaveStateOpenForWrite(const char* fileName);

uint32_t amstradSaveStateGet(SaveState* state, const char* tagName);
void amstradSaveStateSet(SaveState* state, const char* tagName, uint32_t value);

void amstradSaveStateGetBuffer(SaveState* state, const char* tagName, void* buffer, uint32_t length);
void amstradSaveStateSetBuffer(SaveState* state, const char* tagName, void* buffer, uint32_t length);

#endif /* AMSTRAD_SAVE_STATE_H */

