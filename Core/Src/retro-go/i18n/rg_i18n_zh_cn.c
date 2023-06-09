/*
***********************************************************
*                Warning!!!!!!!                           *
*  This file must be saved with cp936(gbk gb2312) Encoding*
***********************************************************
*/
#if !defined (INCLUDED_ZH_CN)
#define INCLUDED_ZH_CN 0
#endif
#if !defined (CHEAT_CODES)
#define CHEAT_CODES 0
#endif
#if INCLUDED_ZH_CN==1

// Stand ��������

int zh_cn_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, month, day, weekday, hour, minutes, seconds);
};

int zh_cn_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, year, month, day, weekday);
};

int zh_cn_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_zh_cn LANG_DATA = {
    .codepage = 936,
    .extra_font = zh_cn_fonts,
    .s_LangUI = "����",
    .s_LangName = "S_Chinese",
    
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
    .s_Palette = "��ɫ��",
    //=====================================================================

    // Core\Src\porting\md\main_gwenesis.c ================================
    .s_md_keydefine = "����ӳ�� A-B-C",
    .s_md_Synchro = "ͬ����ʽ",
    .s_md_Synchro_Audio = "��Ƶ",
    .s_md_Synchro_Vsync = "��Ƶ",
    .s_md_Dithering = "������ʾ",
    .s_md_Debug_bar = "������Ϣ",
    .s_md_Option_ON = "\x6",
    .s_md_Option_OFF = "\x5",
    .s_md_AudioFilter = "��Ƶ����",
    .s_md_VideoUpscaler = "��Ƶ����",
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette= "��ɫ��" dul
    .s_Default = "Ĭ��",
    //=====================================================================

    // Core\Src\porting\md\main_wsv.c ================================
    .s_wsv_palette_Default = "Ĭ��",
    .s_wsv_palette_Amber = "����",
    .s_wsv_palette_Green = "��ɫ",
    .s_wsv_palette_Blue = "��ɫ",
    .s_wsv_palette_BGB = "����",
    .s_wsv_palette_Wataroo = "������",
    //=====================================================================

    // Core\Src\porting\md\main_msx.c ================================
    .s_msx_Change_Dsk = "������Ƭ",
    .s_msx_Select_MSX = "ѡ��汾",
    .s_msx_MSX1_EUR = "MSX1 (ŷ)",
    .s_msx_MSX2_EUR = "MSX2 (ŷ)",
    .s_msx_MSX2_JP = "MSX2+ (��)",
    .s_msx_Frequency = "��Ƶ",
    .s_msx_Freq_Auto = "�Զ�",
    .s_msx_Freq_50 = "50Hz",
    .s_msx_Freq_60 = "60Hz",
    .s_msx_A_Button = "����",
    .s_msx_B_Button = "�¼�",
    .s_msx_Press_Key = "ģ�ⰴ��",
    //=====================================================================

    // Core\Src\porting\md\main_amstrad.c ================================
    .s_amd_Change_Dsk = "������Ƭ",
    .s_amd_Controls = "�����豸",
    .s_amd_Controls_Joystick = "ҡ��",
    .s_amd_Controls_Keyboard = "����",
    .s_amd_palette_Color = "��ɫ",
    .s_amd_palette_Green = "��ɫ",
    .s_amd_palette_Grey = "��ɫ",
    .s_amd_Press_Key = "ģ�ⰴ��",
    //=====================================================================

    // Core\Src\porting\gw\main_gw.c =======================================
    .s_copy_RTC_to_GW_time = "��ϵͳʱ��ͬ��",
    .s_copy_GW_time_to_RTC = "ͬ��ʱ�䵽ϵͳ",
    .s_LCD_filter = "��Ļ�����",
    .s_Display_RAM = "��ʾ�ڴ���Ϣ",
    .s_Press_ACL = "������Ϸ",
    .s_Press_TIME = "ģ�� TIME  �� [B+TIME]",
    .s_Press_ALARM = "ģ�� ALARM �� [B+GAME]",
    .s_filter_0_none = "��",
    .s_filter_1_medium = "��",
    .s_filter_2_high = "��",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",

    .s_No_Cover = "�޷���",

    .s_Yes = "�� ��",
    .s_No = "�� ��",
    .s_PlsChose = "��ѡ��",
    .s_OK = "�� ȷ��",
    .s_Confirm = "��Ϣȷ��",
    .s_Brightness = "����",
    .s_Volume = "����",
    .s_OptionsTit = "ϵͳ����",
    .s_FPS = "֡��",
    .s_BUSY = "���أ�CPU��",
    .s_Scaling = "����",
    .s_SCalingOff = "�ر�",
    .s_SCalingFit = "����Ӧ",
    .s_SCalingFull = "ȫ��",
    .s_SCalingCustom = "�Զ���",
    .s_Filtering = "����",
    .s_FilteringNone = "��",
    .s_FilteringOff = "�ر�",
    .s_FilteringSharp = "����",
    .s_FilteringSoft = "���",
    .s_Speed = "�ٶ�",
    .s_Speed_Unit = "��",
    .s_Save_Cont = "�� �������",
    .s_Save_Quit = "�� �����˳�",
    .s_Reload = "�� ���¼���",
    .s_Options = "�� ��Ϸ����",
    .s_Power_off = "�� �ػ�����",
    .s_Quit_to_menu = "�� �˳���Ϸ",
    .s_Retro_Go_options = "��Ϸѡ��",

    .s_Font = "����",
    .s_Colors = "ɫ��",
    .s_Theme_Title = "չʾ",
    .s_Theme_sList = "���б�",
    .s_Theme_CoverV = "��ֱ����", // vertical
    .s_Theme_CoverH = "ˮƽ����", // horizontal
    .s_Theme_CoverLightV = "��ֱ����",
    .s_Theme_CoverLightH = "ˮƽ����",
    //=====================================================================

    // Core\Src\retro-go\rg_emulators.c ====================================

    .s_File = "���ƣ�",
    .s_Type = "���ͣ�",
    .s_Size = "��С��",
    .s_ImgSize = "ͼ��",
    .s_Close = "�� �ر�",
    .s_GameProp = "��Ϸ�ļ�����",
    .s_Resume_game = "�� ������Ϸ",
    .s_New_game = "�� ��ʼ��Ϸ",
    .s_Del_favorite = "�� �Ƴ��ղ�",
    .s_Add_favorite = "�� ����ղ�",
    .s_Delete_save = "�� ɾ������",
    .s_Confiem_del_save = "��ȷ��Ҫɾ���ѱ������Ϸ���ȣ�",
#if CHEAT_CODES == 1
    .s_Cheat_Codes = "�� ����ָ��",
    .s_Cheat_Codes_Title = "����ָ",
    .s_Cheat_Codes_ON = "\x6",
    .s_Cheat_Codes_OFF = "\x5",
#endif

    //=====================================================================

    // Core\Src\retro-go\rg_main.c =========================================
    .s_CPU_Overclock = "��Ƶ",
    .s_CPU_Overclock_0 = "�ر�",
    .s_CPU_Overclock_1 = "�ʶ�",
    .s_CPU_Overclock_2 = "����",
    .s_CPU_OC_Upgrade_to = "����",
    .s_CPU_OC_Downgrade_to = "����",
    .s_CPU_OC_Stay_at = "����",
    .s_Confirm_OC_Reboot = "��Ƶ������Ҫ�����������Ч����ȷ������������",
#if INTFLASH_BANK == 2
    .s_Reboot = "����",
    .s_Original_system = "ԭ��ϵͳ",
    .s_Confirm_Reboot = "��ȷ��Ҫ�����豸��",
#endif
    .s_Second_Unit = "��",
    .s_Version = "�桡������",
    .s_Author = "�ر��ף�",
    .s_Author_ = "����������",
    .s_UI_Mod = "����������",
    .s_Lang = "�������ģ�",
    .s_LangAuthor = "�ӽ�����",
    .s_Debug_menu = "�� ������Ϣ",
    .s_Reset_settings = "�� �����趨",
    //.s_Close                  = "Close",
    .s_Retro_Go = "���� Retro-Go",
    .s_Confirm_Reset_settings = "��ȷ��Ҫ���������趨��Ϣ��",

    .s_Flash_JEDEC_ID = "�洢 JEDEC ID",
    .s_Flash_Name = "�洢оƬ",
    .s_Flash_SR = "�洢״̬",
    .s_Flash_CR = "�洢����",
    .s_Smallest_erase = "��С��λ",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "���� DBGMCU CK",
    .s_Disable_DBGMCU_CK = "���� DBGMCU CK",
    //.s_Close                  = "Close",
    .s_Debug_Title = "����ѡ��",
	.s_Idle_power_off = "���д���",
    .s_Splash_Option = "��������",
    .s_Splash_On = "\x6",
    .s_Splash_Off = "\x5",
	
    .s_Time = "ʱ�䣺",
    .s_Date = "���ڣ�",
    .s_Time_Title = "ʱ��",
    .s_Hour = "ʱ��",
    .s_Minute = "�֣�",
    .s_Second = "�룺",
    .s_Time_setup = "ʱ������",

    .s_Day = "��  ��",
    .s_Month = "��  ��",
    .s_Year = "��  ��",
    .s_Weekday = "���ڣ�",
    .s_Date_setup = "��������",

    .s_Weekday_Mon = "һ",
    .s_Weekday_Tue = "��",
    .s_Weekday_Wed = "��",
    .s_Weekday_Thu = "��",
    .s_Weekday_Fri = "��",
    .s_Weekday_Sat = "��",
    .s_Weekday_Sun = "��",

    .s_Turbo_Button = "����",
    .s_Turbo_None = "��",
    .s_Turbo_A = "��",
    .s_Turbo_B = "��",
    .s_Turbo_AB = "���ͣ�",    

    .s_Date_Format = "20%02d��%02d��%02d�� ��%s",
    .s_Title_Date_Format = "%02d-%02d ��%s %02d:%02d:%02d",
    .s_Time_Format = "%02d:%02d:%02d",

    .fmt_Title_Date_Format = zh_cn_fmt_Title_Date_Format,
    .fmtDate = zh_cn_fmt_Date,
    .fmtTime = zh_cn_fmt_Time,
    //=====================================================================
};

#endif
