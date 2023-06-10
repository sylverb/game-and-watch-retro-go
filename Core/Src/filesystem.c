#include "gw_flash.h"
#include "gw_linker.h"
#include <string.h>
#include "filesystem.h"

#define LFS_CACHE_SIZE 256
#define LFS_LOOKAHEAD_SIZE 16

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

void filesystem_init(void){
    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (lfs_mount(&lfs, &cfg)) {
        assert(lfs_format(&lfs, &cfg) == 0);
        assert(lfs_mount(&lfs, &cfg) == 0);
    }
}
