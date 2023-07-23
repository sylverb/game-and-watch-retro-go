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


/**
 *
 */
#define MAX_OPEN_FILES 2  // Cannot be >8
typedef struct{
    lfs_file_t file;
    uint8_t buffer[LFS_CACHE_SIZE];
    struct lfs_attr file_attrs[LFS_NUM_ATTRS];
    struct lfs_file_config config;
} fs_file_handle_t;

static fs_file_handle_t file_handles[MAX_OPEN_FILES];
static uint8_t file_handles_used_bitmask = 0;
static int8_t file_index_using_compression = -1;  //negative value indicates that compressor/decompressor is available.

/********************************************
 * Tamp Compressor/Decompressor definitions *
 ********************************************/

#define TAMP_WINDOW_BUFFER_BITS 10  // 1KB
// TODO: if we want to save RAM, we could reuse the inactive lcd frame buffer.
static unsigned char tamp_window_buffer[1 << TAMP_WINDOW_BUFFER_BITS];
typedef union{
    TampDecompressor d;
    TampCompressor c;
} tamp_compressor_or_decompressor_t;
static tamp_compressor_or_decompressor_t tamp_engine;


/******************************
 * LittleFS Driver Definition *
 ******************************/
// Pointer to the data "on disk"
static uint8_t fs_partition[1 << 20] __attribute__((section(".filesystem"))) __attribute__((aligned(4096)));

lfs_t lfs = {0};

static uint8_t read_buffer[LFS_CACHE_SIZE] = {0};
static uint8_t prog_buffer[LFS_CACHE_SIZE] = {0};
static uint8_t lookahead_buffer[LFS_LOOKAHEAD_SIZE] __attribute__((aligned(4))) = {0};


static int littlefs_api_read(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size) {
    unsigned char *address = fs_partition + (block * c->block_size) + off;
    memcpy(buffer, address, size);
    return 0;
}

static int littlefs_api_prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    uint32_t address = (fs_partition - &__EXTFLASH_BASE__) + (block * c->block_size) + off;
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
    uint32_t address = (fs_partition - &__EXTFLASH_BASE__) + (block * c->block_size);

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
    lfs_file_t *file;
    uint32_t boot_count = 0;
    const char filename[] = "boot_counter";

    // read current count
    file = fs_open(filename, fs_READ, fs_RAW);
    fs_read(file, (unsigned char *)&boot_count, sizeof(boot_count));
    fs_close(file);

    boot_count += 1;  // update boot count

    // write back new boot count
    file = fs_open(filename, fs_WRITE, fs_RAW);
    assert(sizeof(boot_count) == fs_write(file, (unsigned char*)&boot_count, sizeof(boot_count)));
    fs_close(file);

    printf("boot_count: %ld\n", boot_count);
}

/**
 * Initialize and mount the filesystem. Format the filesystem if unmountable (and then reattempt mount).
 */
void fs_init(void){
    // reformat if we can't mount the fs
    // this should only happen on the first boot
    cfg.block_count = (&__FILESYSTEM_END__ - &__FILESYSTEM_START__) >> 12;  // divide by block size
    if (lfs_mount(&lfs, &cfg)) {
        printf("filesystem formatting...\n");
        assert(lfs_format(&lfs, &cfg) == 0);
        assert(lfs_mount(&lfs, &cfg) == 0);
    }
    printf("filesytem mounted.\n");

    boot_counter();  // TODO: remove when done developing; causes unnecessary writes.
}

static bool file_is_using_compression(fs_file_t *file){
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
static fs_file_handle_t *acquire_file_handle(bool use_compression){
    uint8_t test_bit = 0x01;

    for(uint8_t i=0; i < MAX_OPEN_FILES; i++){
        if(!(file_handles_used_bitmask & test_bit)){
            fs_file_handle_t *handle;
            // Set the bit, indicating this file_handle is in use.
            file_handles_used_bitmask |= test_bit;

            if(use_compression){
                // Check if the compressor/decompressor is available.
                if(file_index_using_compression >= 0)
                    return NULL;
                // Indicate that this file is using the compressor/decompressor.
                file_index_using_compression = i;
            }
            handle = &file_handles[i];
            memset(handle, 0, sizeof(fs_file_handle_t));
            return handle;
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
static void release_file_handle(fs_file_t *file){
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

/**
 * Only 1 tamp-compressed file can be open at a time.
 *
 * If:
 *   * write_mode==true: Opens the file for writing; creates file if it doesn't exist.
 *   * write_mode==false: Opens the file for reading; erroring (returning NULL) if it doesn't exist.
 */
fs_file_t *fs_open(const char *path, bool write_mode, bool use_compression){
    int flags = write_mode ? LFS_O_WRONLY | LFS_O_CREAT : LFS_O_RDONLY;
    
    fs_file_handle_t *fs_file_handle = acquire_file_handle(use_compression);

    if(!fs_file_handle){
        printf("Unable to allocate file handle.");
        return NULL;
    }

    if(use_compression){
        // TODO: initialize tamp; it's globally already been reserved.
        if(write_mode){
            TampConf conf = {.window=TAMP_WINDOW_BUFFER_BITS, .literal=8, .use_custom_dictionary=false};
            assert(TAMP_OK == tamp_compressor_init(&tamp_engine.c, &conf, tamp_window_buffer));
        }
        else{
            assert(TAMP_OK == tamp_decompressor_init(&tamp_engine.d, NULL, tamp_window_buffer));
        }
    }

    if(write_mode){
        // TODO: create directories if necessary
    }

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

int fs_write(fs_file_t *file, unsigned char *data, size_t size){
    // TODO: do we want to put delete-oldest-savestate-logic in here?
    
    if(file_is_using_compression(file)){
        int output_written_size = 0;
        unsigned char output_buffer[4];
        while(size){
            size_t consumed;

            // Sink Data into input buffer.
            tamp_compressor_sink(&tamp_engine.c, data, size, &consumed);
            data += consumed;
            output_written_size += consumed;
            size -= consumed;

            // Run the engine; remaining size indicates that the compressor input buffer is full
            if(TAMP_LIKELY(size)){
                size_t chunk_output_written_size;
                assert(TAMP_OK == tamp_compressor_compress_poll(
                        &tamp_engine.c,
                        output_buffer,
                        sizeof(output_buffer),
                        &chunk_output_written_size
                        ));
                // TODO: better error-handling if the disk is full.
                assert(chunk_output_written_size == lfs_file_write(&lfs, file, output_buffer, chunk_output_written_size));
            }
            // Note: we really return the number of consumed bytes, not the number of
            // compressed-bytes.
            return output_written_size;
        }
        assert(0 && "tamp compression not yet implemented");
    }
    else{
        return lfs_file_write(&lfs, file, data, size);
    }
}

int fs_read(fs_file_t *file, unsigned char *buffer, size_t size){
    if(file_is_using_compression(file)){
        assert(0 && "tamp compression not yet implemented");
    }
    return lfs_file_read(&lfs, file, buffer, size);
}

void fs_close(lfs_file_t *file){
    if(file_is_using_compression(file)){
        assert(0 && "tamp compression not yet implemented");
    }
    assert(lfs_file_close(&lfs, file) >= 0);
    release_file_handle(file);
}

int fs_seek(lfs_file_t *file, lfs_soff_t off, int whence){
    assert(file_is_using_compression(file) == false);  // Cannot seek with compression.
    return lfs_file_seek(&lfs, file, off, whence);
}
