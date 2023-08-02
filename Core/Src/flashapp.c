#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"

#include <odroid_system.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "githash.h"
#include "gui.h"
#include "bitmaps.h"
#include "gw_buttons.h"
#include "gw_flash.h"
#include "gw_lcd.h"
#include "gw_linker.h"
#include "main.h"
#include "rg_emulators.h"
#include "lzma.h"

#include "utils.h"
#include "sha256.h"

#define DBG(...) printf(__VA_ARGS__)
// #define DBG(...)

#define STATUS_HEIGHT (33)
#define HEADER_HEIGHT (47)
#define IMAGE_BANNER_HEIGHT (32)
#define IMAGE_BANNER_WIDTH  (ODROID_SCREEN_WIDTH)

static const int font_height = 8; //odroid_overlay_get_font_size();
static const int font_width = 8; //odroid_overlay_get_font_width();

#define LIST_X_OFFSET    (0)
#define LIST_Y_OFFSET    (STATUS_HEIGHT)
#define LIST_WIDTH       (ODROID_SCREEN_WIDTH)
#define LIST_HEIGHT      (ODROID_SCREEN_HEIGHT - STATUS_HEIGHT - HEADER_HEIGHT)
#define LIST_LINE_HEIGHT (font_height + 2)
#define LIST_LINE_COUNT  (LIST_HEIGHT / LIST_LINE_HEIGHT)

#define PROGRESS_X_OFFSET (ODROID_SCREEN_WIDTH / 5 / 2)
#define PROGRESS_Y_OFFSET (LIST_Y_OFFSET + 9 * LIST_LINE_HEIGHT)
#define PROGRESS_WIDTH    (4 * (PROGRESS_X_OFFSET * 2))
#define PROGRESS_HEIGHT   (2 * LIST_LINE_HEIGHT)

#define PERFORM_HASH_CHECK 1

// TODO: Make this nicer
extern OSPI_HandleTypeDef hospi1;

typedef enum {
    FLASHAPP_INIT                   = 0x00,
    FLASHAPP_IDLE                   = 0x01,
    FLASHAPP_START                  = 0x02,
    FLASHAPP_CHECK_HASH_RAM_NEXT    = 0x03,
    FLASHAPP_CHECK_HASH_RAM         = 0x04,
    FLASHAPP_DECOMPRESSING          = 0x05,
    FLASHAPP_ERASE_NEXT             = 0x06,
    FLASHAPP_ERASE                  = 0x07,
    FLASHAPP_PROGRAM_NEXT           = 0x08,
    FLASHAPP_PROGRAM                = 0x09,
    FLASHAPP_CHECK_HASH_FLASH_NEXT  = 0x0A,
    FLASHAPP_CHECK_HASH_FLASH       = 0x0B,

    FLASHAPP_FINAL                  = 0x0C,
    FLASHAPP_ERROR                  = 0x0D,
} flashapp_state_t;

typedef enum {
    FLASHAPP_BOOTING = 0,

    FLASHAPP_STATUS_BAD_HASH_RAM    = 0xbad00001,
    FLASHAPP_STATUS_BAD_HAS_FLASH   = 0xbad00002,
    FLASHAPP_STATUS_NOT_ALIGNED     = 0xbad00003,

    FLASHAPP_STATUS_IDLE            = 0xcafe0000,
    FLASHAPP_STATUS_DONE            = 0xcafe0001,
    FLASHAPP_STATUS_BUSY            = 0xcafe0002,
} flashapp_status_t;

typedef struct {
    tab_t    tab;
    uint32_t erase_address;
    uint32_t erase_bytes_left;
    uint32_t current_program_address;
    uint32_t program_bytes_left;
    uint8_t* program_buf;
    uint32_t progress_max;
    uint32_t progress_value;
} flashapp_t;

struct work_context {
    // Number of bytes to program in the flash
    uint32_t size;

    // Where to program in the flash
    // offset into flash, not an absolute address 0x9XXX_XXXX
    uint32_t address;

    // Whether or not an erase should be performed
    uint32_t erase;

    // Number of bytes to be erased from program_address
    int32_t erase_bytes;

    // 0 if the data has not been compressed
    uint32_t decompressed_size;

    // The expected sha256 of the loaded binary
    uint8_t expected_sha256[32];

    // The expected sha256 hash of the decompressed data (if originally compressed)
    uint8_t expected_sha256_decompressed[32];

