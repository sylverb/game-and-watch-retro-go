#include "gw_flash.h"
#include "gw_linker.h"
#include "gw_lcd.h"
#include <string.h>
#include "filesystem.h"
#include "rg_rtc.h"

#define LFS_CACHE_SIZE 256
#define LFS_LOOKAHEAD_SIZE 16
#define LFS_NUM_ATTRS 1

#ifndef LFS_NO_MALLOC
    #error "GW does not support malloc"
#endif

uint8_t filesystem_partition[1 << 20] __attribute__((section (".filesystem"))) __attribute__((aligned(4096)));

lfs_t lfs = {0};

static uint8_t read_buffer[LFS_CACHE_SIZE] = {0};
static uint8_t prog_buffer[LFS_CACHE_SIZE] = {0};
static uint8_t lookahead_buffer[LFS_LOOKAHEAD_SIZE] = {0};


static int littlefs_api_read(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size) {
    unsigned char *address = filesystem_partition + (block * c->block_size) + off;
    memcpy(buffer, address, size);
    return 0;
}

static int littlefs_api_prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    uint32_t address = (&__FILESYSTEM_START__ - &__EXTFLASH_BASE__) + (block * c->block_size) + off;
    OSPI_DisableMemoryMappedMode();
    OSPI_Program(address, buffer, size);
    OSPI_EnableMemoryMappedMode();
    return 0;
}

static int littlefs_api_erase(const struct lfs_config *c, lfs_block_t block) {
    uint32_t address = (&__FILESYSTEM_START__ - &__EXTFLASH_BASE__) + (block * c->block_size);

    OSPI_DisableMemoryMappedMode();
    OSPI_EraseSync(address, c->block_size);
    OSPI_EnableMemoryMappedMode();

    return 0;
}

static int littlefs_api_sync(const struct lfs_config *c) {
    /* Unnecessary*/
    return 0;
}

static const struct lfs_config cfg = {
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
    //.block_count = __FILESYSTEM_LENGTH__ >> 12,  // divide by block size
    .block_count = 256,  // divide by block size; TODO how can we make this depend on __FILESYSTEM_LENGTH__
    .block_cycles = 500,
};

/**
 * Demo function to demonstrate the filesystem working
 */
static void boot_counter(){
    lfs_file_t file;
    uint8_t buffer[LFS_CACHE_SIZE] = {0};
    int flags = LFS_O_RDWR | LFS_O_CREAT;
    struct lfs_file_config file_cfg = {
        .buffer = buffer,
        .attrs=NULL,
        .attr_count=0,
    };

    assert(0 == lfs_file_opencfg(&lfs, &file, "boot_counter", flags, &file_cfg));

    // read current count
    uint32_t boot_count = 0;
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    assert(0 == lfs_file_rewind(&lfs, &file));
    assert(sizeof(boot_count) == lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count)));

    lfs_file_close(&lfs, &file);

    printf("boot_count: %ld\n", boot_count);
}

void filesystem_init(void){
    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (lfs_mount(&lfs, &cfg)) {
        printf("Formatting filesystem...\n");
        assert(lfs_format(&lfs, &cfg) == 0);
        assert(lfs_mount(&lfs, &cfg) == 0);
    }

    boot_counter();
    boot_counter();
}

void filesystem_write(const char *path, unsigned char *data, size_t size){
    // Lets just use the inactive frame buffer (153600 bytes)
    // Offset deep into frame buffer so we can save most states to mem
    // prior to flushing to disk.
    uint8_t *buffer = (uint8_t *)lcd_get_inactive_buffer() + (144 * 1024);

    lfs_file_t file;
    int flags = LFS_O_WRONLY | LFS_O_CREAT; // Write-only, create if it doesn't exist
    struct lfs_attr file_attrs[LFS_NUM_ATTRS] = {0};
    struct lfs_file_config file_cfg = {
        .buffer = buffer,
        .attrs=file_attrs,
        .attr_count=LFS_NUM_ATTRS
    };
    buffer += LFS_CACHE_SIZE;

    // Add time attribute
    uint32_t current_time = GW_GetUnixTime();
    assert(current_time);
    file_attrs[0].type = 't';  // 't' for "time"
    file_attrs[0].size = 4;
    file_attrs[0].buffer = &current_time;

    // TODO: add error handling; maybe delete oldest file(s) to make room
    assert(0 == lfs_file_opencfg(&lfs, &file, path, flags, &file_cfg));

    // TODO: error handling
    assert(size == lfs_file_write(&lfs, &file, data, size));

    assert(lfs_file_close(&lfs, &file));
}
