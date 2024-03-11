#include "gw_audio.h"
#include <string.h>
#include "gw_multisync.h"

#define MD_AUDIO_FREQ_NTSC 53267
#define MD_AUDIO_FREQ_PAL 52781
#define GW_AUDIO_FREQUENCY 32768

//TODO: All below vars should be static and have extern removed from .h file
uint32_t audio_mute;

int16_t audiobuffer_dma[AUDIO_BUFFER_LENGTH * 2] __attribute__((section(".audio")));

dma_transfer_state_t dma_state;
uint32_t dma_counter;

static uint16_t audiobuffer_length = AUDIO_BUFFER_LENGTH;

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    dma_counter++;
    dma_state = DMA_TRANSFER_STATE_HF;
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    dma_counter++;
    dma_state = DMA_TRANSFER_STATE_TC;
}

/* set audio frequency  */
void audio_set_frequency(uint32_t frequency) {
    // (re)config PLL2 and SAI
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    if (frequency == 12000) {
        // MCLK = 8, target = 12000, realized = 12000, error = 0.00%
        PeriphClkInitStruct.PLL2.PLL2M = 25;
        PeriphClkInitStruct.PLL2.PLL2N = 96;
        PeriphClkInitStruct.PLL2.PLL2P = 10;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    } else if (frequency == SAI_AUDIO_FREQUENCY_16K) {
        // MCLK = 8, target = 16000, realized = 16000, error = 0.00%
        PeriphClkInitStruct.PLL2.PLL2M = 25;
        PeriphClkInitStruct.PLL2.PLL2N = 128;
        PeriphClkInitStruct.PLL2.PLL2P = 10;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    } else if (frequency == 18000) {
        // MCLK = 8, target = 18000, realized = 18000, error = 0.00%
        PeriphClkInitStruct.PLL2.PLL2M = 25;
        PeriphClkInitStruct.PLL2.PLL2N = 144;
        PeriphClkInitStruct.PLL2.PLL2P = 10;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    } else if (frequency == 22050) {
        // MCLK = 8, target = 22050, realized = 22050, error = 0.00%
        PeriphClkInitStruct.PLL2.PLL2M = 18;
        PeriphClkInitStruct.PLL2.PLL2N = 127;
        PeriphClkInitStruct.PLL2.PLL2P = 10;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 66;
    } else if (frequency == 31200) {
        // MCLK = 8, target = 31200, realized = 31200, error = 0.00%
        PeriphClkInitStruct.PLL2.PLL2M = 14;
        PeriphClkInitStruct.PLL2.PLL2N = 55;
        PeriphClkInitStruct.PLL2.PLL2P = 4;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 7458;
    } else if (frequency == 31440) {
        // MCLK = 8, target = 31440, realized = 31440, error = 0.00%
        PeriphClkInitStruct.PLL2.PLL2M = 33;
        PeriphClkInitStruct.PLL2.PLL2N = 166;
        PeriphClkInitStruct.PLL2.PLL2P = 5;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 27;
    } else if (frequency == GW_AUDIO_FREQUENCY) {
        // MCLK = 6, target = 32768, realized = 32768, error = 0.00%
        PeriphClkInitStruct.PLL2.PLL2M = 25;
        PeriphClkInitStruct.PLL2.PLL2N = 196;
        PeriphClkInitStruct.PLL2.PLL2P = 10;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 5000;
    } else if (frequency == MD_AUDIO_FREQ_NTSC) {
        // MCLK = 8, target = 53267, realized = 53267, error = 0.00%
        PeriphClkInitStruct.PLL2.PLL2M = 11;
        PeriphClkInitStruct.PLL2.PLL2N = 75;
        PeriphClkInitStruct.PLL2.PLL2P = 4;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    } else if (frequency == MD_AUDIO_FREQ_PAL) {
        // MCLK = 8, target = 52781, realized = 52781, error = 0.00%
        PeriphClkInitStruct.PLL2.PLL2M = 9;
        PeriphClkInitStruct.PLL2.PLL2N = 76;
        PeriphClkInitStruct.PLL2.PLL2P = 5;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 38;
    } else {
        /* default to 48 KHz */
        // MCLK = 8, target = 48000, realized = 48000, error = 0.00%
        PeriphClkInitStruct.PLL2.PLL2M = 25;
        PeriphClkInitStruct.PLL2.PLL2N = 192;
        PeriphClkInitStruct.PLL2.PLL2P = 5;
        PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
        frequency = SAI_AUDIO_FREQUENCY_48K;
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL2;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;

    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 5;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_1;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /* remove the current configuration */
    HAL_SAI_DeInit(&hsai_BlockA1);

    /* set new frequency and let HAL_SAI_Init() auto calculate the MCLK */
    hsai_BlockA1.Init.AudioFrequency = frequency;

    /* apply the new configuration */
    HAL_SAI_Init(&hsai_BlockA1);
    HAL_SAI_RegisterCallback(&hsai_BlockA1, HAL_SAI_TX_HALFCOMPLETE_CB_ID, &multisync_SAI_TxHalfCpltCallback);
    HAL_SAI_RegisterCallback(&hsai_BlockA1, HAL_SAI_TX_COMPLETE_CB_ID, &multisync_SAI_TxCpltCallback);
}

uint16_t audio_get_buffer_length() {
    return audiobuffer_length;
}

uint16_t audio_get_buffer_size() {
    return audio_get_buffer_length() * sizeof(int16_t);
}

int16_t *audio_get_active_buffer(void) {
    size_t offset = (dma_state == DMA_TRANSFER_STATE_HF) ? 0 : audiobuffer_length;

    return &audiobuffer_dma[offset];
}

int16_t *audio_get_inactive_buffer(void) {
    size_t offset = (dma_state == DMA_TRANSFER_STATE_TC) ? 0 : audiobuffer_length;

    return &audiobuffer_dma[offset];
}

void audio_clear_active_buffer() {
    memset(audio_get_active_buffer(), 0, audiobuffer_length * sizeof(audiobuffer_dma[0]));
}

void audio_clear_inactive_buffer() {
    memset(audio_get_inactive_buffer(), 0, audiobuffer_length * sizeof(audiobuffer_dma[0]));
}

void audio_clear_buffers() {
    memset(audiobuffer_dma, 0, sizeof(audiobuffer_dma));
}

void audio_set_buffer_length(uint16_t length) {
    audiobuffer_length = length;
}

void audio_start_playing(uint16_t length) {
    audio_clear_buffers();
    audio_set_buffer_length(length);
    HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t *) audiobuffer_dma, audiobuffer_length * 2);
}

void audio_stop_playing() {
    HAL_SAI_DMAStop(&hsai_BlockA1);
}
