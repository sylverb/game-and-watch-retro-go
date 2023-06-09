/*
***************************************************
*                Warning!!!!!!!                   *
*  This file must be saved with cp949(EUC-KR) Encoding   *
***************************************************
*/
#if !defined (INCLUDED_KO_KR)
#define INCLUDED_KO_KR 0
#endif
#if INCLUDED_KO_KR==1

int ko_kr_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, day, month, weekday, hour, minutes, seconds);
};

int ko_kr_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, day, month, year, weekday);
};

int ko_kr_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_ko_kr LANG_DATA = {
    .codepage = 949,
    .extra_font = ko_kr_fonts,
    .s_LangUI = "���",
    .s_LangName = "Korean",
    //�ѱ���
    // If you can translate, please feed back the translation results to me, thank you
    // translate by  Augen(����������):

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
    .s_Palette = "�ȷ�Ʈ",
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette= "Palette" dul
    .s_Default = "�⺻",
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
    .s_msx_MSX1_EUR = "MSX1(EUR)",
    .s_msx_MSX2_EUR = "MSX2(EUR)",
    .s_msx_MSX2_JP = "MSX2+(JP)",
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
    .s_copy_RTC_to_GW_time = "copy RTC to G&W time",
    .s_copy_GW_time_to_RTC = "copy G&W time to RTC",
    .s_LCD_filter = "LCD filter",
    .s_Display_RAM = "Display RAM",
    .s_Press_ACL = "Press ACL or reset",
    .s_Press_TIME = "Press TIME [B+TIME]",
    .s_Press_ALARM = "Press ALARM [B+GAME]",
    .s_filter_0_none = "0-none",
    .s_filter_1_medium = "1-medium",
    .s_filter_2_high = "2-high",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",

    .s_No_Cover = "Ŀ�� ����",

    .s_Yes = "��",
    .s_No = "�ƴϿ�",
    .s_PlsChose = "������ �ּ��� := ",
    .s_OK = "Ȯ��",
    .s_Confirm = "����",
    .s_Brightness = "���",
    .s_Volume = "�Ҹ�ũ��",
    .s_OptionsTit = "ȯ�� ����",
    .s_FPS = "FPS",
    .s_BUSY = "Ŭ��(CPU)= ",
    .s_Scaling = "������",
    .s_SCalingOff = "Off",
    .s_SCalingFit = "Fit",
    .s_SCalingFull = "��üȭ��",
    .s_SCalingCustom = "Custom",
    .s_Filtering = "���͸�",
    .s_FilteringNone = "���͸� ����",
    .s_FilteringOff = "Off",
    .s_FilteringSharp = "Sharp",
    .s_FilteringSoft = "Soft",
    .s_Speed = "�ӵ�(���)",
    .s_Speed_Unit = "x",
    .s_Save_Cont = "���� �� ��� �ϱ�",
    .s_Save_Quit = "���� �� ���� �ϱ�",
    .s_Reload = "�ٽ� �ҷ�����",
    .s_Options = "����",
    .s_Power_off = "���� ����",
    .s_Quit_to_menu = "�Ŵ��� ������",
    .s_Retro_Go_options = "Retro-Go",

    .s_Font = "Font",
    .s_Colors = "Colors",
    .s_Theme_Title = "UI �¸�",
    .s_Theme_sList = "���� ����Ʈ",
    .s_Theme_CoverV = "Ŀ���÷ο� ����",
    .s_Theme_CoverH = "Ŀ���÷ο� ����",
    .s_Theme_CoverLightV = "Ŀ���÷ο� V",
    .s_Theme_CoverLightH = "Ŀ���÷ο�",

    //=====================================================================

    // Core\Src\retro-go\rg_emulators.c ====================================
    .s_File = "����",
    .s_Type = "����",
    .s_Size = "ũ��",
    .s_ImgSize = "�̹��� ũ��",
    .s_Close = "�ݱ�",
    .s_GameProp = "�Ӽ�",
    .s_Resume_game = "��� ���� �ϱ�",
    .s_New_game = "���� ���� ���� �ϱ�",
    .s_Del_favorite = "���ã�� ����",
    .s_Add_favorite = "���ã�� �߰�",
    .s_Delete_save = "���嵥���� ����",
    .s_Confiem_del_save = "���� �����͸� �����Ͻðڽ��ϱ�?",
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
    .s_Confirm_OC_Reboot = "CPU Overclock configuration has changed and needs to reboot now. Are you sure?",
#if INTFLASH_BANK == 2
    .s_Reboot = "Reboot",
    .s_Original_system = "Original system",
    .s_Confirm_Reboot = "Confirm reboot?",
#endif
    .s_Second_Unit = "��",
    .s_Version = "Ver.",
    .s_Author = "By",
    .s_Author_ = "\t\t+",
    .s_UI_Mod = "�������̽� ���",
    .s_Lang = "�ѱ���",
    .s_LangAuthor = "Augen(����������)",
    .s_Debug_menu = "����� �Ŵ�",
    .s_Reset_settings = "��� ���� �ʱ�ȭ",
    //.s_Close                  = "�ݱ�",
    .s_Retro_Go = "Retro-Go ����= ",
    .s_Confirm_Reset_settings = "��� ������ �缳�� �Ͻðڽ��ϱ�?",

    .s_Flash_JEDEC_ID = "�÷��� JEDEC ID",
    .s_Flash_Name = "�÷��� �̸�",
    .s_Flash_SR = "�÷��� SR",
    .s_Flash_CR = "�÷��� CR",
    .s_Smallest_erase = "Smallest �����",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "DBGMCU CK Ȱ��ȭ",
    .s_Disable_DBGMCU_CK = "DBGMCU CK ��Ȱ��ȭ",
    //.s_Close                  = "�ݱ�",
    .s_Debug_Title = "�����",
    .s_Idle_power_off = "��� ���� ����",
	.s_Splash_Option = "���� �ִϸ��̼�",
    .s_Splash_On = "\x6",
    .s_Splash_Off = "\x5",	

    .s_Time = "�ð�",
    .s_Date = "����",
    .s_Time_Title = "�ð�",
    .s_Hour = "��",
    .s_Minute = "��",
    .s_Second = "��",
    .s_Time_setup = "�ð� ����",

    .s_Day = "��",
    .s_Month = "��",
    .s_Year = "��",
    .s_Weekday = "��",
    .s_Date_setup = "��¥ ����",

    .s_Weekday_Mon = "��",
    .s_Weekday_Tue = "ȭ",
    .s_Weekday_Wed = "��",
    .s_Weekday_Thu = "��",
    .s_Weekday_Fri = "��",
    .s_Weekday_Sat = "��",
    .s_Weekday_Sun = "��",

    .s_Turbo_Button = "Turbo",
    .s_Turbo_None = "None",
    .s_Turbo_A = "A",
    .s_Turbo_B = "B",
    .s_Turbo_AB = "A & B",    

    .s_Title_Date_Format = "%02d-%02d %s %02d:%02d:%02d",
    .s_Date_Format = "%02d.%02d.20%02d %s",
    .s_Time_Format = "%02d:%02d:%02d",
    .fmt_Title_Date_Format = ko_kr_fmt_Title_Date_Format,
    .fmtDate = ko_kr_fmt_Date,
    .fmtTime = ko_kr_fmt_Time,
    //=====================================================================
};
#endif
