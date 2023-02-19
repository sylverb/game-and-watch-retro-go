#ifndef _GW_MALLOC_H_
#define _GW_MALLOC_H_

#include <stdint.h>
#include <stddef.h>
void ahb_init();
void *ahb_malloc(size_t size);
void *ahb_calloc(size_t count,size_t size);

void itc_init();
void *itc_malloc(size_t size);
void *itc_calloc(size_t count,size_t size);

#endif
