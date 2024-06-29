#include <odroid_system.h>

#include "main.h"
#include "gui.h"
#include "rg_i18n.h"
#include "filesystem.h"

#define MAX_SAVE_ENTRIES_COUNT 8

static void show_saves(char *path, int offset) {
    odroid_dialog_choice_t  saveinfo[50];
    fs_folder_entry         entry[20];
    fs_folder_entry         entry_save;
    int index;
    int count;
    int entry_index;
    bool read_folder_status;

    while (offset >= 0) {
        index=0;
        count=0;
        entry_index=0;
        if (fs_dir_open(0, path) >= 0) {
            while(((read_folder_status = fs_dir_read(0, &entry[entry_index])) == 1) &&
                  (entry_index<MAX_SAVE_ENTRIES_COUNT))
            {
                if ((strcmp(".",entry[entry_index].name) != 0) &&
                    (strcmp("..",entry[entry_index].name) != 0)) {
                    if (entry[entry_index].is_folder) {
                        char save_path[100];
                        entry[entry_index].has_savestate = false;
                        entry[entry_index].has_sram = false;

                        sprintf(save_path,"%s/%s",path,entry[entry_index].name);
                        // Check content of folder
                        fs_dir_open(1, save_path);
                        while((read_folder_status = fs_dir_read(1, &entry_save)) == 1) {
                            if (strcmp(entry_save.name,"0") == 0) {
                                entry[entry_index].has_savestate = true;
                            } else if (strcmp(entry_save.name,"0.srm") == 0) {
                                entry[entry_index].has_sram = true;
                            }
                        }
                        fs_dir_close(1);

                        if (entry[entry_index].has_savestate ||
                            entry[entry_index].has_sram)
                        {
                            saveinfo[index].id = index;
                            saveinfo[index].label = entry[entry_index].name;
                            saveinfo[index].value = entry[entry_index].is_folder?NULL:entry[entry_index].size;
                            saveinfo[index].enabled = 1;
                            saveinfo[index].update_cb = NULL;
                            count++;
                            if (count > offset) {
                                index++;
                                entry_index++;
                            }
                        }
                    }
                }
            }
            fs_dir_close(0);

            if (offset != 0) {
                // We can go to previous items
                saveinfo[index].id = 0x0F0F0F0E;
                saveinfo[index].label = "-";
                saveinfo[index].value = "-";
                saveinfo[index].enabled = -1;
                saveinfo[index++].update_cb = NULL;

                saveinfo[index].id = -3;
                saveinfo[index].label = curr_lang->s_Previous;
                saveinfo[index].value = "";
                saveinfo[index].enabled = 1;
                saveinfo[index++].update_cb = NULL;
            }

            if (read_folder_status) {
                // There are some folder left
                saveinfo[index].id = 0x0F0F0F0E;
                saveinfo[index].label = "-";
                saveinfo[index].value = "-";
                saveinfo[index].enabled = -1;
                saveinfo[index++].update_cb = NULL;

                saveinfo[index].id = -2;
                saveinfo[index].label = curr_lang->s_Next;
                saveinfo[index].value = "";
                saveinfo[index].enabled = 1;
                saveinfo[index++].update_cb = NULL;
            }

            saveinfo[index].id = 0x0F0F0F0E;
            saveinfo[index].label = "-";
            saveinfo[index].value = "-";
            saveinfo[index].enabled = -1;
            saveinfo[index++].update_cb = NULL;

            saveinfo[index].id = -1;
            saveinfo[index].label = curr_lang->s_Close;
            saveinfo[index].value = "";
            saveinfo[index].enabled = 1;
            saveinfo[index++].update_cb = NULL;

            saveinfo[index].id = 0x0F0F0F0F;
            saveinfo[index].label = "LAST";
            saveinfo[index].value = "LAST";
            saveinfo[index].enabled = 0xFFFF;
            saveinfo[index++].update_cb = NULL;

            int sel = odroid_overlay_dialog(curr_lang->s_Save_manager, saveinfo, 0, &gui_redraw_callback);
            if (sel >= 0) {
                if (entry[sel].has_savestate) {
                    if (odroid_overlay_confirm(curr_lang->s_Confirm_del_save, false, &gui_redraw_callback) == 1) {
                        char save_path[FS_MAX_PATH_SIZE];
                        char gnw_data_path[FS_MAX_PATH_SIZE];
                        sprintf(save_path,"%s/%s/0",path,entry[sel].name);
                        sprintf(gnw_data_path,"%s/%s/0.gnw",path,entry[sel].name);
                        fs_delete(save_path);
                        fs_delete(gnw_data_path);
                    }
                }
                if (entry[sel].has_sram) {
                    if (odroid_overlay_confirm(curr_lang->s_Confirm_del_sram, false, &gui_redraw_callback) == 1) {
                        char save_path[FS_MAX_PATH_SIZE];
                        sprintf(save_path,"%s/%s/0.srm",path,entry[sel].name);
                        fs_delete(save_path);
                    }
                }
            } else if (sel == -2) {
                offset += MAX_SAVE_ENTRIES_COUNT;
            } else if (sel == -3) {
                offset -= MAX_SAVE_ENTRIES_COUNT;
            } else if (sel == -1) { // Close
                offset = -1;
            }
        } else {
            // Missing folder = no savestate
            offset = -1;
        }
    }
}

void handle_save_manager_menu()
{
    tab_t *tab = gui_get_current_tab();
    char path[74];
    sprintf(path, "savestate/%s", tab->name);

    show_saves(path, 0);
}
