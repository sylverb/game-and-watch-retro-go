#pragma once
#if !defined (CHEAT_CODES)
#define CHEAT_CODES 0
#endif
typedef struct
{
    const uint32_t codepage;
    const char **extra_font; 
    const char *s_LangUI;
    const char *s_LangName;  //used for English name
    // Core\Src\porting\nes-fceu\main_nes_fceu.c ===========================
    const char *s_Crop_Vertical_Overscan;
    const char *s_Crop_Horizontal_Overscan;
    const char *s_Disable_Sprite_Limit;
    const char *s_NES_CPU_OC;
    const char *s_NES_Eject_Insert_FDS;
    const char *s_NES_Eject_FDS;
    const char *s_NES_Insert_FDS;
    const char *s_NES_Swap_Side_FDS;
    const char *s_NES_FDS_Side_Format;
    //=====================================================================
    // Core\Src\porting\gb\main_gb.c =======================================
    const char *s_Palette;
    //=====================================================================
    // Core\Src\porting\nes\main_nes.c =====================================
    // const char *s_Palette "Palette" dul
    const char *s_Default;
    //=====================================================================

    // Core\Src\porting\md\main_gwenesis.c ================================
    const char *s_md_keydefine;
    const char *s_md_Synchro;
    const char *s_md_Synchro_Audio;
    const char *s_md_Synchro_Vsync;
    const char *s_md_Dithering;
    const char *s_md_Debug_bar;
    const char *s_md_Option_ON;
    const char *s_md_Option_OFF;
    const char *s_md_AudioFilter;
    const char *s_md_VideoUpscaler;
    //=====================================================================

    // Core\Src\porting\md\main_wsv.c ================================
    const char *s_wsv_palette_Default;
    const char *s_wsv_palette_Amber;
    const char *s_wsv_palette_Green;
    const char *s_wsv_palette_Blue;
    const char *s_wsv_palette_BGB;
    const char *s_wsv_palette_Wataroo;
    //=====================================================================

    // Core\Src\porting\md\main_msx.c ================================
    const char *s_msx_Change_Dsk;
    const char *s_msx_Select_MSX;
    const char *s_msx_MSX1_EUR;
    const char *s_msx_MSX2_EUR;
    const char *s_msx_MSX2_JP;
    const char *s_msx_Frequency;
    const char *s_msx_Freq_Auto;
    const char *s_msx_Freq_50;
    const char *s_msx_Freq_60;
    const char *s_msx_A_Button;
    const char *s_msx_B_Button;
    const char *s_msx_Press_Key;
    //=====================================================================

    // Core\Src\porting\md\main_amstrad.c ================================
    const char *s_amd_Change_Dsk;
    const char *s_amd_Controls;
    const char *s_amd_Controls_Joystick;
    const char *s_amd_Controls_Keyboard;
    const char *s_amd_palette_Color;
    const char *s_amd_palette_Green;
    const char *s_amd_palette_Grey;
    const char *s_amd_Press_Key;
    //=====================================================================

    // Core\Src\porting\gw\main_gw.c =======================================
    const char *s_copy_RTC_to_GW_time;
    const char *s_copy_GW_time_to_RTC;
    const char *s_LCD_filter;
    const char *s_Display_RAM;
    const char *s_Press_ACL;
    const char *s_Press_TIME;
    const char *s_Press_ALARM;
    const char *s_filter_0_none;
    const char *s_filter_1_medium;
    const char *s_filter_2_high;
    //=====================================================================
    // Core\Src\porting\odroid_overlay.c ===================================
    const char *s_Full;
    const char *s_Fill;
    const char *s_No_Cover;
    const char *s_Yes;
    const char *s_No;
    const char *s_PlsChose;
    const char *s_OK;
    const char *s_Confirm;
    const char *s_Brightness;
    const char *s_Volume;
    const char *s_OptionsTit;
    const char *s_FPS;
    const char *s_BUSY;
    const char *s_Scaling;
    const char *s_SCalingOff;
    const char *s_SCalingFit;
    const char *s_SCalingFull;
    const char *s_SCalingCustom;
    const char *s_Filtering;
    const char *s_FilteringNone;
    const char *s_FilteringOff;
    const char *s_FilteringSharp;
    const char *s_FilteringSoft;
    const char *s_Speed;
    const char *s_Speed_Unit;
    const char *s_Save_Cont;
    const char *s_Save_Quit;
    const char *s_Reload;
    const char *s_Options;
    const char *s_Power_off;
    const char *s_Quit_to_menu;
    const char *s_Retro_Go_options;
    const char *s_Font;
    const char *s_Colors;
    const char *s_Theme_Title;
    const char *s_Theme_sList;
    const char *s_Theme_CoverV;
    const char *s_Theme_CoverH;
    const char *s_Theme_CoverLightV;
    const char *s_Theme_CoverLightH;
    //=====================================================================
    // Core\Src\retro-go\rg_emulators.c ====================================
    const char *s_File;
    const char *s_Type;
    const char *s_Size;
    const char *s_ImgSize;
    const char *s_Close;
    const char *s_GameProp;
    const char *s_Resume_game;
    const char *s_New_game;
    const char *s_Del_favorite;
    const char *s_Add_favorite;
    const char *s_Delete_save;
    const char *s_Confiem_del_save;
#if CHEAT_CODES == 1
    const char *s_Cheat_Codes;
    const char *s_Cheat_Codes_Title;
    const char *s_Cheat_Codes_ON;
    const char *s_Cheat_Codes_OFF;
#endif    
    //=====================================================================
    // Core\Src\retro-go\rg_main.c =========================================
    const char *s_CPU_Overclock;
    const char *s_CPU_Overclock_0;
    const char *s_CPU_Overclock_1;
    const char *s_CPU_Overclock_2;
    const char *s_CPU_OC_Upgrade_to;
    const char *s_CPU_OC_Downgrade_to;
    const char *s_CPU_OC_Stay_at;
    const char *s_Confirm_OC_Reboot;
#if INTFLASH_BANK == 2
    const char *s_Reboot;
    const char *s_Original_system;
    const char *s_Confirm_Reboot;
#endif
    const char *s_Second_Unit;
    const char *s_Version;
    const char *s_Author;
    const char *s_Author_;
    const char *s_UI_Mod;
    const char *s_UI_Mod_;
    const char *s_Lang;
    const char *s_LangAuthor;
    const char *s_Debug_menu;
    const char *s_Reset_settings;
    const char *s_Retro_Go;
    const char *s_Confirm_Reset_settings;
    const char *s_Flash_JEDEC_ID;
    const char *s_Flash_Name;
    const char *s_Flash_SR;
    const char *s_Flash_CR;
    const char *s_Smallest_erase;
    const char *s_DBGMCU_IDCODE;
    const char *s_DBGMCU_CR;
    const char *s_DBGMCU_clock;
    const char *s_DBGMCU_clock_on;
    const char *s_DBGMCU_clock_auto;
    const char *s_Debug_Title;
    const char *s_Idle_power_off;
    const char *s_Time;
    const char *s_Date;
    const char *s_Time_Title;
    const char *s_Hour;
    const char *s_Minute;
    const char *s_Second;
    const char *s_Time_setup;
    const char *s_Day;
    const char *s_Month;
    const char *s_Year;
    const char *s_Weekday;
    const char *s_Date_setup;
    const char *s_Weekday_Mon;
    const char *s_Weekday_Tue;
    const char *s_Weekday_Wed;
    const char *s_Weekday_Thu;
    const char *s_Weekday_Fri;
    const char *s_Weekday_Sat;
    const char *s_Weekday_Sun;
    const char *s_Title_Date_Format;
    const char *s_Date_Format;
    const char *s_Time_Format;
    const char *s_Turbo_Button;
    const char *s_Turbo_None;
    const char *s_Turbo_A;
    const char *s_Turbo_B;
    const char *s_Turbo_AB;

    const int (*fmt_Title_Date_Format)(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds);
    // const char *fmt_Title_Date_Format(outstr,datefmt,day,month,weekday,hour,minutes,seconds) sprintf(outstr,datefmt,day,month,weekday,hour,minutes,seconds)
    const int (*fmtDate)(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday);
    // const char *fmtDate(outstr,datefmt,day,month,year,weekday) sprintf(outstr,datefmt,day,month,year,weekday)
    const int (*fmtTime)(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds);
    //=====================================================================
    //           ------------ end ---------------
} lang_t;
