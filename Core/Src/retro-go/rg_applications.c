#include <odroid_system.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "rg_emulators.h"
#include "rg_applications.h"
#include "bitmaps.h"
#include "gui.h"
#include "filesystem.h"
#include "main.h"
#include "gw_lcd.h"

static application_t *applications;
static int applications_count = 0;
static tab_t *apps_tab;

static void applications_load();

static void event_handler(gui_event_t event, tab_t *tab)
{
    listbox_item_t *item = gui_get_selected_item(tab);
    retro_emulator_file_t *file = (retro_emulator_file_t *)(item ? item->arg : NULL);

    if (event == TAB_INIT)
    {
        applications_load();
    }

    if (file == NULL)
        return;

    if (event == KEY_PRESS_A)
    {
        // Turn screen off before loading into framebuffer
        lcd_backlight_off();
        
        // Load file into RAM (override framebuffer1 at beginning of RAM region)
        char* filepath = file->name;
        printf("Loading application file from FS to RAM: filepath=%s buffer=0x%08x size=%d\n", filepath, &framebuffer1, file->size);
        fs_file_t *fd;
        fd = fs_open(filepath, FS_READ, FS_COMPRESS);
        fs_read(fd, &framebuffer1, /*file->size*/ sizeof(framebuffer1) + sizeof(framebuffer2)); // Max uncompressed filesize is 307200B
        fs_close(fd);
        
        // Reset and boot application in RAM
        boot_magic_set(BOOT_MAGIC_BOOT2RAM);
        HAL_NVIC_SystemReset();
    }
}


// Read application from filesystem
static void applications_load()
{
    applications_count = 0;
    if (applications != NULL) {
        free(applications);
    }
    
    // Count files in /apps directory
    printf("Counting regular files in FS dir %s...\n", APPS_FOLDER);
    if (fs_exists(APPS_FOLDER)) {
        fs_dir_t dir;
        fs_dir_open(APPS_FOLDER, &dir);
        fs_info_t info;
        while(fs_dir_read(&dir, &info)) {
            if (info.type == FS_TYPE_REG) {
                printf("Regular file in FS dir %s: %d\n", APPS_FOLDER, info.size);
                applications_count++;
            }
        }
        fs_dir_close(&dir);

        applications = calloc(applications_count + 1, sizeof(application_t));
        gui_resize_list(apps_tab, applications_count);

        // List files in /apps directory
        printf("Looking for regular files in FS dir %s...\n", APPS_FOLDER);
        fs_dir_open(APPS_FOLDER, &dir);
        int pos = 0;
        while (fs_dir_read(&dir, &info)) {
            if (info.type == FS_TYPE_REG) {
                printf("Regular file in FS dir %s: %d\n", APPS_FOLDER, info.size);

                application_t *application = &applications[pos];

                memcpy(application->name, info.name, 16);
                snprintf(application->path, sizeof(application->path), "%s/%s", APPS_FOLDER, application->name);
                retro_emulator_file_t* file = &application->file;
                memset(file, 0, sizeof(retro_emulator_file_t));
                file->name = application->path;
                file->size = info.size;
                apps_tab->listbox.items[pos].text = application->name;
                apps_tab->listbox.items[pos].arg = &application->file;
                pos++;
            }
        }
        fs_dir_close(&dir);
    }

    if (applications_count > 0)
    {
        sprintf(apps_tab->status, "Applications: %d", applications_count);
        gui_resize_list(apps_tab, applications_count);
        gui_sort_list(apps_tab, 0);
        apps_tab->is_empty = false;
    }
    else
    {
        sprintf(apps_tab->status, "No applications");
        gui_resize_list(apps_tab, 6);
        apps_tab->listbox.items[0].text = "Welcome to Retro-Go!";
        apps_tab->listbox.items[2].text = "You have no applications.";
        apps_tab->listbox.items[4].text = "Use LEFT and RIGHT to navigate.";
        apps_tab->listbox.cursor = 3;
        apps_tab->is_empty = true;
    }
}

void applications_init()
{
    apps_tab = gui_add_tab("applications", &pad_nes, NULL, NULL, event_handler);
}
