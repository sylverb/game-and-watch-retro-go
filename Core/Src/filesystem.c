#include "gw_flash.h"
#include "gw_linker.h"
#include "gw_lcd.h"
#include <string.h>
#include "filesystem.h"
#include "rg_rtc.h"
#include "tamp/compressor.h"
#include "tamp/decompressor.h"

#define LFS_CACHE_SIZE 256
#define LFS_LOOKAHEAD_SIZE 16
#define LFS_NUM_ATTRS 1  // Number of atttached file attributes; currently just 1 for "time".

#ifndef LFS_NO_MALLOC
    #error "GW does not support malloc"
#endif


#define MAX_OPEN_FILES 2  // Cannot be >8
typedef struct{
    lfs_file_t file;
    uint8_t buffer[LFS_CACHE_SIZE];
    struct lfs_attr file_attrs[LFS_NUM_ATTRS];
    struct lfs_file_config config;
} filesystem_file_handle_t;

static filesystem_file_handle_t file_handles[MAX_OPEN_FILES];
static uint8_t file_handles_used_bitmask = 0;
static int8_t file_index_using_compression = -1;  //negative value indicates that compressor/decompressor is available.
// TODO: we can add a single tamp structure here, if we want to allow a single compressor file


/******************************
 * LittleFS Driver Definition *
 ******************************/
// Pointer to the data "on disk"
uint8_t filesystem_partition[1 << 20] __attribute__((section(".filesystem"))) __attribute__((aligned(4096)));

lfs_t lfs = {0};

static uint8_t read_buffer[LFS_CACHE_SIZE] = {0};
static uint8_t prog_buffer[LFS_CACHE_SIZE] = {0};
static uint8_t lookahead_buffer[LFS_LOOKAHEAD_SIZE] __attribute__((aligned(4))) = {0};


static int littlefs_api_read(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size) {
    unsigned char *address = filesystem_partition + (block * c->block_size) + off;
    memcpy(buffer, address, size);
    return 0;
}

static int littlefs_api_prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    uint32_t address = (filesystem_partition - &__EXTFLASH_BASE__) + (block * c->block_size) + off;
    assert((address & 0xFF) == 0);

    SCB_DisableDCache();
    SCB_InvalidateDCache();

    OSPI_DisableMemoryMappedMode();
    OSPI_Program(address, buffer, size);
    OSPI_EnableMemoryMappedMode();

    SCB_EnableDCache();

    return 0;
}

static int littlefs_api_erase(const struct lfs_config *c, lfs_block_t block) {
    uint32_t address = (filesystem_partition - &__EXTFLASH_BASE__) + (block * c->block_size);

    assert((address & (4*1024 - 1)) == 0);

    SCB_DisableDCache();
    SCB_InvalidateDCache();

    OSPI_DisableMemoryMappedMode();
    OSPI_EraseSync(address, c->block_size);
    OSPI_EnableMemoryMappedMode();

    SCB_EnableDCache();

    return 0;
}

static int littlefs_api_sync(const struct lfs_config *c) {
    return 0;
}

static struct lfs_config cfg = {
    // block device operations
    .read  = littlefs_api_read,
    .prog  = littlefs_api_prog,
    .erase = littlefs_api_erase,
    .sync  = littlefs_api_sync,

    // statically allocated buffers
    .read_buffer = read_buffer,
    .prog_buffer = prog_buffer,
    .lookahead_buffer = lookahead_buffer,

    // block device configuration
    .cache_size = LFS_CACHE_SIZE,
    .read_size = LFS_CACHE_SIZE,
    .prog_size = LFS_CACHE_SIZE,
    .lookahead_size = LFS_LOOKAHEAD_SIZE,
    .block_size = 4096,
    //.block_count will be set later
    .block_cycles = 500,
};

/*************************
 * Filesystem Public API *
 *************************/

/**
 * Demo function to demonstrate the filesystem working.
 */