    unsigned char *buffer;

    // This work context is ready for the on-device flashapp to process.
    // Put this last so a memset sets it last.
    uint32_t ready;
};

struct flashapp_comm {  // Values are read or written by the debugger
                        // only add attributes at the end (before work_buffers)
                        // so that addresses don't change.
    // FlashApp state-machine state
    uint32_t flashapp_state;

    // Status register
    uint32_t program_status;

    // Host-setable timestamp; if 0, RTC is not updated.
    uint32_t utc_timestamp;

    // Current chunk index
    uint32_t program_chunk_idx;

    // Number of chunks
    uint32_t program_chunk_count;

    struct work_context contexts[2];

    unsigned char buffer[2][256 << 10];

    uint32_t active_context_index;

    struct work_context active_context;

    unsigned char decompress_buffer[256 << 10];
};

// framebuffer1 is used as an actual framebuffer.
// framebuffer2 and onwards is used as a buffer for the flash.
static volatile struct flashapp_comm *comm = (struct flashapp_comm *)framebuffer2;

// TODO: Expose properly
int odroid_overlay_draw_text_line(uint16_t x_pos,
                                  uint16_t y_pos,
                                  uint16_t width,
                                  const char *text,
                                  uint16_t color,
                                  uint16_t color_bg);

static void draw_text_line_centered(uint16_t y_pos,
                                    const char *text,
                                    uint16_t color,
                                    uint16_t color_bg)
{
    int width = strlen(text) * font_width;
    int x_pos = ODROID_SCREEN_WIDTH / 2 - width / 2;

    odroid_overlay_draw_text_line(x_pos, y_pos, width, text, color, color_bg);
}

static void draw_progress(flashapp_t *flashapp)
{
    char progress_str[16];

    odroid_overlay_draw_fill_rect(0, LIST_Y_OFFSET, LIST_WIDTH, LIST_HEIGHT, curr_colors->bg_c);

    odroid_overlay_draw_text_line(8, LIST_Y_OFFSET + LIST_LINE_HEIGHT, strlen(flashapp->tab.status) * font_width, flashapp->tab.status, curr_colors->sel_c, curr_colors->bg_c);

    draw_text_line_centered(LIST_Y_OFFSET + 5 * LIST_LINE_HEIGHT, flashapp->tab.name, curr_colors->sel_c, curr_colors->bg_c);

    if (flashapp->progress_max != 0) {
        int32_t progress_percent = (100 * (uint64_t)flashapp->progress_value) / flashapp->progress_max;
        int32_t progress_width = (PROGRESS_WIDTH * (uint64_t)flashapp->progress_value) / flashapp->progress_max;

        sprintf(progress_str, "%ld%%", progress_percent);

        odroid_overlay_draw_fill_rect(PROGRESS_X_OFFSET,
                                      PROGRESS_Y_OFFSET,
                                      PROGRESS_WIDTH,
                                      PROGRESS_HEIGHT,
                                      curr_colors->main_c);

        odroid_overlay_draw_fill_rect(PROGRESS_X_OFFSET,
                                      PROGRESS_Y_OFFSET,
                                      progress_width,
                                      PROGRESS_HEIGHT,
                                      curr_colors->sel_c);

        draw_text_line_centered(LIST_Y_OFFSET + 8 * LIST_LINE_HEIGHT, progress_str, curr_colors->sel_c, curr_colors->bg_c);
    }
}

static void redraw(flashapp_t *flashapp)
{
    // Re-use header, status and footer from the retro-go code
    gui_draw_header(&flashapp->tab);
    gui_draw_status(&flashapp->tab);

    // Empty logo
    //odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - IMAGE_BANNER_HEIGHT - 15,
    //                              IMAGE_BANNER_WIDTH, IMAGE_BANNER_HEIGHT, curr_colors->main_c);

    draw_progress(flashapp);
    lcd_swap();
}


static void state_set(flashapp_state_t state_next)
{
    printf("State: %ld -> %d\n", comm->flashapp_state, state_next);

    comm->flashapp_state = state_next;
}

static void state_inc(void)
{
    state_set(comm->flashapp_state + 1);
}

