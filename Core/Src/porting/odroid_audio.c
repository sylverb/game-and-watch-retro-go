#include "odroid_audio.h"
#include "gw_audio.h"
#include "gw_multisync.h"
#include "odroid_system.h"

uint8_t audio_level = ODROID_AUDIO_VOLUME_MAX;

void odroid_audio_init(int sample_rate) {
    audio_set_frequency(sample_rate);
    multisync_init();
    audio_level = odroid_settings_Volume_get();
}

void odroid_audio_volume_set(int level) {
    audio_level = level;
    odroid_settings_Volume_set(audio_level);
}

int odroid_audio_volume_get() {
    return audio_level;
}
