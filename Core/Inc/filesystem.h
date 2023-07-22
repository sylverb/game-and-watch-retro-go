#ifndef GW_FILESYSTEM_H__
#define GW_FILESYSTEM_H__

#include <stdint.h>
#include <stddef.h>
#include "lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

void filesystem_init(void);

lfs_file_t *filesystem_open(const char *path, bool use_compression);
int filesystem_write(lfs_file_t *file, unsigned char *data, size_t size);
int filesystem_read(lfs_file_t *file, unsigned char *buffer, size_t size);
int filesystem_seek(lfs_file_t *file, lfs_soff_t off, int whence);
void filesystem_close(lfs_file_t *file);

#ifdef __cplusplus
}
#endif

#endif
