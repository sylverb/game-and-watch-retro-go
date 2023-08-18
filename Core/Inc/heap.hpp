#pragma once

extern "C" void *heap_alloc_mem(size_t s);
extern "C" size_t heap_free_mem();
extern "C" void heap_itc_alloc(bool itc);
extern "C" void cpp_heap_init(size_t bss_end);