#ifndef GW_FILESYSTEM_H__
#define GW_FILESYSTEM_H__

#include <stdint.h>
#include <stddef.h>
#include "lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read a region in a block.
 *
 * Negative error codes are propogated to the user.
 *
 * @return errorcode. 0 on success.
 */
int littlefs_api_read(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size);

/**
 * @brief Program a region in a block.
 *
 * The block must have previously been erased. 
 * Negative error codes are propogated to the user.
 * May return LFS_ERR_CORRUPT if the block should be considered bad.
 *
 * @return errorcode. 0 on success.
 */
int littlefs_api_prog(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size);

/**
 * @brief Erase a block.
 *
 * A block must be erased before being programmed.
 * The state of an erased block is undefined.
 * Negative error codes are propogated to the user.
 * May return LFS_ERR_CORRUPT if the block should be considered bad.
 * @return errorcode. 0 on success.
 */
int littlefs_api_erase(const struct lfs_config *c, lfs_block_t block);

/**
 * @brief Sync the state of the underlying block device.
 *
 * Negative error codes are propogated to the user.
 *
 * @return errorcode. 0 on success.
 */
int littlefs_api_sync(const struct lfs_config *c);

#ifdef __cplusplus
}
#endif

#endif
