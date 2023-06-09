/*
***************************************************************
*                Warning!!!!!!!                               *
*  This file must be saved with cp950(BIG5)        Encoding   *
***************************************************************
*/
#if !defined (INCLUDED_ZH_TW)
#define INCLUDED_ZH_TW 0
#endif
// Stand �c�^����
#if INCLUDED_ZH_TW==1

int zh_tw_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, month, day, weekday, hour, minutes, seconds);
};

int zh_tw_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, year, month, day, weekday);
};

int zh_tw_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_zh_tw LANG_DATA = {
    .codepage = 950,
    .extra_font = zh_tw_fonts,
    .s_LangUI = "�y��",
    .s_LangName = "T_Chinese",

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
    .s_Palette = "�զ�L",
    //=====================================================================

    // Core\Src\porting\md\main_gwenesis.c ================================
    .s_md_keydefine = "����M�g A-B-C",
    .s_md_Synchro = "�P�B�覡",
    .s_md_Synchro_Audio = "���T",
    .s_md_Synchro_Vsync = "���W",
    .s_md_Dithering = "�ݰ����",
    .s_md_Debug_bar = "���իH��",
    .s_md_Option_ON = "\x6",
    .s_md_Option_OFF = "\x5",
    .s_md_AudioFilter = "���T����",
    .s_md_VideoUpscaler = "���W����",    
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette= "�զ�O" dul
    .s_Default = "�w�]",
    //=====================================================================

    // Core\Src\porting\md\main_wsv.c ================================
    .s_wsv_palette_Default = "�w�]",
    .s_wsv_palette_Amber = "�[��",
    .s_wsv_palette_Green = "���",
    .s_wsv_palette_Blue = "�Ŧ�",
    .s_wsv_palette_BGB = "�ź�",
    .s_wsv_palette_Wataroo = "�˶�ù",
    //=====================================================================

    // Core\Src\porting\md\main_msx.c ================================
    .s_msx_Change_Dsk = "�󴫺Ф�",
    .s_msx_Select_MSX = "��ܪ���",
    .s_msx_MSX1_EUR = "MSX1 (��)",
    .s_msx_MSX2_EUR = "MSX2 (��)",
    .s_msx_MSX2_JP = "MSX2+ (��)",
    .s_msx_Frequency = "���W",
    .s_msx_Freq_Auto = "�۰�",
    .s_msx_Freq_50 = "50Hz",
    .s_msx_Freq_60 = "60Hz",
    .s_msx_A_Button = "����",
    .s_msx_B_Button = "����",
    .s_msx_Press_Key = "��������",
    //=====================================================================

    // Core\Src\porting\md\main_amstrad.c ================================
    .s_amd_Change_Dsk = "�󴫺Ф�",
    .s_amd_Controls = "����??",
    .s_amd_Controls_Joystick = "?��",
    .s_amd_Controls_Keyboard = "??",
    .s_amd_palette_Color = "�m��",
    .s_amd_palette_Green = "���",
    .s_amd_palette_Grey = "�Ǧ�",
    .s_amd_Press_Key = "��������",
    //=====================================================================


    // Core\Src\porting\gw\main_gw.c =======================================
    .s_copy_RTC_to_GW_time = "�q�t�ήɶ��P�B",
    .s_copy_GW_time_to_RTC = "�P�B�ɶ���t��",
    .s_LCD_filter = "�ù��o��",
    .s_Display_RAM = "��ܰO�����T",
    .s_Press_ACL = "���m�C��",
    .s_Press_TIME = "���� TIME  �� [B+TIME]",
    .s_Press_ALARM = "���� ALARM �� [B+GAME]",
    .s_filter_0_none = "��",
    .s_filter_1_medium = "��",
    .s_filter_2_high = "��",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",

    .s_No_Cover = "�L�ʭ�",

    .s_Yes = "�� �O",
    .s_No = "�� �_",
    .s_PlsChose = "�п�ܡG",
    .s_OK = "�� �T�w",
    .s_Confirm = "��T�T�{",
    .s_Brightness = "�G��",
    .s_Volume = "���q",
    .s_OptionsTit = "�t�γ]�w",
    .s_FPS = "�V�W",
    .s_BUSY = "�t���]CPU�^",
    .s_Scaling = "�Y��",
    .s_SCalingOff = "����",
    .s_SCalingFit = "�۰�",
    .s_SCalingFull = "���ù�",
    .s_SCalingCustom = "�ۭq",
    .s_Filtering = "�o��",
    .s_FilteringNone = "�L",
    .s_FilteringOff = "����",
    .s_FilteringSharp = "�U�Q",
    .s_FilteringSoft = "�X�M",
    .s_Speed = "�t��",
    .s_Speed_Unit = "��",
    .s_Save_Cont = "�� �x�s�i��",
    .s_Save_Quit = "�� �x�s��h�X",
    .s_Reload = "�� ���s���J",
    .s_Options = "�� �C���]�w",
    .s_Power_off = "�s ������v",
    .s_Quit_to_menu = "�� ���}�C��",
    .s_Retro_Go_options = "�C���ﶵ",
    .s_Font = "�r���˦�",
    .s_Colors = "�t����",
    .s_Theme_Title = "�����˦�",
    .s_Theme_sList = "��²�C��",
    .s_Theme_CoverV = "��������",
    .s_Theme_CoverH = "��������",
    .s_Theme_CoverLightV = "�����u��",
    .s_Theme_CoverLightH = "�����u��",
    //=====================================================================
    // Core\Src\retro-go\rg_emulators.c ====================================
    .s_File = "�W�١G",
    .s_Type = "�����G",
    .s_Size = "�j�p�G",
    .s_ImgSize = "�Ϲ��G",
    .s_Close = "�� ����",
    .s_GameProp = "�C���ݩ�",
    .s_Resume_game = "�� ���J�s��",
    .s_New_game = "�� �}�l�C��",
    .s_Del_favorite = "�� ��������",
    .s_Add_favorite = "�� �K�[����",
    .s_Delete_save = "�� �R���i��",
    .s_Confiem_del_save = "�z�T�{�n�R���ثe���C���s�ɡH",
#if CHEAT_CODES == 1
    .s_Cheat_Codes = "�� ���F�N�X",
    .s_Cheat_Codes_Title = "�����",
    .s_Cheat_Codes_ON = "\x6",
    .s_Cheat_Codes_OFF = "\x5",
#endif

    //=====================================================================
    // Core\Src\retro-go\rg_main.c =========================================
    .s_CPU_Overclock = "�W�W",
    .s_CPU_Overclock_0 = "����",
    .s_CPU_Overclock_1 = "�A��",
    .s_CPU_Overclock_2 = "����",
    .s_CPU_OC_Upgrade_to = "�ɦ�",
    .s_CPU_OC_Downgrade_to = "����",
    .s_CPU_OC_Stay_at = "�O��",
    .s_Confirm_OC_Reboot = "�W�W�t�m�ݭn���ҫ�~��ͮġC�z�T�{�ߧY���ҡH",
#if INTFLASH_BANK == 2
    .s_Reboot = "����",
    .s_Original_system = "��ͨt��",
    .s_Confirm_Reboot = "�z�T�w�n���ҳ]�ơH",
#endif
    .s_Second_Unit = "��",
    .s_Version = "���@�@���G",
    .s_Author = "�S�O�^�m�G",
    .s_Author_ = "�@�@�@�@�G",
    .s_UI_Mod = "�������ơG",
    .s_Lang = "�c�餤��G",
    .s_LangAuthor = "���߽k��",
    .s_Debug_menu = "�� �ոտﶵ",
    .s_Reset_settings = "�� ��_�w�]",
    //.s_Close                  = "Close",
    .s_Retro_Go = "���� Retro-Go",
    .s_Confirm_Reset_settings = "�z�T�w�n��_�Ҧ��]�w�ƾڡH",

    .s_Flash_JEDEC_ID = "�s�x JEDEC ID",
    .s_Flash_Name = "�s�x����",
    .s_Flash_SR = "�s�x���A",
    .s_Flash_CR = "�s�x�t�m",
    .s_Smallest_erase = "�̤p�ٰ����",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "�}�� DBGMCU CK",
    .s_Disable_DBGMCU_CK = "���� DBGMCU CK",
    //.s_Close                  = "Close",
    .s_Debug_Title = "�ոտﶵ",
    .s_Idle_power_off = "�ٹq�ݾ�",
	.s_Splash_Option = "�Ұʵe��",
    .s_Splash_On = "\x6",
    .s_Splash_Off = "\x5",

    .s_Time = "�ɶ��G",
    .s_Date = "����G",
    .s_Time_Title = "�ɶ�",
    .s_Hour = "�ɡG",
    .s_Minute = "���G",
    .s_Second = "��G",
    .s_Time_setup = "�ɶ��]�w",

    .s_Day = "��  �G",
    .s_Month = "��  �G",
    .s_Year = "�~  �G",
    .s_Weekday = "�P���G",
    .s_Date_setup = "����]�w",

    .s_Weekday_Mon = "�@",
    .s_Weekday_Tue = "�G",
    .s_Weekday_Wed = "�T",
    .s_Weekday_Thu = "�|",
    .s_Weekday_Fri = "��",
    .s_Weekday_Sat = "��",
    .s_Weekday_Sun = "��",

    .s_Turbo_Button = "�s�o",
    .s_Turbo_None = "����",
    .s_Turbo_A = "��",
    .s_Turbo_B = "��",
    .s_Turbo_AB = "�ϩM��",    

    .s_Title_Date_Format = "%02d-%02d �P%s %02d:%02d:%02d",
    .s_Date_Format = "20%02d�~%02d��%02d�� �P%s",
    .s_Time_Format = "%02d:%02d:%02d",

    .fmt_Title_Date_Format = zh_tw_fmt_Title_Date_Format,
    .fmtDate = zh_tw_fmt_Date,
    .fmtTime = zh_tw_fmt_Time,
    //=====================================================================
};
#endif
