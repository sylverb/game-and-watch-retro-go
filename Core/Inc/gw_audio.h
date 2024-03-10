#ifndef _GW_AUDIO_H_
#define _GW_AUDIO_H_

#include "main.h"

extern SAI_HandleTypeDef hsai_BlockA1;
extern DMA_HandleTypeDef hdma_sai1_a;

// Default to 50Hz as it results in more samples than at 60Hz
#define AUDIO_SAMPLE_RATE   (48000)
#define AUDIO_BUFFER_LENGTH (AUDIO_SAMPLE_RATE / 50)
extern uint32_t audio_mute;

typedef enum {
    DMA_TRANSFER_STATE_HF = 0x00,
    DMA_TRANSFER_STATE_TC = 0x01,
} dma_transfer_state_t;

extern int16_t audiobuffer_dma[AUDIO_BUFFER_LENGTH * 2] __attribute__((section (".audio")));
extern dma_transfer_state_t dma_state;
extern uint32_t dma_counter;

//TODO: Rename to follow naming convention
void set_audio_frequency(uint32_t frequency);
uint16_t audio_get_buffer_length(void);
uint16_t audio_get_buffer_size(void);
int16_t *audio_get_active_buffer(void);
int16_t *audio_get_inactive_buffer(void);
void audio_clear_active_buffer(void);
void audio_clear_inactive_buffer(void);
void audio_clear_buffers(void);
void audio_set_buffer_length(uint16_t length);
void audio_start_playing(uint16_t length);
void audio_stop_playing(void);

#endif
