#include <odroid_system.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "gw_linker.h"
#include "gw_malloc.h"
#include "rg_emulators.h"
#include "rg_i18n.h"
#include "bitmaps.h"
#include "gui.h"
#include "rom_manager.h"
#include "gw_lcd.h"
#include "main.h"
#include "main_gb.h"
#include "main_nes.h"
#include "main_nes_fceu.h"
#include "main_smsplusgx.h"
#include "main_pce.h"
#include "main_msx.h"
#include "main_gw.h"
#include "main_wsv.h"
#include "main_gwenesis.h"
#include "main_a7800.h"
#include "main_amstrad.h"
#include "rg_rtc.h"

#if !defined(COVERFLOW)
#define COVERFLOW 0
#endif /* COVERFLOW */
// Increase when adding new emulators
#define MAX_EMULATORS 13
static retro_emulator_t emulators[MAX_EMULATORS];
static int emulators_count = 0;

const unsigned int intflash_magic_sign = 0xABAB;
const unsigned int extflash_magic_sign __attribute__((section(".extflash_emu_data"))) = intflash_magic_sign;

static retro_emulator_file_t *CHOSEN_FILE = NULL;

retro_emulator_t *file_to_emu(retro_emulator_file_t *file) {
    for (int i = 0; i < MAX_EMULATORS; i++)
        if (emulators[i].system == file->system)
            return &emulators[i];
    return NULL;
}

static void event_handler(gui_event_t event, tab_t *tab)
{
    retro_emulator_t *emu = (retro_emulator_t *)tab->arg;
    listbox_item_t *item = gui_get_selected_item(tab);
    retro_emulator_file_t *file = (retro_emulator_file_t *)(item ? item->arg : NULL);

    if (event == TAB_INIT)
    {
        emulator_init(emu);

        if (emu->roms.count > 0)
        {
            sprintf(tab->status, "%s", emu->system_name);
            gui_resize_list(tab, emu->roms.count);

            for (int i = 0; i < emu->roms.count; i++)
            {
                tab->listbox.items[i].text = emu->roms.files[i].name;
                tab->listbox.items[i].arg = (void *)&emu->roms.files[i];
            }

            gui_sort_list(tab, 0);
            tab->is_empty = false;
        }
        else
        {
            sprintf(tab->status, " No games");
            gui_resize_list(tab, 8);
            //size_t len = 0;
            //tab->listbox.items[0].text = asnprintf(NULL, &len, "Place roms in folder: /roms/%s", emu->dirname);
            //len = 0;
            //tab->listbox.items[2].text = asnprintf(NULL, &len, "With file extension: .%s", emu->ext);
            //tab->listbox.items[4].text = "Use SELECT and START to navigate.";
            tab->listbox.cursor = 3;
            tab->is_empty = true;
        }
    }

    /* The rest of the events require a file to be selected */
    if (file == NULL)
        return;

    if (event == KEY_PRESS_A)
    {
        emulator_show_file_menu(file);
        gui_redraw();
    }
    else if (event == KEY_PRESS_B)
    {
        emulator_show_file_info(file);
        gui_redraw();
    }
    else if (event == TAB_IDLE)
    {
        //if (file->checksum == 0) {
        //    emulator_crc32_file(file);
        // }
    }
    else if (event == TAB_REDRAW)
    {
        gui_redraw();
    }
}

static void add_emulator(const char *system, const char *dirname, const char* ext, const char *part,
                          uint16_t crc_offset, const void *logo, const void *header)
{
    assert(emulators_count <= MAX_EMULATORS);
    retro_emulator_t *p = &emulators[emulators_count++];
    strcpy(p->system_name, system);
    //strcpy(p->dirname, dirname);
    strcpy(p->ext, ext);
    p->partition = 0;
    p->roms.count = 0;
    p->roms.files = NULL;
    p->initialized = false;
    p->crc_offset = crc_offset;

    gui_add_tab(dirname, logo, header, p, event_handler);

    emulator_init(p);
}

