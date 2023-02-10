#include "build/config.h"

#if defined(ENABLE_EMULATOR_NES) && FORCE_NOFRENDO == 0
/* Copyright  (C) 2010-2020 The RetroArch team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (memory_stream.c).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <odroid_system.h>
#include "common.h"
#include "gw_flash.h"
#include "gw_linker.h"
#include <inttypes.h>

#include <nes_memory_stream.h>

/* TODO/FIXME - static globals */
static uint8_t* g_buffer      = NULL;
static uint64_t g_size         = 0;
static uint64_t last_file_size = 0;

struct memstream
{
   uint64_t size;
   uint64_t ptr;
   uint64_t max_ptr;
   uint8_t *buf;
   unsigned writing;
};

#define WORK_BLOCK_SIZE (256) // Shall be a multiple of 256 as we write blocks of 256 bytes in flash
static int flashBlockOffset = 0;
static uint8_t work_buffer[WORK_BLOCK_SIZE];

// This function fills WORK_BLOCK_SIZE bytes blocks and writes them in flash when full
static void SaveFlashSaveData(uint8_t *dest, uint8_t *src, int size) {
   // Convert mem mapped pointer to flash address
   uint32_t flash_address = dest - &__EXTFLASH_BASE__;

   int blockNumber = 0;
   for (int i = 0; i < size; i++) {
      // We use a temporary buffer
      work_buffer[flashBlockOffset] = src[i];
      flashBlockOffset++;
      if (flashBlockOffset == WORK_BLOCK_SIZE) {
         // Write block in flash
         int intDest = (int)flash_address+blockNumber*WORK_BLOCK_SIZE;
         intDest = intDest & ~(WORK_BLOCK_SIZE-1);
         unsigned char *newDest = (unsigned char *)intDest;
         OSPI_DisableMemoryMappedMode();
         OSPI_Program((uint32_t)newDest,(const uint8_t *)work_buffer,WORK_BLOCK_SIZE);
         OSPI_EnableMemoryMappedMode();
         flashBlockOffset = 0;
         blockNumber++;
         // We fill block with 0xff so unchanged bytes will not change existing bytes in flash.
         // It will allow to update some values later if needed
         memset(work_buffer,0xff,WORK_BLOCK_SIZE);
      }
   }
   if (size == 0) {
      // force writing data block
      int intDest = (int)flash_address+blockNumber*WORK_BLOCK_SIZE;
      intDest = intDest & ~(WORK_BLOCK_SIZE-1);
      unsigned char *newDest = (unsigned char *)intDest;
      OSPI_DisableMemoryMappedMode();
      OSPI_Program((uint32_t)newDest,(const uint8_t *)work_buffer,WORK_BLOCK_SIZE);
      OSPI_EnableMemoryMappedMode();
      flashBlockOffset = 0;
      blockNumber++;
      // We fill block with 0xff so unchanged bytes will not change existing bytes in flash.
      // It will allow to update some values later if needed
      memset(work_buffer,0xff,WORK_BLOCK_SIZE);
   }
}

void memstream_set_buffer(uint8_t *buffer, uint64_t size)
{
   g_buffer = buffer;
   g_size   = size;
}

uint64_t memstream_get_last_size(void)
{
   return last_file_size;
}

memstream_t *memstream_open(unsigned writing)
{
   memstream_t *stream;
   if (!g_buffer || !g_size)
      return NULL;

   stream = (memstream_t*)malloc(sizeof(*stream));

   if (!stream)
      return NULL;

   stream->buf       = g_buffer;
   stream->size      = g_size;
   stream->ptr       = 0;
   stream->max_ptr   = 0;
   stream->writing   = writing;

   g_buffer          = NULL;
   g_size            = 0;

   if (stream->writing) {
      // Erase flash memory
      store_erase((const uint8_t *)stream->buf, stream->size);
      memset(work_buffer,0xff,WORK_BLOCK_SIZE);
   }
   return stream;
}

void memstream_close(memstream_t *stream)
{
   if (!stream)
      return;

   if (stream->writing) {
      // Before closing, write remaining data if needed
      SaveFlashSaveData((uint8_t *)(stream->buf + stream->ptr), NULL, 0);
   }

   last_file_size = stream->writing ? stream->max_ptr : stream->size;
   free(stream);
}

uint64_t memstream_get_ptr(memstream_t *stream)
{
   return stream->ptr;
}

uint64_t memstream_read(memstream_t *stream, void *data, uint64_t bytes)
{
   uint64_t avail = 0;

   if (!stream)
      return 0;

   avail               = stream->size - stream->ptr;
   if (bytes > avail)
      bytes            = avail;

   memcpy(data, stream->buf + stream->ptr, (size_t)bytes);
   stream->ptr        += bytes;
   if (stream->ptr > stream->max_ptr)
      stream->max_ptr  = stream->ptr;
   return bytes;
}

uint64_t memstream_write(memstream_t *stream,
      const void *data, uint64_t bytes)
{
   uint64_t avail = 0;

   if (!stream)
      return 0;

   avail = stream->size - stream->ptr;
   if (bytes > avail)
      bytes = avail;

   SaveFlashSaveData(stream->buf + stream->ptr,(uint8_t *)data, (int)bytes);

   stream->ptr += bytes;
   if (stream->ptr > stream->max_ptr)
      stream->max_ptr = stream->ptr;
   return bytes;
}

int64_t memstream_seek(memstream_t *stream, int64_t offset, int whence)
{
   uint64_t ptr;

   switch (whence)
   {
      case SEEK_SET:
         ptr = offset;
         break;
      case SEEK_CUR:
         ptr = stream->ptr + offset;
         break;
      case SEEK_END:
         ptr = (stream->writing ? stream->max_ptr : stream->size) + offset;
         break;
      default:
         return -1;
   }

   if (ptr <= stream->size)
   {
      if (stream->writing) {
         uint8_t fill = 0xff;

         // Before pointing elsewhere, write remaining data if needed
         SaveFlashSaveData(stream->buf + stream->ptr, NULL, 0);

         // Create a block aligned to lower WORK_BLOCK_SIZE bytes and fill it with FF so
         // unchanged bytes will not change values in flash.
         for (int i = 0; i < offset; i++) {
            SaveFlashSaveData(((uint8_t *)((((uint32_t)stream->buf) + offset)&~WORK_BLOCK_SIZE) + i), &fill, 1);
         }
      }

      stream->ptr = ptr;
      return 0;
   }

   return -1;
}

void memstream_rewind(memstream_t *stream)
{
   memstream_seek(stream, 0L, SEEK_SET);
}

uint64_t memstream_pos(memstream_t *stream)
{
   return stream->ptr;
}

char *memstream_gets(memstream_t *stream, char *buffer, size_t len)
{
   return NULL;
}

int memstream_getc(memstream_t *stream)
{
   int ret = 0;
   if (stream->ptr >= stream->size)
      return EOF;
   ret = stream->buf[stream->ptr++];

   if (stream->ptr > stream->max_ptr)
      stream->max_ptr = stream->ptr;

   return ret;
}

void memstream_putc(memstream_t *stream, int c)
{
   if (stream->ptr < stream->size)
      SaveFlashSaveData(&stream->buf[stream->ptr++],(uint8_t *)&c,1);

   if (stream->ptr > stream->max_ptr)
      stream->max_ptr = stream->ptr;
}
#endif