static void boot_counter(){
    lfs_file_t *file = filesystem_open("boot_counter", false);

    // read current count
    uint32_t boot_count = 0;
    filesystem_read(file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    assert(0 == filesystem_seek(file, 0, LFS_SEEK_SET));
    assert(sizeof(boot_count) == filesystem_write(file, &boot_count, sizeof(boot_count)));

    filesystem_close(file);

    printf("boot_count: %ld\n", boot_count);
}

void filesystem_init(void){
    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    cfg.block_count = (&__FILESYSTEM_END__ - &__FILESYSTEM_START__) >> 12;  // divide by block size
    if (lfs_mount(&lfs, &cfg)) {
        printf("Filesystem formatting...\n");
        assert(lfs_format(&lfs, &cfg) == 0);
        assert(lfs_mount(&lfs, &cfg) == 0);
    }
    printf("Filesystem mounted.\n");

    boot_counter();  // TODO: remove when done developing; causes unnecessary writes.
}

static bool file_is_using_compression(filesystem_file_t *file){
    for(uint8_t i=0; i < MAX_OPEN_FILES; i++){
        if(file == &(file_handles[i].file) && i == file_index_using_compression)
            return true;
    }
    return false;
}

/**
 * Get a file handle from the statically allocated file handles.
 * Not responsible for initializing the file handle.
 *
 * If we want to use dynamic allocation in the future, malloc inside this function.
 */
static filesystem_file_handle_t *acquire_file_handle(bool use_compression){
    uint8_t test_bit = 0x01;

    for(uint8_t i=0; i < MAX_OPEN_FILES; i++){
        if(!(file_handles_used_bitmask & test_bit)){
            // Set the bit, indicating this file_handle is in use.
            file_handles_used_bitmask |= test_bit;

            if(use_compression){
                // Check if the compressor/decompressor is available.
                assert(file_index_using_compression < 0);
                // Indicate that this file is using the compressor/decompressor.
                file_index_using_compression = i;
            }

            return &file_handles[i];
        }
        test_bit <<= 1;
    }

    return NULL;
}

/**
 * Release the file handle.
 * Not responsible for closing the file handle.
 *
 * If we want to use dynamic allocation in the future, free inside this function.
 */
static void release_file_handle(filesystem_file_t *file){
    uint8_t test_bit = 0x01;

    for(uint8_t i=0; i < MAX_OPEN_FILES; i++){
        if(file == &(file_handles[i].file)){
            // Clear the bit, indicating this file_handle is no longer in use.
            file_handles_used_bitmask &= ~test_bit;
            if(file_is_using_compression(file)){
                file_index_using_compression = -1;
            }
            return;
        }
    }
    assert(0);  // Should never reach here.
}

filesystem_file_t *filesystem_open(const char *path, bool use_compression){
    const int flags = LFS_O_RDWR | LFS_O_CREAT;
    
    filesystem_file_handle_t *fs_file_handle = acquire_file_handle(use_compression);

    // Not sure if clearing is necessary, can maybe remove
    memset(fs_file_handle, 0, sizeof(filesystem_file_handle_t));
    fs_file_handle->config.buffer = fs_file_handle->buffer;
    fs_file_handle->config.attrs = fs_file_handle->file_attrs;
    fs_file_handle->config.attr_count = LFS_NUM_ATTRS;

    // Add time attribute; may be useful for deleting oldest savestates to make room for new ones.
    uint32_t current_time = GW_GetUnixTime();
    assert(current_time);
    fs_file_handle->file_attrs[0].type = 't';  // 't' for "time"
    fs_file_handle->file_attrs[0].size = 4;
    fs_file_handle->file_attrs[0].buffer = &current_time;

    // TODO: add error handling; maybe delete oldest file(s) to make room
    assert(0 == lfs_file_opencfg(&lfs, &fs_file_handle->file, path, flags, &fs_file_handle->config));

    return &fs_file_handle->file;
}

int filesystem_write(filesystem_file_t *file, unsigned char *data, size_t size){
    if(file_is_using_compression(file)){
        assert(0 && "tamp compression not yet implemented");
    }
    return lfs_file_write(&lfs, file, data, size);
}

int filesystem_read(filesystem_file_t *file, unsigned char *buffer, size_t size){
    if(file_is_using_compression(file)){
        assert(0 && "tamp compression not yet implemented");
    }
    return lfs_file_read(&lfs, file, buffer, size);
}

void filesystem_close(lfs_file_t *file){
    if(file_is_using_compression(file)){
        assert(0 && "tamp compression not yet implemented");
    }
    assert(lfs_file_close(&lfs, file) >= 0);
    release_file_handle(file);
}

int filesystem_seek(lfs_file_t *file, lfs_soff_t off, int whence){
    assert(file_is_using_compression(file) == false);  // Cannot seek with compression.
    return lfs_file_seek(&lfs, file, off, whence);
}