void emulator_init(retro_emulator_t *emu)
{
    if (emu->initialized)
        return;

    emu->initialized = true;

    printf("Retro-Go: Initializing emulator '%s'\n", emu->system_name);


    const rom_system_t *system = rom_manager_system(&rom_mgr, emu->system_name);
    if (system) {
        emu->system = system;
        emu->roms.files = system->roms;
        emu->roms.count = system->roms_count;
#if COVERFLOW != 0        
        emu->cover_height = system->cover_height;
        emu->cover_width = system->cover_width;
#endif
    } else {
        while(1) {
            lcd_backlight_on();
            HAL_Delay(100);
            lcd_backlight_off();
            HAL_Delay(100);
        }
    }

    // retro_emulator_file_t *file = &emu->roms.files[emu->roms.count++];
    // strcpy(file->folder, "/");
    // strcpy(file->name, "test");
    // strcpy(file->ext, "gb");
    // file->emulator = (void*)emu;
    // file->crc_offset = emu->crc_offset;
    // file->checksum = 0;


    // char path[128];
    // char *files = NULL;
    // size_t count = 0;

    // sprintf(path, ODROID_BASE_PATH_CRC_CACHE "/%s", emu->dirname);
    // odroid_sdcard_mkdir(path);

    // sprintf(path, ODROID_BASE_PATH_SAVES "/%s", emu->dirname);
    // odroid_sdcard_mkdir(path);

    // sprintf(path, ODROID_BASE_PATH_ROMS "/%s", emu->dirname);
    // odroid_sdcard_mkdir(path);

    // if (odroid_sdcard_list(path, &files, &count) == 0 && count > 0)
    // {
    //     emu->roms.files = rg_alloc(count * sizeof(retro_emulator_file_t), MEM_ANY);
    //     emu->roms.count = 0;

    //     char *ptr = files;
    //     for (int i = 0; i < count; ++i)
    //     {
    //         const char *name = ptr;
    //         const char *ext = odroid_sdcard_get_extension(ptr);
    //         size_t name_len = strlen(name);

    //         // Advance pointer to next entry
    //         ptr += name_len + 1;

    //         if (!ext || strcasecmp(emu->ext, ext) != 0) //  && strcasecmp("zip", ext) != 0
    //             continue;

    //         retro_emulator_file_t *file = &emu->roms.files[emu->roms.count++];
    //         strcpy(file->folder, path);
    //         strcpy(file->name, name);
    //         strcpy(file->ext, ext);
    //         file->name[name_len-strlen(ext)-1] = 0;
    //         file->emulator = (void*)emu;
    //         file->crc_offset = emu->crc_offset;
    //         file->checksum = 0;
    //     }
    // }
    // free(files);
}

const uint32_t *emu_get_file_address(retro_emulator_file_t *file)
{
    // static char buffer[192];
    // if (file == NULL) return NULL;
    // sprintf(buffer, "%s/%s.%s", file->folder, file->name, file->ext);
    // return (const char*)&buffer;
    return (uint32_t *) file->address;
}

/*bool emulator_build_file_object(const char *path, retro_emulator_file_t *file)
{
    const char *name = odroid_sdcard_get_filename(path);
    const char *ext = odroid_sdcard_get_extension(path);

    if (ext == NULL || name == NULL)
        return false;

    memset(file, 0, sizeof(retro_emulator_file_t));
    strncpy(file->folder, path, strlen(path)-strlen(name)-1);
    strncpy(file->name, name, strlen(name)-strlen(ext)-1);
    strcpy(file->ext, ext);

    const char *dirname = odroid_sdcard_get_filename(file->folder);

    for (int i = 0; i < emulators_count; ++i)
    {
        if (strcmp(emulators[i].dirname, dirname) == 0)
        {
            file->crc_offset = emulators[i].crc_offset;
            file->emulator = &emulators[i];
            return true;
        }
    }

    return false;
}*/

