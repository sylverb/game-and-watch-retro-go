#ifndef GW_FILESYSTEM_H__
#define GW_FILESYSTEM_H__

#include <stdint.h>
#include <stddef.h>
#include "lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef lfs_file_t filesystem_file_t;
void filesystem_init(void);

filesystem_file_t *filesystem_open(const char *path, bool use_compression);
int filesystem_write(filesystem_file_t *file, unsigned char *data, size_t size);
int filesystem_read(filesystem_file_t *file, unsigned char *buffer, size_t size);
int filesystem_seek(filesystem_file_t *file, lfs_soff_t off, int whence);
void filesystem_close(filesystem_file_t *file);

#ifdef __cplusplus
}
#endif

#endif
