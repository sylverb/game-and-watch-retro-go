#include "gw_flash.h"
#include "gw_linker.h"
#include "gw_lcd.h"
#include <string.h>
#include "filesystem.h"
#include "porting/lib/littlefs/lfs.h"
#include "rg_rtc.h"
#include "tamp/compressor.h"
#include "tamp/decompressor.h"

#define LFS_CACHE_SIZE 256
#define LFS_LOOKAHEAD_SIZE 16
#define LFS_NUM_ATTRS 1  // Number of atttached file attributes; currently just 1 for "time".

#ifndef LFS_NO_MALLOC
    #error "GW does not support malloc"
#endif


#define MAX_OPEN_FILES 1  // Cannot be >8
#if MAX_OPEN_FILES > 8 || MAX_OPEN_FILES < 1
    #error "MAX_OPEN_FILES must be in range [1, 8]"
#endif
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
static bool tamp_is_compressing;


/******************************
 * LittleFS Driver Definition *
 ******************************/
// Pointer to the data "on disk"
static uint8_t *fs_partition;

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
    //.block_size will be set later
    //.block_count will be set later
    .block_cycles = 500,
};

/**
 * @brief Recursively make all parent directories for a file.
 * @param[in] dir Path of directories to make up to. The last element
 * of the path is assumed to be the file and IS NOT created.
 *   e.g.
 *       "foo/bar/baz"
 *   will create directories "foo" and "bar"
 */
static void mkdirs(const char *path) {
    char partial_path[FS_MAX_PATH_SIZE];
    char *p = NULL;

    strlcpy(partial_path, path, sizeof(partial_path));
    for(p = partial_path + 1; *p; p++) {
        if(*p == '/') {
            *p = '\0';
            lfs_mkdir(&lfs, partial_path);
            *p = '/';
        }
    }
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
    printf("Reading boot count\n");
    file = fs_open(filename, FS_READ, FS_RAW);
    if(file){
        fs_read(file, (unsigned char *)&boot_count, sizeof(boot_count));
        fs_close(file);
    }

    boot_count += 1;  // update boot count

    // write back new boot count
    printf("Writing boot count\n");
    file = fs_open(filename, FS_WRITE, FS_RAW);
    assert(sizeof(boot_count) == fs_write(file, (unsigned char*)&boot_count, sizeof(boot_count)));
    fs_close(file);

    printf("boot_count: %ld\n", boot_count);
}

/**
 * Initialize and mount the filesystem. Format the filesystem if unmountable (and then reattempt mount).
 */
void fs_init(void){
    fs_partition = (unsigned char *)&__FILESYSTEM_START__;
    cfg.block_size = OSPI_GetSmallestEraseSize();
    cfg.block_count = (&__FILESYSTEM_END__ - &__FILESYSTEM_START__) / cfg.block_size;

    // reformat if we can't mount the fs
    // this should only happen on the first boot
    if (lfs_mount(&lfs, &cfg)) {
        printf("filesystem formatting...\n");
        assert(lfs_format(&lfs, &cfg) == 0);
        assert(lfs_mount(&lfs, &cfg) == 0);
    }
    printf("filesytem mounted.\n");
    printf("%ld/%ld blocks allocated (block_size=%ld)\n", lfs_fs_size(&lfs), cfg.block_count, cfg.block_size);

    boot_counter();  // TODO: remove when done developing; causes unnecessary writes.
}

/**
 * Only 1 tamp-compressed file can be open at a time.
 *
 * In write mode, directories are created as necessary.
 *
 * If:
 *   * write_mode==true: Opens the file for writing; creates file if it doesn't exist.
 *   * write_mode==false: Opens the file for reading; erroring (returning NULL) if it doesn't exist.
 */
fs_file_t *fs_open(const char *path, bool write_mode, bool use_compression){
    int res;
    int flags = write_mode ? LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC: LFS_O_RDONLY;
    
    fs_file_handle_t *fs_file_handle = acquire_file_handle(use_compression);
    tamp_is_compressing = write_mode;

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

    if(write_mode)
        mkdirs(path);

    fs_file_handle->config.buffer = fs_file_handle->buffer;
    fs_file_handle->config.attrs = fs_file_handle->file_attrs;
    fs_file_handle->config.attr_count = LFS_NUM_ATTRS;

    // Add time attribute; may be useful for deleting oldest savestates to make room for new ones.
    uint32_t current_time = GW_GetUnixTime();
    fs_file_handle->file_attrs[0].type = 't';  // 't' for "time"
    fs_file_handle->file_attrs[0].size = 4;
    fs_file_handle->file_attrs[0].buffer = &current_time;

    // TODO: add error handling; maybe delete oldest file(s) to make room
    res = lfs_file_opencfg(&lfs, &fs_file_handle->file, path, flags, &fs_file_handle->config);
    if(res != 0)
        goto error;

    return &fs_file_handle->file;

error:
    release_file_handle(&fs_file_handle->file);
    return NULL;
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
        }
        // Note: we really return the number of consumed bytes, not the number of
        // compressed-bytes.
        return output_written_size;
    }
    else{
        return lfs_file_write(&lfs, file, data, size);
    }
}

int fs_read(fs_file_t *file, unsigned char *buffer, size_t size){
    if(file_is_using_compression(file)){
        tamp_res res;
        int output_written_size = 0;
        size_t input_chunk_consumed_size = 0;
        size_t output_chunk_written_size;
        int read_size = 0;
        unsigned char input_byte;
        bool file_exhausted = false;

        while(true){
            res = tamp_decompressor_decompress(
                    &tamp_engine.d,
                    buffer,
                    size,
                    &output_chunk_written_size,
                    &input_byte,
                    read_size,
                    &input_chunk_consumed_size
                    );
            assert(res >= TAMP_OK);  // i.e. an "ok" result
            assert(input_chunk_consumed_size == read_size);
                                     
            output_written_size += output_chunk_written_size;
            buffer += output_chunk_written_size;
            size -= output_chunk_written_size;

            if(res == TAMP_OUTPUT_FULL)
                break;
            if(res == TAMP_INPUT_EXHAUSTED && file_exhausted)
                break;

            read_size = lfs_file_read(&lfs, file, &input_byte, 1);
            file_exhausted = read_size == 0;
        }
        return output_written_size;
    }
    else{
        return lfs_file_read(&lfs, file, buffer, size);
    }
}

void fs_close(lfs_file_t *file){
    if(file_is_using_compression(file) && tamp_is_compressing){
        // flush the compressor
        unsigned char output_buffer[32];
        size_t output_written_size;
        tamp_res res;
        res = tamp_compressor_flush(
                &tamp_engine.c,
                output_buffer,
                sizeof(output_buffer),
                &output_written_size,
                false
        );
        assert(res == TAMP_OK);
        assert(output_written_size == lfs_file_write(&lfs, file, output_buffer, output_written_size));
    }
    assert(lfs_file_close(&lfs, file) >= 0);
    release_file_handle(file);
}

int fs_seek(lfs_file_t *file, lfs_soff_t off, int whence){
    if(file_is_using_compression(file)){
        if(whence == LFS_SEEK_CUR){
            unsigned char byte_buffer;
            assert(off > 0);
            printf("Seek %d\n", off);
            for(int i=0; i < off; i++){
                fs_read(file, &byte_buffer, 1);
            }
            return 0;  // WARNING: this is not a compliant value!
        }
        else{
            assert(0); // not implemented
        }
    }
    return lfs_file_seek(&lfs, file, off, whence);
}

bool fs_exists(const char *path){
    struct lfs_info info;
    return lfs_stat(&lfs, path, &info) == LFS_ERR_OK;
}
