#ifndef GW_FILESYSTEM_H__
#define GW_FILESYSTEM_H__

#include <stdint.h>
#include <stddef.h>
#include "lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

void filesystem_init(void);

void filesystem_write(const char *path, unsigned char *data, size_t size);

#ifdef __cplusplus
}
#endif

#endif
