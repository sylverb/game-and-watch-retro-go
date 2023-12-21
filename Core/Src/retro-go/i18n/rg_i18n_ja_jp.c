/*
***************************************************
*                Warning!!!!!!!                   *
*  This file must be saved with cp932  Encoding   *
***************************************************
*/
#if !defined (INCLUDED_JA_JP)
#define INCLUDED_JA_JP 0
#endif
#if INCLUDED_JA_JP==1
//#include "rg_i18n_lang.h"
// Jp lang

int ja_jp_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, day, month, weekday, hour, minutes, seconds);
};

int ja_jp_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, day, month, year, weekday);
};

int ja_jp_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_ja_jp LANG_DATA = {
    .codepage = 932,
    .extra_font = ja_jp_fonts,
    .s_LangUI = "����",
    .s_LangName = "Japanese",
    // Core\Src\porting\gb\main_gb.c =======================================
    .s_Palette = "�p���b�g",
    //=====================================================================

    // Core\Src\porting\nes-fceu\main_nes_fceu.c ===========================
    .s_Crop_Vertical_Overscan = "�c�̃I�[�o�[�X�L�����폜",
    .s_Crop_Horizontal_Overscan = "���̃I�[�o�[�X�L�����폜",
    .s_Disable_Sprite_Limit = "�X�v���C�g���~�b�g����",
    .s_NES_CPU_OC = "NES CPU�̃I�[�o�[�N���b�N",
    .s_NES_Eject_Insert_FDS = "�f�B�X�N�̎��o���E�}��",
    .s_NES_Eject_FDS = "�f�B�X�N���o��",
    .s_NES_Insert_FDS = "�f�B�X�N�}��",
    .s_NES_Swap_Side_FDS = "�f�B�X�N�T�C�h����",
    .s_NES_FDS_Side_Format = "�f�B�X�N %d - %s ��",
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette = "�p���b�g" dul
    .s_Default = "�W��",
    //=====================================================================

    // Core\Src\porting\md\main_gwenesis.c ================================
    .s_md_keydefine = "keys: A-B-C",
    .s_md_Synchro = "�V���N��",
    .s_md_Synchro_Audio = "�I�[�f�B�I",
    .s_md_Synchro_Vsync = "VSYNC",
    .s_md_Dithering = "�f�B�U�����O",
    .s_md_Debug_bar = "�f�o�b�O�o�[",
    .s_md_Option_ON = "\x6",
    .s_md_Option_OFF = "\x5",
    .s_md_AudioFilter = "�I�[�f�B�I�t�B���^�[",
    .s_md_VideoUpscaler = "�r�f�I�A�b�v�X�P�[��",
    //=====================================================================
    
    // Core\Src\porting\md\main_wsv.c ================================
    .s_wsv_palette_Default = "�W��",
    .s_wsv_palette_Amber = "����",
    .s_wsv_palette_Green = "��",
    .s_wsv_palette_Blue = "��",
    .s_wsv_palette_BGB = "BGB",
    .s_wsv_palette_Wataroo = "�p���b�g",
    //=====================================================================

    // Core\Src\porting\md\main_msx.c ================================
    .s_msx_Change_Dsk = "Dsk����",
    .s_msx_Select_MSX = "MSX�I��",
    .s_msx_MSX1_EUR = "MSX1(EUR)",
    .s_msx_MSX2_EUR = "MSX2(EUR)",
    .s_msx_MSX2_JP = "MSX2+(���{)",
    .s_msx_Frequency = "���g��",
    .s_msx_Freq_Auto = "����",
    .s_msx_Freq_50 = "50Hz",
    .s_msx_Freq_60 = "60Hz",
    .s_msx_A_Button = "A �{�^��",
    .s_msx_B_Button = "B �{�^��",
    .s_msx_Press_Key = "�L�[������",
    //=====================================================================

    // Core\Src\porting\md\main_amstrad.c ================================
    .s_amd_Change_Dsk = "Dsk����",
    .s_amd_Controls = "�R���g���[��",
    .s_amd_Controls_Joystick = "�W���C�X�e�B�b�N",
    .s_amd_Controls_Keyboard = "�L�[�{�[�h",
    .s_amd_palette_Color = "�F",
    .s_amd_palette_Green = "��",
    .s_amd_palette_Grey = "�D�F",
    .s_amd_game_Button = "Game �{�^��",
    .s_amd_time_Button = "Time �{�^��",
    .s_amd_start_Button = "Start �{�^��",
    .s_amd_select_Button = "Select �{�^��",
    .s_amd_A_Button = "A �{�^��",
    .s_amd_B_Button = "B �{�^��",
    .s_amd_Press_Key = "�L�[������",
    //=====================================================================

    // Core\Src\porting\gw\main_gw.c =======================================
    .s_copy_RTC_to_GW_time = "RTC��G&W�����ɃR�s�[",
    .s_copy_GW_time_to_RTC = "G&W������RTC�ɃR�s�[",
    .s_LCD_filter = "LCD�t�B���^�[",
    .s_Display_RAM = "RAM��\��",
    .s_Press_ACL = "ACL�����������Z�b�g",
    .s_Press_TIME = "TIME������ [B+TIME]",
    .s_Press_ALARM = "ALARM������ [B+GAME]",
    .s_filter_0_none = "0-�Ȃ�",
    .s_filter_1_medium = "1-��",
    .s_filter_2_high = "2-��",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",

    .s_No_Cover = "�J�o�[����",

    .s_Yes = "Yes",
    .s_No = "No",
    .s_PlsChose = "�I�����Ă�������",
    .s_OK = "OK",
    .s_Confirm = "�m�F",
    .s_Brightness = "���邳",
    .s_Volume = "����",
    .s_OptionsTit = "�I�v�V����",
    .s_FPS = "FPS",
    .s_BUSY = "�r�W�[",
    .s_Scaling = "�X�P�[�����O",
    .s_SCalingOff = "Off",
    .s_SCalingFit = "�t�B�b�g",
    .s_SCalingFull = "�t��",
    .s_SCalingCustom = "�J�X�^��",
    .s_Filtering = "�t�B���^�����O",
    .s_FilteringNone = "�Ȃ�",
    .s_FilteringOff = "�I�t",
    .s_FilteringSharp = "Sharp",
    .s_FilteringSoft = "Soft",
    .s_Speed = "�X�s�[�h",
    .s_Speed_Unit = "x",
    .s_Save_Cont = "�Z�[�u+���s",
    .s_Save_Quit = "�Z�[�u+�I��",
    .s_Reload = "�ēǂݍ���",
    .s_Options = "�I�v�V����",
    .s_Power_off = "�d���I�t",
    .s_Quit_to_menu = "Menu�ɖ߂�",
    .s_Retro_Go_options = "Retro-Go",

    .s_Font = "�t�H���g",
    .s_Colors = "�F",
    .s_Theme_Title = "�e�[�}",
    .s_Theme_sList = "�V���v���ȃ��X�g",
    .s_Theme_CoverV = "Coverflow�c",
    .s_Theme_CoverH = "Coverflow��",
    .s_Theme_CoverLightV = "CoverLight�c",
    .s_Theme_CoverLightH = "CoverLight��",
    //=====================================================================

    // Core\Src\retro-go\rg_emulators.c ====================================

    .s_File = "�t�@�C��",
    .s_Type = "�^�C�v",
    .s_Size = "�T�C�Y",
    .s_ImgSize = "�C���[�W�T�C�Y",
    .s_Close = "����",
    .s_GameProp = "�v���p�e�B",
    .s_Resume_game = "��������V��",
    .s_New_game = "�ŏ�����V��",
    .s_Del_favorite = "���C�ɓ���폜",
    .s_Add_favorite = "���C�ɓ���ǉ�",
    .s_Delete_save = "�Z�[�u���폜",
    .s_Confiem_del_save = "�Z�[�u�������H",
    .s_Free_space_alert = "Not enough free space for a new save, please delete some.",
#if CHEAT_CODES == 1
    .s_Cheat_Codes = "�`�[�g�R�[�h",
    .s_Cheat_Codes_Title = "�`�[�g�I�v�V����",
    .s_Cheat_Codes_ON = "\x6",
    .s_Cheat_Codes_OFF = "\x5",
#endif

    //=====================================================================

    // Core\Src\retro-go\rg_main.c =========================================
    .s_CPU_Overclock = "CPU��OverClock",
    .s_CPU_Overclock_0 = "None",
    .s_CPU_Overclock_1 = "Mid",
    .s_CPU_Overclock_2 = "Max",
    .s_CPU_OC_Upgrade_to = "Up to ",
    .s_CPU_OC_Downgrade_to = "Down to ",
    .s_CPU_OC_Stay_at = "Stay at ",
    .s_Confirm_OC_Reboot = "CPU�I�[�o�[�N���b�N�̐ݒ肪�ύX����܂����B�ċN�����K�v�ł��B��낵���ł����H",
#if INTFLASH_BANK == 2
    .s_Reboot = "�ċN��",
    .s_Original_system = "�I���W�i���V�X�e��",
    .s_Confirm_Reboot = "�ċN�����Ă�낵���ł����H",
#endif
    .s_Second_Unit = "s",
    .s_Version = "Ver.",
    .s_Author = "By",
    .s_Author_ = "\t\t+",
    .s_UI_Mod = "UI Mod",
    .s_Lang = "���{��",
    .s_LangAuthor = "�W��",
    .s_Debug_menu = "�f�o�b�O���j���[",
    .s_Reset_settings = "�ݒ�����Z�b�g",
    //.s_Close                   = "����",
    .s_Retro_Go = "Retro-Go�ɂ���",
    .s_Confirm_Reset_settings = "�S�Ă̐ݒ�����Z�b�g���܂���?",

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
    //.s_Close                   = "Close",
    .s_Debug_Title = "Debug",
    .s_Idle_power_off = "�X���[�v�܂ł̎���",

    .s_Time = "����",
    .s_Date = "���t",
    .s_Time_Title = "����",
    .s_Hour = "��",
    .s_Minute = "��",
    .s_Second = "�b",
    .s_Time_setup = "�����ݒ�",

    .s_Day = "��",
    .s_Month = "��",
    .s_Year = "�N",
    .s_Weekday = "�j��",
    .s_Date_setup = "���t�ݒ�",
            
/*
    .s_Weekday_Mon = "��",
    .s_Weekday_Tue = "��",
    .s_Weekday_Wed = "��",
    .s_Weekday_Thu = "��",
    .s_Weekday_Fri = "��",
    .s_Weekday_Sat = "�y",
    .s_Weekday_Sun = "��",
*/
    .s_Weekday_Mon = "Mon",
    .s_Weekday_Tue = "Tue",
    .s_Weekday_Wed = "Wed",
    .s_Weekday_Thu = "Thu",
    .s_Weekday_Fri = "Fri",
    .s_Weekday_Sat = "Sat",
    .s_Weekday_Sun = "Sun",
            
    .s_Turbo_Button = "�A��",
    .s_Turbo_None = "����",
    .s_Turbo_A = "A",
    .s_Turbo_B = "B",
    .s_Turbo_AB = "A & B",
    
    .s_Title_Date_Format = "%02d-%02d %s %02d:%02d:%02d",
    .s_Date_Format = "%02d.%02d.20%02d %s",
    .s_Time_Format = "%02d:%02d:%02d",

    .fmt_Title_Date_Format = ja_jp_fmt_Title_Date_Format,
    .fmtDate = ja_jp_fmt_Date,
    .fmtTime = ja_jp_fmt_Time,
    //=====================================================================
    //           ------------ end ---------------
};
#endif