static void flashapp_run(flashapp_t *flashapp)
{
    struct work_context *context = &comm->active_context;
    uint8_t program_calculated_sha256[32];

    switch (comm->flashapp_state) {
    case FLASHAPP_INIT:
        // Clear variables shared with the host
        memset(comm, 0, sizeof(*comm));
        comm->program_chunk_count = 1;

        flashapp->progress_value = 0;
        flashapp->progress_max = 0;

        state_inc();
        break;
    case FLASHAPP_IDLE:
        sprintf(flashapp->tab.name, "1. Waiting for data");

        // Notify that we are ready to start
        comm->program_status = FLASHAPP_STATUS_IDLE;
        flashapp->progress_value = 0;
        flashapp->progress_max = 0;

        if(comm->utc_timestamp){
            // Set time
            GW_SetUnixTime(comm->utc_timestamp);
            comm->utc_timestamp = 0;
        }

        // Attempt to find a ready context
        for(uint8_t i=0; i < 2; i++){
            if(comm->contexts[i].ready){
                comm->active_context_index = i;
                memcpy(context, &comm->contexts[i], sizeof(struct work_context));
                context->buffer = comm->buffer[i];

                if(context->erase){
                    flashapp->erase_address = context->address;
                    flashapp->erase_bytes_left = context->erase_bytes;

                    uint32_t smallest_erase = OSPI_GetSmallestEraseSize();

                    if (flashapp->erase_address & (smallest_erase - 1)) {
                        sprintf(flashapp->tab.name, "** Address not aligned to smallest erase size! **");
                        comm->program_status = FLASHAPP_STATUS_NOT_ALIGNED;
                        state_set(FLASHAPP_ERROR);
                        break;
                    }

                    // Round size up to nearest erase size if needed ?
                    if ((flashapp->erase_bytes_left & (smallest_erase - 1)) != 0) {
                        flashapp->erase_bytes_left += smallest_erase - (flashapp->erase_bytes_left & (smallest_erase - 1));
                    }

                    // Start a non-blocking flash erase to run in the background
                    OSPI_DisableMemoryMappedMode();
                    OSPI_Erase(&flashapp->erase_address, &flashapp->erase_bytes_left, false);
                }
                state_inc();
                break;
            }
        }

        break;
    case FLASHAPP_START:
        comm->program_status = FLASHAPP_STATUS_BUSY;
        state_inc();
        break;
    case FLASHAPP_CHECK_HASH_RAM_NEXT:
        sprintf(flashapp->tab.name, "2. Checking hash in RAM (%ld bytes)", context->size);
        state_inc();
        break;
    case FLASHAPP_CHECK_HASH_RAM:
        // Calculate sha256 hash of the RAM first
#if PERFORM_HASH_CHECK
        sha256(program_calculated_sha256, (const BYTE*) context->buffer, context->size);

        if (memcmp((const void *)program_calculated_sha256, (const void *)context->expected_sha256, 32) != 0) {
            // Hashes don't match even in RAM, openocd loading failed.
            sprintf(flashapp->tab.name, "*** Hash mismatch in RAM ***");
            comm->program_status = FLASHAPP_STATUS_BAD_HASH_RAM;
            state_set(FLASHAPP_ERROR);
            break;
        } else {
            sprintf(flashapp->tab.name, "3. Hash OK in RAM");
            state_inc();
        }
#else
        state_inc();
#endif
        break;
    case FLASHAPP_DECOMPRESSING:
        // Decompress the data; nothing after this state should reference decompression.
        if(context->decompressed_size){
            uint32_t n_decomp_bytes;
            n_decomp_bytes = lzma_inflate(comm->decompress_buffer, sizeof(comm->decompress_buffer),
                                          context->buffer, context->size);
            assert(n_decomp_bytes == context->decompressed_size);

            context->size = context->decompressed_size;
            context->decompressed_size = 0;
            context->buffer = comm->decompress_buffer;
            memcpy(context->expected_sha256, context->expected_sha256_decompressed, 32);

            // We can now early release the context
            memset(&comm->contexts[comm->active_context_index], 0, sizeof(struct work_context));

            state_set(FLASHAPP_CHECK_HASH_RAM_NEXT);
        }
        else{
            if(context->buffer != comm->decompress_buffer){
                //The data came in NOT compressed
                memcpy(comm->decompress_buffer, context->buffer, 256 << 10);
                context->buffer = comm->decompress_buffer;

                // We can now early release the context
                memset(&comm->contexts[comm->active_context_index], 0, sizeof(struct work_context));
            }
            state_inc();
        }
        break;
    case FLASHAPP_ERASE_NEXT:
        if (context->erase) {
            if (context->erase_bytes == 0) {
                sprintf(flashapp->tab.name, "4. Performing Chip Erase (takes time)");
            }
            else {
                sprintf(flashapp->tab.name, "4. Erasing %ld bytes...", flashapp->erase_bytes_left);
                printf("Erasing %ld bytes at 0x%08lx\n", context->erase_bytes, flashapp->erase_address);
            }
            state_inc();
        } else {
            state_set(FLASHAPP_PROGRAM_NEXT);
        }
        break;
    case FLASHAPP_ERASE:
        OSPI_DisableMemoryMappedMode();
        if (context->erase_bytes == 0) {
            OSPI_NOR_WriteEnable();
            OSPI_ChipErase();
            state_inc();
        } else {
            if (OSPI_Erase(&flashapp->erase_address, &flashapp->erase_bytes_left, true)) {
                state_inc();
            }
        }
        break;
    case FLASHAPP_PROGRAM_NEXT:
        sprintf(flashapp->tab.name, "5. Programming...");
        flashapp->progress_value = 0;
        flashapp->current_program_address = context->address;

        flashapp->progress_max = context->size;
        flashapp->program_bytes_left = context->size;
        flashapp->program_buf = context->buffer;

        state_inc();
        break;
    case FLASHAPP_PROGRAM:
        OSPI_DisableMemoryMappedMode();
        if (flashapp->program_bytes_left > 0) {
            uint32_t dest_page = flashapp->current_program_address / 256;
            uint32_t bytes_to_write = flashapp->program_bytes_left > 256 ? 256 : flashapp->program_bytes_left;
            OSPI_NOR_WriteEnable();
            OSPI_PageProgram(dest_page * 256, flashapp->program_buf, bytes_to_write);
            flashapp->current_program_address += bytes_to_write;
            flashapp->program_buf += bytes_to_write;
            flashapp->program_bytes_left -= bytes_to_write;
            flashapp->progress_value = flashapp->progress_max - flashapp->program_bytes_left;
        } else {
            state_inc();
        }
        break;
    case FLASHAPP_CHECK_HASH_FLASH_NEXT:
        sprintf(flashapp->tab.name, "6. Checking hash in FLASH");
        OSPI_EnableMemoryMappedMode();
        state_inc();
        break;
    case FLASHAPP_CHECK_HASH_FLASH:
#if PERFORM_HASH_CHECK
        // Calculate sha256 hash of the FLASH.
        sha256(program_calculated_sha256, (const BYTE*) (0x90000000 + context->address), context->size);

        if (memcmp((char *)program_calculated_sha256, (char *)context->expected_sha256, 32) != 0) {
            // Hashes don't match in FLASH, programming failed.
            sprintf(flashapp->tab.name, "*** Hash mismatch in FLASH ***");
            comm->program_status = FLASHAPP_STATUS_BAD_HAS_FLASH;
            state_set(FLASHAPP_ERROR);
            break;
        }
#endif
        sprintf(flashapp->tab.name, "7. Hash OK in FLASH.");
        state_set(FLASHAPP_IDLE);
        break;
    case FLASHAPP_FINAL:
    case FLASHAPP_ERROR:
        // Stay in state until reset.
        break;
    }
}


void flashapp_main(void)
{
    flashapp_t flashapp = {};
    flashapp.tab.img_header = &logo_flash;
    flashapp.tab.img_logo = &logo_gnw;

    SCB_InvalidateDCache();
    SCB_DisableDCache();

    odroid_system_init(0, 32000);
    lcd_set_buffers(framebuffer1, framebuffer1);

    while (true) {
        if (comm->program_chunk_count == 1) {
            sprintf(flashapp.tab.status, "Game and Watch Flash App");
        } else {
            sprintf(flashapp.tab.status, "Game and Watch Flash App (%ld/%ld)",
                    comm->program_chunk_idx, comm->program_chunk_count);
        }

        // Run multiple times to skip rendering when programming
        for (int i = 0; i < 128; i++) {
            wdog_refresh();
            flashapp_run(&flashapp);
        }

        lcd_sync();
        lcd_swap();
        lcd_wait_for_vblank();
        redraw(&flashapp);
    }
}

#pragma GCC diagnostic pop
