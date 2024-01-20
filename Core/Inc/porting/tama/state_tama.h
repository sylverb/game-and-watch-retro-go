#pragma once

#include <stdint.h>
#include <stdbool.h>

bool tama_state_save(const uint8_t *save_state_ptr);

bool tama_state_load(const uint8_t *save_state_ptr);