#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "main.h"
#include "gw_malloc.h"

static uint32_t current_ahb_pointer;
extern uint32_t __ahbram_start__;
extern uint32_t __ahbram_end__;
extern uint16_t __AHBRAM_LENGTH__;

static uint32_t current_itc_pointer;
extern uint32_t __itcram_start__;
extern uint32_t __itcram_end__;
extern uint16_t __ITCMRAM_LENGTH__;
extern uint16_t __NULLPTR_LENGTH__;

/* Ram allocation here is simple and does not support free or reallocation  */
/* What is possible is to reinitialize all allocated buffers by calling the */
/* AHB/ITC init function.                                                   */

/* AHB RAM is 128kB, but it's not cached (as it's used for audio DMA it can't)
   so access to data in AHB RAM is slower than in other RAMs, only use this RAM
   for not time critical ressources */
void ahb_init() {
  current_ahb_pointer = (uint32_t)(&__ahbram_end__);
}

void *ahb_malloc(size_t size) {
  void *pointer = (void *)current_ahb_pointer;
//  printf("hab_malloc 0x%lx size %d\n",current_ahb_pointer,size);
  current_ahb_pointer += size;
  assert((current_ahb_pointer) <= ((((uint32_t)&__ahbram_start__) + ((uint32_t)(&__AHBRAM_LENGTH__)))));
  return pointer;
}

void *ahb_calloc(size_t count,size_t size) {
  void *pointer = ahb_malloc(count*size);
  memset(pointer,0,count*size);
  return pointer;
}

/* AHB RAM is 64kB, it's fast RAM and can be used for any purpose */

void itc_init() {
  current_itc_pointer = (uint32_t)(&__itcram_end__);
}

void *itc_malloc(size_t size) {
  void *pointer = (void *)current_itc_pointer;
//  printf("itc_malloc 0x%lx size %d\n",current_itc_pointer,size);
  current_itc_pointer += size;
  assert((current_itc_pointer) <= ((((uint32_t)&__itcram_start__) + ((uint32_t)(&__ITCMRAM_LENGTH__)) - ((uint32_t)(&__NULLPTR_LENGTH__)))));
  return pointer;
}

void *itc_calloc(size_t count,size_t size) {
  void *pointer = itc_malloc(count*size);
  memset(pointer,0,count*size);
  return pointer;
}