void emulator_crc32_file(retro_emulator_file_t *file)
{
    // if (file == NULL || file->checksum > 0)
    //     return;

    // const int chunk_size = 32768;
    // const char *file_path = emu_get_file_path(file);
    // char *cache_path = odroid_system_get_path(ODROID_PATH_CRC_CACHE, file_path);
    // FILE *fp, *fp2;

    // file->missing_cover = 0;

    // if ((fp = fopen(cache_path, "rb")) != NULL)
    // {
    //     fread(&file->checksum, 4, 1, fp);
    //     fclose(fp);
    // }
    // else if ((fp = fopen(file_path, "rb")) != NULL)
    // {
    //     void *buffer = malloc(chunk_size);
    //     uint32_t crc_tmp = 0;
    //     uint32_t count = 0;

    //     gui_draw_notice("        CRC32", C_GREEN);

    //     fseek(fp, file->crc_offset, SEEK_SET);
    //     while (true)
    //     {
    //         odroid_input_read_gamepad(&gui.joystick);
    //         if (gui.joystick.bitmask > 0) break;

    //         count = fread(buffer, 1, chunk_size, fp);
    //         if (count == 0) break;

    //         crc_tmp = crc32_le(crc_tmp, buffer, count);
    //         if (count < chunk_size) break;
    //     }

    //     free(buffer);

    //     if (feof(fp))
    //     {
    //         file->checksum = crc_tmp;
    //         if ((fp2 = fopen(cache_path, "wb")) != NULL)
    //         {
    //             fwrite(&file->checksum, 4, 1, fp2);
    //             fclose(fp2);
    //         }
    //     }
    //     fclose(fp);
    // }
    // else
    // {
    //     file->checksum = 1;
    // }

    // free(cache_path);

    // gui_draw_notice(" ", C_RED);
}

void emulator_show_file_info(retro_emulator_file_t *file)
{
    char filename_value[128];
    char type_value[32];
    char size_value[32];
#if COVERFLOW != 0
    char img_size[32];
#endif
    //char crc_value[32];
    //crc_value[0] = '\x00';

    odroid_dialog_choice_t choices[] = {
        {-1, curr_lang->s_File, filename_value, 0, NULL},
        {-1, curr_lang->s_Type, type_value, 0, NULL},
        {-1, curr_lang->s_Size, size_value, 0, NULL},
		#if COVERFLOW != 0
        {-1, curr_lang->s_ImgSize, img_size, 0, NULL},
		#endif
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {1, curr_lang->s_Close, "", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST
    };

    sprintf(choices[0].value, "%.127s", file->name);
    sprintf(choices[1].value, "%s", file->ext);
    sprintf(choices[2].value, "%d KB", (int)(file->size / 1024));
    #if COVERFLOW != 0
    sprintf(choices[3].value, "%d KB", file->img_size / 1024);
	#endif

    odroid_overlay_dialog(curr_lang->s_GameProp, choices, -1);
}

#if CHEAT_CODES == 1
static bool cheat_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    bool is_on = odroid_settings_ActiveGameGenieCodes_is_enabled(CHOSEN_FILE->id, option->id);
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) 
    {
        is_on = is_on ? false : true;
        odroid_settings_ActiveGameGenieCodes_set(CHOSEN_FILE->id, option->id, is_on);
    }
    strcpy(option->value, is_on ? curr_lang->s_Cheat_Codes_ON : curr_lang->s_Cheat_Codes_OFF);
    return event == ODROID_DIALOG_ENTER;
}

static bool show_cheat_dialog()
{
    static odroid_dialog_choice_t last = ODROID_DIALOG_CHOICE_LAST;

    // +1 for the terminator sentinel
    odroid_dialog_choice_t *choices = rg_alloc((CHOSEN_FILE->cheat_count + 1) * sizeof(odroid_dialog_choice_t), MEM_ANY);
    char svalues[MAX_CHEAT_CODES][10];
    for(int i=0; i<CHOSEN_FILE->cheat_count; i++) 
    {
        const char *label = CHOSEN_FILE->cheat_descs[i];
        if (label == NULL) {
            label = CHOSEN_FILE->cheat_codes[i];
        }
        choices[i].id = i;
        choices[i].label = label;
        choices[i].value = svalues[i];
        choices[i].enabled = 1;
        choices[i].update_cb = cheat_update_cb;
    }
    choices[CHOSEN_FILE->cheat_count] = last;
    odroid_overlay_dialog(curr_lang->s_Cheat_Codes_Title, choices, 0);

    rg_free(choices);
    odroid_settings_commit();
    return false;
}
#endif

