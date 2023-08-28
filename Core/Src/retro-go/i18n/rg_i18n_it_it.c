/*
*********************************************************
*                Warning!!!!!!!                         *
*  This file must be saved with Windows 1252 Encoding   *
*********************************************************
*/
#if !defined (INCLUDED_IT_IT)
#define INCLUDED_IT_IT 1
#endif
#if INCLUDED_IT_IT == 1
//#include "rg_i18n_lang.h"
// Stand Italian
// Created by SantX27, checked by MrPalloncini

int it_it_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, day, month, weekday, hour, minutes, seconds);
};

int it_it_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, day, month, year, weekday);
};

int it_it_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_it_it LANG_DATA = {
    .codepage = 1252,
    .extra_font = NULL,
    .s_LangUI = "Lingua",
    .s_LangName = "Italian",

    // Core\Src\porting\nes-fceu\main_nes_fceu.c ===========================
    .s_Crop_Vertical_Overscan = "Crop Vertical Overscan",
    .s_Crop_Horizontal_Overscan = "Crop Horizontal Overscan",
    .s_Disable_Sprite_Limit = "Disable sprite limit",
    .s_NES_CPU_OC = "NES CPU Overclocking",
    .s_NES_Eject_Insert_FDS = "Eject/Insert Disk",
    .s_NES_Eject_FDS = "Eject Disk",
    .s_NES_Insert_FDS = "Insert Disk",
    .s_NES_Swap_Side_FDS = "Swap FDisk side",
    .s_NES_FDS_Side_Format = "Disk %d Side %s",
    //=====================================================================

    // Core\Src\porting\gb\main_gb.c =======================================
    .s_Palette = "Palette",
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette = "Palette" dul
    .s_Default = "Predefinita",
    //=====================================================================

    // Core\Src\porting\md\main_gwenesis.c ================================
    .s_md_keydefine = "keys: A-B-C",
    .s_md_Synchro = "Synchro",
    .s_md_Synchro_Audio = "AUDIO",
    .s_md_Synchro_Vsync = "VSYNC",
    .s_md_Dithering = "Dithering",
    .s_md_Debug_bar = "Debug bar",
    .s_md_Option_ON = "\x6",
    .s_md_Option_OFF = "\x5",
    .s_md_AudioFilter = "Audio Filter",
    .s_md_VideoUpscaler = "Video Upscaler",
    //=====================================================================

    // Core\Src\porting\md\main_wsv.c ================================
    .s_wsv_palette_Default = "Default",
    .s_wsv_palette_Amber = "Amber",
    .s_wsv_palette_Green = "Green",
    .s_wsv_palette_Blue = "Blue",
    .s_wsv_palette_BGB = "BGB",
    .s_wsv_palette_Wataroo = "Wataroo",
    //=====================================================================

    // Core\Src\porting\md\main_msx.c ================================
    .s_msx_Change_Dsk = "Change Dsk",
    .s_msx_Select_MSX = "Select MSX",
    .s_msx_MSX1_EUR = "MSX1 (EUR)",
    .s_msx_MSX2_EUR = "MSX2 (EUR)",
    .s_msx_MSX2_JP = "MSX2+ (JP)",
    .s_msx_Frequency = "Frequency",
    .s_msx_Freq_Auto = "Auto",
    .s_msx_Freq_50 = "50Hz",
    .s_msx_Freq_60 = "60Hz",
    .s_msx_A_Button = "A Button",
    .s_msx_B_Button = "B Button",
    .s_msx_Press_Key = "Press Key",
    //=====================================================================

    // Core\Src\porting\md\main_amstrad.c ================================
    .s_amd_Change_Dsk = "Change Dsk",
    .s_amd_Controls = "Controls",
    .s_amd_Controls_Joystick = "Joystick",
    .s_amd_Controls_Keyboard = "Keyboard",
    .s_amd_palette_Color = "Color",
    .s_amd_palette_Green = "Green",
    .s_amd_palette_Grey = "Grey",
    .s_amd_Press_Key = "Press Key",
    //=====================================================================

    // Core\Src\porting\gw\main_gw.c =======================================
    .s_copy_RTC_to_GW_time = "Copia RTC sull'orologio G&W",
    .s_copy_GW_time_to_RTC = "Copia orario G&W sull'RTC",
    .s_LCD_filter = "Filtro LCD",
    .s_Display_RAM = "Mostra la RAM",
    .s_Press_ACL = "Premi ACL o Reset",
    .s_Press_TIME = "Premi TIME [B+TIME]",
    .s_Press_ALARM = "Premi ALARM [B+GAME]",
    .s_filter_0_none = "0-Nessuno",
    .s_filter_1_medium = "1-Medio",
    .s_filter_2_high = "2-Alto",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",
    .s_No_Cover = "Senza immagine",
    .s_Yes = "S�",
    .s_No = "No",
    .s_PlsChose = "Seleziona",
    .s_OK = "OK",
    .s_Confirm = "Conferma",
    .s_Brightness = "Luminosit�",
    .s_Volume = "Volume",
    .s_OptionsTit = "Opzioni",
    .s_FPS = "FPS",
    .s_BUSY = "OCCUPATO",
    .s_Scaling = "Scala",
    .s_SCalingOff = "Nessuno",
    .s_SCalingFit = "Adatta",
    .s_SCalingFull = "Sch. Intero",
    .s_SCalingCustom = "Personaliz.",
    .s_Filtering = "Filtro",
    .s_FilteringNone = "Nessuno",
    .s_FilteringOff = "Off",
    .s_FilteringSharp = "Nitido",
    .s_FilteringSoft = "Leggero",
    .s_Speed = "Velocit�",
    .s_Speed_Unit = "x",
    .s_Save_Cont = "Salva e Continua",
    .s_Save_Quit = "Salva ed Esci",
    .s_Reload = "Ricarica",
    .s_Options = "Opzioni",
    .s_Power_off = "Spegni",
    .s_Quit_to_menu = "Esci e torna al men�",
    .s_Retro_Go_options = "Retro-Go",
    .s_Font = "Carattere",
    .s_Colors = "Colori",
    .s_Theme_Title = "Temi UI",
    .s_Theme_sList = "Lista",
    .s_Theme_CoverV = "Galleria Vert",
    .s_Theme_CoverH = "Galleria Oriz",
    .s_Theme_CoverLightV = "Mini Vert",
    .s_Theme_CoverLightH = "Mini Oriz",
    //=====================================================================

    // Core\Src\retro-go\rg_emulators.c ====================================
    .s_File = "File",
    .s_Type = "Tipo",
    .s_Size = "Dimensione",
    .s_ImgSize = "Dimensione immagine",
    .s_Close = "Chiudi",
    .s_GameProp = "Propriet�",
    .s_Resume_game = "Riprendi gioco",
    .s_New_game = "Nuova partita",
    .s_Del_favorite = "Rimuovi dai preferiti",
    .s_Add_favorite = "Aggiungi ai preferiti",
    .s_Delete_save = "Elimina il salvataggio",
    .s_Confiem_del_save = "Eliminare il salvataggio?",
#if CHEAT_CODES == 1
    .s_Cheat_Codes = "Codici Cheat",
    .s_Cheat_Codes_Title = "Codici Cheat",
    .s_Cheat_Codes_ON = "\x6",
    .s_Cheat_Codes_OFF = "\x5",
#endif
    //=====================================================================

    // Core\Src\retro-go\rg_main.c =========================================
    .s_CPU_Overclock = "CPU Overclock",
    .s_CPU_Overclock_0 = "No",
    .s_CPU_Overclock_1 = "Intermediate",
    .s_CPU_Overclock_2 = "Maximum",
    .s_CPU_OC_Upgrade_to = "Upgrade to ",
    .s_CPU_OC_Downgrade_to = "Downgrade to ",
    .s_CPU_OC_Stay_at = "Stay at ",
    .s_Confirm_OC_Reboot = "CPU Overclock configuration has changed and needs to reboot now. Are you sure?",
#if INTFLASH_BANK == 2
    .s_Reboot = "Reboot",
    .s_Original_system = "Original system",
    .s_Confirm_Reboot = "Confirm reboot?",
#endif
    .s_Second_Unit = "s",
    .s_Version = "Ver.",
    .s_Author = "Di",
    .s_Author_ = "\t\t+",
    .s_UI_Mod = "Mod UI",
    .s_UI_Mod_ = "\t\t+",
    .s_Lang = "Italiano",
    .s_LangAuthor = "SantX27",
    .s_Debug_menu = "Men� di Debug",
    .s_Reset_settings = "Ripristina configurazione",
    .s_Retro_Go = "Riguardo Retro-Go",
    .s_Confirm_Reset_settings = "Ripristinare configurazione?",
    .s_Flash_JEDEC_ID = "ID Flash JEDEC",
    .s_Flash_Name = "Nome Flash",
    .s_Flash_SR = "SR Flash",
    .s_Flash_CR = "CR Flash",
    .s_Smallest_erase = "Eliminazione minima",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "Attiva DBGMCU CK",
    .s_Disable_DBGMCU_CK = "Disattiva DBGMCU CK",
    .s_Debug_Title = "Debug",
    .s_Idle_power_off = "Spegnimento automatico",
    .s_Time = "Ora",
    .s_Date = "Data",
    .s_Time_Title = "Tempo",
    .s_Hour = "Ora",
    .s_Minute = "Minuti",
    .s_Second = "Secondi",
    .s_Time_setup = "Conf. orario",
    .s_Day = "Giorno",
    .s_Month = "Mese",
    .s_Year = "Anno",
    .s_Weekday = "Giorno della settimana",
    .s_Date_setup = "Configura data",
    .s_Weekday_Mon = "Lun",
    .s_Weekday_Tue = "Mar",
    .s_Weekday_Wed = "Mer",
    .s_Weekday_Thu = "Gio",
    .s_Weekday_Fri = "Ven",
    .s_Weekday_Sat = "Sab",
    .s_Weekday_Sun = "Dom",
    .s_Turbo_Button = "Turbo",
    .s_Turbo_None = "None",
    .s_Turbo_A = "A",
    .s_Turbo_B = "B",
    .s_Turbo_AB = "A & B",
    .s_Title_Date_Format = "%02d-%02d %s %02d:%02d:%02d",
    .s_Date_Format = "%02d.%02d.20%02d %s",
    .s_Time_Format = "%02d:%02d:%02d",
    .fmt_Title_Date_Format = it_it_fmt_Title_Date_Format,
    .fmtDate = it_it_fmt_Date,
    .fmtTime = it_it_fmt_Time,
    //=====================================================================
    //           ------------ end ---------------
};

#endif
