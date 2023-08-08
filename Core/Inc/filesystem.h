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

#define FS_TYPE_REG LFS_TYPE_REG
#define FS_TYPE_DIR LFS_TYPE_DIR

typedef lfs_file_t fs_file_t;
typedef lfs_dir_t fs_dir_t;
typedef struct lfs_info fs_info_t;

void fs_init(void);

fs_file_t *fs_open(const char *path, bool write_mode, bool use_compression);
int fs_write(fs_file_t *file, unsigned char *data, size_t size);
int fs_delete(const char *path);
int fs_read(fs_file_t *file, unsigned char *buffer, size_t size);
int fs_seek(fs_file_t *file, lfs_soff_t off, int whence);
void fs_close(fs_file_t *file);
bool fs_exists(const char *path);
bool fs_dir_open(const char *path, fs_dir_t *dir);
bool fs_dir_read(fs_dir_t *dir, fs_info_t *info);
bool fs_dir_close(fs_dir_t *dir);
uint32_t fs_free_blocks();

#ifdef __cplusplus
}
#endif

#endif