bool emulator_show_file_menu(retro_emulator_file_t *file)
{
    CHOSEN_FILE = file;
    // char *save_path = odroid_system_get_path(ODROID_PATH_SAVE_STATE, emu_get_file_path(file));
    // char *sram_path = odroid_system_get_path(ODROID_PATH_SAVE_SRAM, emu_get_file_path(file));
    // bool has_save = odroid_sdcard_get_filesize(save_path) > 0;
    // bool has_sram = odroid_sdcard_get_filesize(sram_path) > 0;
    // bool is_fav = favorite_find(file) != NULL;

    bool has_save = 1;
    bool has_sram = 0;
    bool force_redraw = false;

#if CHEAT_CODES == 1
    odroid_dialog_choice_t last = ODROID_DIALOG_CHOICE_LAST;
    odroid_dialog_choice_t cheat_row = {4, curr_lang->s_Cheat_Codes, "", 1, NULL};
    odroid_dialog_choice_t cheat_choice = last; 
    if (CHOSEN_FILE->cheat_count != 0) {
        cheat_choice = cheat_row;
    }

#endif

    odroid_dialog_choice_t choices[] = {
        {0, curr_lang->s_Resume_game, "", has_save && (file->save_address != 0), NULL},
        {1, curr_lang->s_New_game, "", 1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        //{3, is_fav ? s_Del_favorite : s_Add_favorite, "", 1, NULL},
		//ODROID_DIALOG_CHOICE_SEPARATOR,
        {2, curr_lang->s_Delete_save, "", (has_save || has_sram) && (file->save_address != 0), NULL},
#if CHEAT_CODES == 1
        ODROID_DIALOG_CHOICE_SEPARATOR,
        cheat_choice,
#endif

        ODROID_DIALOG_CHOICE_LAST
    };
#if CHEAT_CODES == 1
    if (CHOSEN_FILE->cheat_count == 0)
        choices[4] = last;
#endif

    //Del Some item
    if (file->save_address == 0)
    {
        choices[0] = choices[1];
#if CHEAT_CODES == 1
        choices[1] = choices[2];
        choices[2] = choices[3];
        choices[3] = choices[6];
#else
        choices[1] = choices[4];
#endif
    }

    int sel = odroid_overlay_dialog(file->name, choices, has_save ? 0 : 1);

    if (sel == 0 || sel == 1) {
        gui_save_current_tab();
        emulator_start(file, sel == 0, false, 0);
    }
    else if (sel == 2) {
        if (odroid_overlay_confirm(curr_lang->s_Confiem_del_save, false) == 1) {
            store_erase(file->save_address, file->save_size);
        }
    }
    else if (sel == 3) {
        // if (is_fav)
        //     favorite_remove(file);
        // else
        //     favorite_add(file);
    }
    else if (sel == 4) {
#if CHEAT_CODES == 1
        if (CHOSEN_FILE->cheat_count != 0)
            show_cheat_dialog();
        force_redraw = true;
#endif
    }

    // free(save_path);
    // free(sram_path);
    CHOSEN_FILE = NULL;

    return force_redraw;
}

void emulator_start(retro_emulator_file_t *file, bool load_state, bool start_paused, uint8_t save_slot)
{
    printf("Retro-Go: Starting game: %s\n", file->name);
    rom_manager_set_active_file(file);

    // odroid_settings_StartAction_set(load_state ? ODROID_START_ACTION_RESUME : ODROID_START_ACTION_NEWGAME);
    // odroid_settings_RomFilePath_set(path);
    // odroid_settings_commit();

    // Reinit AHB & ITC RAM memory allocation
    ahb_init();
    itc_init();

    // odroid_system_switch_app(((retro_emulator_t *)file->emulator)->partition);
    retro_emulator_t *emu = file_to_emu(file);
    // TODO: Make this cleaner
    if(strcmp(emu->system_name, "Nintendo Gameboy") == 0) {
#ifdef ENABLE_EMULATOR_GB
        memcpy(&__RAM_EMU_START__, &_OVERLAY_GB_LOAD_START, (size_t)&_OVERLAY_GB_SIZE);
        memset(&_OVERLAY_GB_BSS_START, 0x0, (size_t)&_OVERLAY_GB_BSS_SIZE);
        SCB_CleanDCache_by_Addr((uint32_t *)&__RAM_EMU_START__, (size_t)&_OVERLAY_GB_SIZE);
        app_main_gb(load_state, start_paused, save_slot);
#endif
    } else if(strcmp(emu->system_name, "Nintendo Entertainment System") == 0) {
#ifdef ENABLE_EMULATOR_NES
#if FORCE_NOFRENDO == 1
        memcpy(&__RAM_EMU_START__, &_OVERLAY_NES_LOAD_START, (size_t)&_OVERLAY_NES_SIZE);
        memset(&_OVERLAY_NES_BSS_START, 0x0, (size_t)&_OVERLAY_NES_BSS_SIZE);
        SCB_CleanDCache_by_Addr((uint32_t *)&__RAM_EMU_START__, (size_t)&_OVERLAY_NES_SIZE);
        app_main_nes(load_state, start_paused, save_slot);
#else
        memcpy(&__RAM_EMU_START__, &_OVERLAY_NES_FCEU_LOAD_START, (size_t)&_OVERLAY_NES_FCEU_SIZE);
        memset(&_OVERLAY_NES_FCEU_BSS_START, 0x0, (size_t)&_OVERLAY_NES_FCEU_BSS_SIZE);
        SCB_CleanDCache_by_Addr((uint32_t *)&__RAM_EMU_START__, (size_t)&_OVERLAY_NES_FCEU_SIZE);
        app_main_nes_fceu(load_state, start_paused, save_slot);
#endif
#endif
    } else if(strcmp(emu->system_name, "Sega Master System") == 0 ||
              strcmp(emu->system_name, "Sega Game Gear") == 0     ||
              strcmp(emu->system_name, "Sega SG-1000") == 0       ||
              strcmp(emu->system_name, "Colecovision") == 0 ) {
#if defined(ENABLE_EMULATOR_SMS) || defined(ENABLE_EMULATOR_GG) || defined(ENABLE_EMULATOR_COL) || defined(ENABLE_EMULATOR_SG1000)
        memcpy(&__RAM_EMU_START__, &_OVERLAY_SMS_LOAD_START, (size_t)&_OVERLAY_SMS_SIZE);
        memset(&_OVERLAY_SMS_BSS_START, 0x0, (size_t)&_OVERLAY_SMS_BSS_SIZE);
        SCB_CleanDCache_by_Addr((uint32_t *)&__RAM_EMU_START__, (size_t)&_OVERLAY_SMS_SIZE);
        if (! strcmp(emu->system_name, "Colecovision")) app_main_smsplusgx(load_state, start_paused, save_slot, SMSPLUSGX_ENGINE_COLECO);
        else
        if (! strcmp(emu->system_name, "Sega SG-1000")) app_main_smsplusgx(load_state, start_paused, save_slot, SMSPLUSGX_ENGINE_SG1000);
        else                                            app_main_smsplusgx(load_state, start_paused, save_slot, SMSPLUSGX_ENGINE_OTHERS);
#endif
    } else if(strcmp(emu->system_name, "Game & Watch") == 0 ) {
#ifdef ENABLE_EMULATOR_GW
        memcpy(&__RAM_EMU_START__, &_OVERLAY_GW_LOAD_START, (size_t)&_OVERLAY_GW_SIZE);
        memset(&_OVERLAY_GW_BSS_START, 0x0, (size_t)&_OVERLAY_GW_BSS_SIZE);
        SCB_CleanDCache_by_Addr((uint32_t *)&__RAM_EMU_START__, (size_t)&_OVERLAY_GW_SIZE);
        app_main_gw(load_state, save_slot);
#endif
    } else if(strcmp(emu->system_name, "PC Engine") == 0) {
#ifdef ENABLE_EMULATOR_PCE
      memcpy(&__RAM_EMU_START__, &_OVERLAY_PCE_LOAD_START, (size_t)&_OVERLAY_PCE_SIZE);
      memset(&_OVERLAY_PCE_BSS_START, 0x0, (size_t)&_OVERLAY_PCE_BSS_SIZE);
      SCB_CleanDCache_by_Addr((uint32_t *)&__RAM_EMU_START__, (size_t)&_OVERLAY_PCE_SIZE);
      app_main_pce(load_state, start_paused, save_slot);
#endif
    } else if(strcmp(emu->system_name, "MSX") == 0) {
#ifdef ENABLE_EMULATOR_MSX
      memcpy(&__RAM_EMU_START__, &_OVERLAY_MSX_LOAD_START, (size_t)&_OVERLAY_MSX_SIZE);
      memset(&_OVERLAY_MSX_BSS_START, 0x0, (size_t)&_OVERLAY_MSX_BSS_SIZE);
      SCB_CleanDCache_by_Addr((uint32_t *)&__RAM_EMU_START__, (size_t)&_OVERLAY_MSX_SIZE);
      app_main_msx(load_state, start_paused, save_slot);
#endif
    } else if(strcmp(emu->system_name, "Watara Supervision") == 0) {
#ifdef ENABLE_EMULATOR_WSV
      memcpy(&__RAM_EMU_START__, &_OVERLAY_WSV_LOAD_START, (size_t)&_OVERLAY_WSV_SIZE);
      memset(&_OVERLAY_WSV_BSS_START, 0x0, (size_t)&_OVERLAY_WSV_BSS_SIZE);
      SCB_CleanDCache_by_Addr((uint32_t *)&__RAM_EMU_START__, (size_t)&_OVERLAY_WSV_SIZE);
      app_main_wsv(load_state, start_paused, save_slot);
#endif
    } else if(strcmp(emu->system_name, "Sega Genesis") == 0)  {
 #ifdef ENABLE_EMULATOR_MD
      memcpy(&__RAM_EMU_START__, &_OVERLAY_MD_LOAD_START, (size_t)&_OVERLAY_MD_SIZE);
      memset(&_OVERLAY_MD_BSS_START, 0x0, (size_t)&_OVERLAY_MD_BSS_SIZE);
      SCB_CleanDCache_by_Addr((uint32_t *)&__RAM_EMU_START__, (size_t)&_OVERLAY_MD_SIZE);
      app_main_gwenesis(load_state, start_paused, save_slot);
 #endif
    } else if(strcmp(emu->system_name, "Atari 7800") == 0)  {
 #ifdef ENABLE_EMULATOR_A7800
      memcpy(&__RAM_EMU_START__, &_OVERLAY_A7800_LOAD_START, (size_t)&_OVERLAY_A7800_SIZE);
      memset(&_OVERLAY_A7800_BSS_START, 0x0, (size_t)&_OVERLAY_A7800_BSS_SIZE);
      SCB_CleanDCache_by_Addr((uint32_t *)&__RAM_EMU_START__, (size_t)&_OVERLAY_A7800_SIZE);
      app_main_a7800(load_state, start_paused, save_slot);
 #endif
    } else if(strcmp(emu->system_name, "Amstrad CPC") == 0)  {
 #ifdef ENABLE_EMULATOR_AMSTRAD
      memcpy(&__RAM_EMU_START__, &_OVERLAY_AMSTRAD_LOAD_START, (size_t)&_OVERLAY_AMSTRAD_SIZE);
      memset(&_OVERLAY_AMSTRAD_BSS_START, 0x0, (size_t)&_OVERLAY_AMSTRAD_BSS_SIZE);
      SCB_CleanDCache_by_Addr((uint32_t *)&__RAM_EMU_START__, (size_t)&_OVERLAY_AMSTRAD_SIZE);
      app_main_amstrad(load_state, start_paused, save_slot);
  #endif
    }
    
}

void emulators_init()
{
#if !( defined(ENABLE_EMULATOR_GB) || defined(ENABLE_EMULATOR_NES) || defined(ENABLE_EMULATOR_SMS) || defined(ENABLE_EMULATOR_GG) || defined(ENABLE_EMULATOR_COL) || defined(ENABLE_EMULATOR_SG1000) || defined(ENABLE_EMULATOR_PCE) || defined(ENABLE_EMULATOR_GW) || defined(ENABLE_EMULATOR_MSX) || defined(ENABLE_EMULATOR_WSV) || defined(ENABLE_EMULATOR_MD) || defined(ENABLE_EMULATOR_A7800) || defined(ENABLE_EMULATOR_AMSTRAD))
    // Add gameboy as a placeholder in case no emulator is built.
    add_emulator("Nintendo Gameboy", "gb", "gb", "gnuboy-go", 0, &pad_gb, &header_gb);
#endif


#ifdef ENABLE_EMULATOR_GB
    add_emulator("Nintendo Gameboy", "gb", "gb", "gnuboy-go", 0, &pad_gb, &header_gb);
    // add_emulator("Nintendo Gameboy Color", "gbc", "gbc", "gnuboy-go", 0, logo_gbc, header_gbc);
#endif

#ifdef ENABLE_EMULATOR_NES
    add_emulator("Nintendo Entertainment System", "nes", "nes", "nofrendo-go", 16, &pad_nes, &header_nes);
#endif
    
#ifdef ENABLE_EMULATOR_GW
    add_emulator("Game & Watch", "gw", "gw", "LCD-Game-Emulator", 0, &pad_gw, &header_gw);
#endif

#ifdef ENABLE_EMULATOR_PCE
    add_emulator("PC Engine", "pce", "pce", "pce-go", 0, &pad_pce, &header_pce);
#endif

#ifdef ENABLE_EMULATOR_GG
    add_emulator("Sega Game Gear", "gg", "gg", "smsplusgx-go", 0, &pad_gg, &header_gg);
#endif

#ifdef ENABLE_EMULATOR_SMS
    add_emulator("Sega Master System", "sms", "sms", "smsplusgx-go", 0, &pad_sms, &header_sms);
#endif

#ifdef ENABLE_EMULATOR_MD
    add_emulator("Sega Genesis", "md", "md", "GnWesis", 0, &pad_gen, &header_gen);
#endif

#ifdef ENABLE_EMULATOR_SG1000
    add_emulator("Sega SG-1000", "sg", "sg", "smsplusgx-go", 0, &pad_sg1000, &header_sg1000);
#endif

#ifdef ENABLE_EMULATOR_COL
    add_emulator("Colecovision", "col", "col", "smsplusgx-go", 0, &pad_col, &header_col);
#endif

#ifdef ENABLE_EMULATOR_MSX
    add_emulator("MSX", "msx", "msx", "blueMSX", 0, &pad_msx, &header_msx);
#endif

#ifdef ENABLE_EMULATOR_WSV
    add_emulator("Watara Supervision", "wsv", "wsv", "potator", 0, &pad_wsv, &header_wsv);
#endif

#ifdef ENABLE_EMULATOR_A7800
    add_emulator("Atari 7800", "a7800", "a7800", "prosystem-go", 0, &pad_a7800, &header_a7800);
#endif

#ifdef ENABLE_EMULATOR_AMSTRAD
    add_emulator("Amstrad CPC", "amstrad", "amstrad", "caprice32", 0, &pad_amstrad, &header_amstrad);
#endif

    // add_emulator("ColecoVision", "col", "col", "smsplusgx-go", 0, logo_col, header_col);
    // add_emulator("PC Engine", "pce", "pce", "huexpress-go", 0, logo_pce, header_pce);
    // add_emulator("Atari Lynx", "lnx", "lnx", "handy-go", 64, logo_lnx, header_lnx);
    // add_emulator("Atari 2600", "a26", "a26", "stella-go", 0, logo_a26, header_a26);
}

bool emulator_is_file_valid(retro_emulator_file_t *file)
{
    for (int i = 0; i < emulators_count; i++) {
        for (int j = 0; j < emulators[i].roms.count; j++) {
            if (&emulators[i].roms.files[j] == file) {
                return true;
            }
        }
    }

    return false;
}
