#ifndef _MAIN_MSX_H_
#define _MAIN_MSX_H_
#include <stdint.h>
extern uint8_t msx_framebuffer[];
extern int msx_button_a_key_index;
extern int msx_button_b_key_index;
void app_main_msx(uint8_t load_state, uint8_t start_paused, int8_t save_slot);
void save_gnw_msx_data();
void load_gnw_msx_data();
void msxLedSetFdd1(int state);
size_t msx_getromdata(uint8_t **data, uint8_t *src_data, size_t src_size, const char *ext);
#if CHEAT_CODES == 1
void update_cheats_msx();
#endif
#endif
