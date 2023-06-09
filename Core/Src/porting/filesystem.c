#include "gw_flash.h"
#include <string.h>
#include "filesystem.h"

int littlefs_api_read(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size) {
    unsigned char *address = ((unsigned char *)&__FILESYSTEM_START__) + (block * c->block_size) + off;
    memcpy(buffer, address, size);
    return 0;
}

int littlefs_api_prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size) {
    uint32_t address = (block * c->block_size) + off;
    OSPI_DisableMemoryMappedMode();
    OSPI_Program(address, buffer, size);
    OSPI_EnableMemoryMappedMode();
    return 0;
}

int littlefs_api_erase(const struct lfs_config *c, lfs_block_t block) {
    uint32_t address = block * c->block_size;

    OSPI_DisableMemoryMappedMode();
    OSPI_EraseSync(address, c->block_size);
    OSPI_EnableMemoryMappedMode();

    return 0;
}

int littlefs_api_sync(const struct lfs_config *c) {
    /* Unnecessary*/
    return 0;
}
