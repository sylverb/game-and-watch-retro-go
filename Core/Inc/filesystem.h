#ifndef GW_FILESYSTEM_H__
#define GW_FILESYSTEM_H__

#include <stdint.h>
#include <stddef.h>
#include "lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define fs_WRITE true
#define fs_READ false

#define fs_COMPRESS true
#define fs_RAW false

typedef lfs_file_t fs_file_t;
void fs_init(void);

fs_file_t *fs_open(const char *path, bool write_mode, bool use_compression);
int fs_write(fs_file_t *file, unsigned char *data, size_t size);
int fs_read(fs_file_t *file, unsigned char *buffer, size_t size);
int fs_seek(fs_file_t *file, lfs_soff_t off, int whence);
void fs_close(fs_file_t *file);

#ifdef __cplusplus
}
#endif

#endif
