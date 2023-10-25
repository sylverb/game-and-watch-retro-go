/*
*********************************************************
*                Warning!!!!!!!                         *
*  This file must be saved with Windows 1252 Encoding   *
*********************************************************
*/
#if !defined (INCLUDED_DE_DE)
#define INCLUDED_DE_DE 1
#endif
#if INCLUDED_DE_DE == 1
//#include "rg_i18n_lang.h"
// Stand German


int de_de_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, weekday, day, month, hour, minutes, seconds);
};

int de_de_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, weekday, day, month, year);
};

int de_de_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_de_de LANG_DATA = {
    .codepage = 1252,
    .extra_font = NULL,
    .s_LangUI = "Sprache",
    .s_LangName = "Deutsch",

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
    .s_Default = "Standard",
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
    .s_copy_RTC_to_GW_time = "RTC -> G&W Zeit",
    .s_copy_GW_time_to_RTC = "G&W Zeit -> RTC",
    .s_LCD_filter = "LCD filter",
    .s_Display_RAM = "Anzeigespeicher",
    .s_Press_ACL = "Drücke ACL oder Reset",
    .s_Press_TIME = "Drücke TIME [B+TIME]",
    .s_Press_ALARM = "Drücke ALARM [B+GAME]",
    .s_filter_0_none = "0-kein",
    .s_filter_1_medium = "1-mittel",
    .s_filter_2_high = "2-hoch",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",
    .s_No_Cover = "kein Cover",
    .s_Yes = "Ja",
    .s_No = "Nein",
    .s_PlsChose = "Auswahl",
    .s_OK = "OK",
    .s_Confirm = "Bestätige",
    .s_Brightness = "Helligkeit",
    .s_Volume = "Lautstärke",
    .s_OptionsTit = "Optionen",
    .s_FPS = "FPS",
    .s_BUSY = "ausgelastet",
    .s_Scaling = "Skalierung",
    .s_SCalingOff = "aus",
    .s_SCalingFit = "einpassen",
    .s_SCalingFull = "Vollbild",
    .s_SCalingCustom = "benutzerdef.",
    .s_Filtering = "Filterung",
    .s_FilteringNone = "keine",
    .s_FilteringOff = "aus",
    .s_FilteringSharp = "scharf",
    .s_FilteringSoft = "weich",
    .s_Speed = "Geschwindigkeit",
    .s_Speed_Unit = "x",
    .s_Save_Cont = "Speichern & fortsetzen",
    .s_Save_Quit = "Speichern & beenden",
    .s_Reload = "Neu laden",
    .s_Options = "Optionen",
    .s_Power_off = "Abschalten",
    .s_Quit_to_menu = "Verlassen (Hautptmenü)",
    .s_Retro_Go_options = "Retro-Go",
    .s_Font = "Schriftart",
    .s_Colors = "Farben",
    .s_Theme_Title = "UI Darstellung",
    .s_Theme_sList = "Einfache Liste",
    .s_Theme_CoverV = "Coverflow V",
    .s_Theme_CoverH = "Coverflow H",
    .s_Theme_CoverLightV = "CoverLight V",
    .s_Theme_CoverLightH = "CoverLight H",
    //=====================================================================

    // Core\Src\retro-go\rg_emulators.c ====================================
    .s_File = "Datei",
    .s_Type = "Typ",
    .s_Size = "Größe",
    .s_ImgSize = "Bildgröße",
    .s_Close = "Schließen",
    .s_GameProp = "Eigenschaften",
    .s_Resume_game = "Spiel fortsetzen",
    .s_New_game = "Neues Spiel",
    .s_Del_favorite = "Favorit löschen",
    .s_Add_favorite = "Favorit hinzufügen",
    .s_Delete_save = "Spielstand löschen",
    .s_Confiem_del_save = "Spielstand wirklich löschen?",
#if CHEAT_CODES == 1
    .s_Cheat_Codes = "Cheat Codes",
    .s_Cheat_Codes_Title = "Cheat Options",
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
    .s_Confirm_OC_Reboot = "CPU Overclock configuration has changed and needs to be reboot. Are you sure?",
#if INTFLASH_BANK == 2
    .s_Reboot = "Reboot",
    .s_Original_system = "Original system",
    .s_Confirm_Reboot = "Confirm reboot?",
#endif
    .s_Second_Unit = "s",
    .s_Version = "Ver.",
    .s_Author = "von",
    .s_Author_ = "\t\t+",
    .s_UI_Mod = "UI Mod",
    .s_Lang = "Deutsch",
    .s_LangAuthor = "LeZerb",
    .s_Debug_menu = "Debug Menü",
    .s_Reset_settings = "Einstellungen zurücksetzen",
    .s_Retro_Go = "Über Retro-Go",
    .s_Confirm_Reset_settings = "Alle Einstellungen zurücksetzen?",
    .s_Flash_JEDEC_ID = "Flash JEDEC ID",
    .s_Flash_Name = "Flash Name",
    .s_Flash_SR = "Flash SR",
    .s_Flash_CR = "Flash CR",
    .s_Smallest_erase = "Smallest erase",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
     .s_DBGMCU_CR = "DBGMCU CR",
    .s_DBGMCU_clock = "DBGMCU Clock",
    .s_DBGMCU_clock_on = "On",
    .s_DBGMCU_clock_auto = "Auto",
    .s_Debug_Title = "Debug",
    .s_Idle_power_off = "Abschalten bei Leerlauf",
    .s_Time = "Zeit",
    .s_Date = "Datum",
    .s_Time_Title = "Zeit",
    .s_Hour = "Stunde",
    .s_Minute = "Minute",
    .s_Second = "Sekunde",
    .s_Time_setup = "Zeit setzen",
    .s_Day = "Tag",
    .s_Month = "Monat",
    .s_Year = "Jahr",
    .s_Weekday = "Wochentag",
    .s_Date_setup = "Datum setzen",
    .s_Weekday_Mon = "Mon",
    .s_Weekday_Tue = "Die",
    .s_Weekday_Wed = "Mit",
    .s_Weekday_Thu = "Don",
    .s_Weekday_Fri = "Fre",
    .s_Weekday_Sat = "Sam",
    .s_Weekday_Sun = "Son",
    .s_Turbo_Button = "Turbo",
    .s_Turbo_None = "None",
    .s_Turbo_A = "A",
    .s_Turbo_B = "B",
    .s_Turbo_AB = "A & B",
    .s_Title_Date_Format = "%s %02d.%02d. %02d:%02d:%02d",
    .s_Date_Format = "%s der %02d.%02d.20%02d",
    .s_Time_Format = "%02d:%02d:%02d",
    .fmt_Title_Date_Format = de_de_fmt_Title_Date_Format,
    .fmtDate = de_de_fmt_Date,
    .fmtTime = de_de_fmt_Time,
    //=====================================================================
    //           ------------ end ---------------
};

#endif
