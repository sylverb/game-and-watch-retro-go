/*
*********************************************************
*                Warning!!!!!!!                         *
*  This file must be saved with Windows 1252 Encoding   *
*********************************************************
*/
#if !defined (INCLUDED_FR_FR)
#define INCLUDED_FR_FR 1
#endif
#if INCLUDED_FR_FR == 1
//#include "rg_i18n_lang.h"
// Stand French

int fr_fr_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, day, month, weekday, hour, minutes, seconds);
};

int fr_fr_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, day, month, year, weekday);
};

int fr_fr_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_fr_fr LANG_DATA = {
    .codepage = 1252,
    .extra_font = NULL,
    .s_LangUI = "Langue",
    .s_LangName = "French",

    // Core\Src\porting\nes-fceu\main_nes_fceu.c ===========================
    .s_Crop_Vertical_Overscan = "Recadrage Vertical",
    .s_Crop_Horizontal_Overscan = "Recadrage Horizontal",
    .s_Disable_Sprite_Limit = "Désactiver limit. nb sprites",
    .s_NES_CPU_OC = "Overclocking du CPU NES",
    .s_NES_Eject_Insert_FDS = "Ejecter/Insérer le disque",
    .s_NES_Eject_FDS = "Ejecter Disque",
    .s_NES_Insert_FDS = "Insérer Disque",
    .s_NES_Swap_Side_FDS = "Changer la face du disque",
    .s_NES_FDS_Side_Format = "Disque %d Face %s",
    //=====================================================================

    // Core\Src\porting\gb\main_gb.c =======================================
    .s_Palette = "Palette",
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette = "Palette" dul
    .s_Default = "Par défaut",
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
    .s_copy_RTC_to_GW_time = "Copie RTC vers horloge G&W",
    .s_copy_GW_time_to_RTC = "Copie temps G&W vers horloge RTC",
    .s_LCD_filter = "Filtre LCD",
    .s_Display_RAM = "Montrer la RAM",
    .s_Press_ACL = "Presser ACL ou Reset",
    .s_Press_TIME = "Presser TIME [B+TIME]",
    .s_Press_ALARM = "Presser ALARM [B+GAME]",
    .s_filter_0_none = "0-aucun",
    .s_filter_1_medium = "1-moyen",
    .s_filter_2_high = "2-élevé",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",

    .s_No_Cover = "Pas d'image",

    .s_Yes = "Oui",
    .s_No = "Non",
    .s_PlsChose = "Question",
    .s_OK = "OK",
    .s_Confirm = "Confirmer",
    .s_Brightness = "Luminosité",
    .s_Volume = "Volume",
    .s_OptionsTit = "Options",
    .s_FPS = "FPS",
    .s_BUSY = "Occupé",
    .s_Scaling = "Echelle",
    .s_SCalingOff = "Off",
    .s_SCalingFit = "Adaptée",
    .s_SCalingFull = "Complete",
    .s_SCalingCustom = "Personalisée",
    .s_Filtering = "Filtrage",
    .s_FilteringNone = "Aucun",
    .s_FilteringOff = "Off",
    .s_FilteringSharp = "Précis",
    .s_FilteringSoft = "Léger",
    .s_Speed = "Vitesse",
    .s_Speed_Unit = "x",
    .s_Save_Cont = "Sauver & Continuer",
    .s_Save_Quit = "Sauver & Quitter",
    .s_Reload = "Recharger",
    .s_Options = "Options",
    .s_Power_off = "Eteindre",
    .s_Quit_to_menu = "Quitter vers le menu",
    .s_Retro_Go_options = "Retro-Go",

    .s_Font = "Polices",
    .s_Colors = "Couleurs",
    .s_Theme_Title = "Theme UI",
    .s_Theme_sList = "Liste seule",
    .s_Theme_CoverV = "Galerie V",
    .s_Theme_CoverH = "Galerie H",
    .s_Theme_CoverLightV = "Mix V",
    .s_Theme_CoverLightH = "Mix H",
    //=====================================================================

    // Core\Src\retro-go\rg_emulators.c ====================================

    .s_File = "Fichier",
    .s_Type = "Type",
    .s_Size = "Taille",
    .s_ImgSize = "Taille image",
    .s_Close = "Fermer",
    .s_GameProp = "Propriétés",
    .s_Resume_game = "Reprendre le jeu",
    .s_New_game = "Nouvelle partie",
    .s_Del_favorite = "Retirer des favoris",
    .s_Add_favorite = "Ajouter aux favoris",
    .s_Delete_save = "Supprimer la sauvegarde",
    .s_Confiem_del_save = "Supprimer la sauvegarde ?",
#if CHEAT_CODES == 1
    .s_Cheat_Codes = "Codes de triche",
    .s_Cheat_Codes_Title = "Options de triche",
    .s_Cheat_Codes_ON = "\x6",
    .s_Cheat_Codes_OFF = "\x5",
#endif

    //=====================================================================

    // Core\Src\retro-go\rg_main.c =========================================
    .s_CPU_Overclock = "Overclocking du CPU",
    .s_CPU_Overclock_0 = "Sans",
    .s_CPU_Overclock_1 = "Moyen",
    .s_CPU_Overclock_2 = "Maximum",
    .s_CPU_OC_Upgrade_to = "Augmenter à ",
    .s_CPU_OC_Downgrade_to = "Diminuer à ",
    .s_CPU_OC_Stay_at = "",
    .s_Confirm_OC_Reboot = "Un redémarrage est nécessaire pour appliquer le nouveau réglage de l'overclocking du CPU. Etes-vous sûr ?",
//    .s_Confirm_OC_Reboot = "CPU Overclock configuration has changed and needs to reboot now. Are you sure?",
#if INTFLASH_BANK == 2
    .s_Reboot = "Reboot",
    .s_Original_system = "Original system",
    .s_Confirm_Reboot = "Confirm reboot?",
#endif
    .s_Second_Unit = "s",
    .s_Version = "Ver.",
    .s_Author = "De",
    .s_Author_ = "\t\t+",
    .s_UI_Mod = "UI Mod",
    .s_Lang = "Français",
    .s_LangAuthor = "Narkoa",
    .s_Debug_menu = "Menu Debug",
    .s_Reset_settings = "Restaurer les paramètres",
    //.s_Close                   = "Fermer",
    .s_Retro_Go = "A propos de Retro-Go",
    .s_Confirm_Reset_settings = "Restaurer les paramètres ?",

    .s_Flash_JEDEC_ID = "Id Flash JEDEC",
    .s_Flash_Name = "Nom Flash",
    .s_Flash_SR = "SR Flash",
    .s_Flash_CR = "CR Flash",
    .s_Smallest_erase = "Plus petite suppression",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "Activer DBGMCU CK",
    .s_Disable_DBGMCU_CK = "Désactiver DBGMCU CK",
    //.s_Close                   = "Fermer",
    .s_Debug_Title = "Debug",
    .s_Idle_power_off = "Temps avant veille",

    .s_Time = "Heure",
    .s_Date = "Date",
    .s_Time_Title = "TEMPS",
    .s_Hour = "Heure",
    .s_Minute = "Minute",
    .s_Second = "Seconde",
    .s_Time_setup = "Réglage",

    .s_Day = "Jour",
    .s_Month = "Mois",
    .s_Year = "Année",
    .s_Weekday = "Jour de la semaine",
    .s_Date_setup = "Réglage Date",

    .s_Weekday_Mon = "Lun",
    .s_Weekday_Tue = "Mar",
    .s_Weekday_Wed = "Mer",
    .s_Weekday_Thu = "Jeu",
    .s_Weekday_Fri = "Ven",
    .s_Weekday_Sat = "Sam",
    .s_Weekday_Sun = "Dim",
    .s_Turbo_Button = "Turbo",
    .s_Turbo_None = "Aucun",
    .s_Turbo_A = "A",
    .s_Turbo_B = "B",
    .s_Turbo_AB = "A & B",


    .s_Title_Date_Format = "%02d-%02d %s %02d:%02d:%02d",
    .s_Date_Format = "%02d.%02d.20%02d %s",
    .s_Time_Format = "%02d:%02d:%02d",

    .fmt_Title_Date_Format = fr_fr_fmt_Title_Date_Format,
    .fmtDate = fr_fr_fmt_Date,
    .fmtTime = fr_fr_fmt_Time,
    //=====================================================================
    //           ------------ end ---------------
};

#endif
