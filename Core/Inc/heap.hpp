#pragma once

#ifdef __cplusplus
extern "C" {
#endif
void *heap_alloc_mem(size_t s);
size_t heap_free_mem();
void heap_itc_alloc(bool itc);
void cpp_heap_init(size_t bss_end);
#ifdef __cplusplus
}
#endif