#pragma once

#include "filesystem.h"

int app_main_gwenesis(uint8_t load_state, uint8_t start_paused, uint8_t save_slot);
void gwenesis_save_local_data(fs_file_t *file);
void gwenesis_load_local_data(fs_file_t *file);
