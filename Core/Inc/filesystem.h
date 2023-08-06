#ifndef GW_FILESYSTEM_H__
#define GW_FILESYSTEM_H__

#include <stdint.h>
#include <stddef.h>
#include "lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FS_MAX_PATH_SIZE (192)  // including null-terminator

#define FS_WRITE true
#define FS_READ false

#define FS_COMPRESS true
#define FS_RAW false

typedef lfs_file_t fs_file_t;
void fs_init(void);

fs_file_t *fs_open(const char *path, bool write_mode, bool use_compression);
int fs_write(fs_file_t *file, unsigned char *data, size_t size);
int fs_delete(const char *path);
int fs_read(fs_file_t *file, unsigned char *buffer, size_t size);
int fs_seek(fs_file_t *file, lfs_soff_t off, int whence);
void fs_close(fs_file_t *file);
bool fs_exists(const char *path);
bool fs_info(const char *path, struct lfs_info *info);
bool fs_dir_open(const char *path, lfs_dir_t *dir);
bool fs_dir_read(lfs_dir_t *dir, struct lfs_info *info);
bool fs_dir_close(lfs_dir_t *dir);
uint32_t fs_free_blocks();

#ifdef __cplusplus
}
#endif

#endif
