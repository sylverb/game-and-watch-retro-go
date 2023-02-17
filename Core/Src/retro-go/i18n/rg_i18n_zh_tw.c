/*
***************************************************************
*                Warning!!!!!!!                               *
*  This file must be saved with cp950(BIG5)        Encoding   *
***************************************************************
*/
#if !defined (INCLUDED_ZH_TW)
#define INCLUDED_ZH_TW 0
#endif
// Stand 繁体中文
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
    .s_LangUI = "語言",
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
    .s_Palette = "調色盤",
    //=====================================================================

    // Core\Src\porting\md\main_gwenesis.c ================================
    .s_md_keydefine = "按鍵映射 A-B-C",
    .s_md_Synchro = "同步方式",
    .s_md_Synchro_Audio = "音訊",
    .s_md_Synchro_Vsync = "視頻",
    .s_md_Dithering = "抖動顯示",
    .s_md_Debug_bar = "測試信息",
    .s_md_Option_ON = "\x6",
    .s_md_Option_OFF = "\x5",
    .s_md_AudioFilter = "音訊提升",
    .s_md_VideoUpscaler = "視頻提升",    
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette= "調色板" dul
    .s_Default = "預設",
    //=====================================================================

    // Core\Src\porting\md\main_wsv.c ================================
    .s_wsv_palette_Default = "預設",
    .s_wsv_palette_Amber = "琥珀",
    .s_wsv_palette_Green = "綠色",
    .s_wsv_palette_Blue = "藍色",
    .s_wsv_palette_BGB = "藍綠",
    .s_wsv_palette_Wataroo = "瓦塔羅",
    //=====================================================================

    // Core\Src\porting\md\main_msx.c ================================
    .s_msx_Change_Dsk = "更換碟片",
    .s_msx_Select_MSX = "選擇版本",
    .s_msx_MSX1_EUR = "MSX1 (歐)",
    .s_msx_MSX2_EUR = "MSX2 (歐)",
    .s_msx_MSX2_JP = "MSX2+ (日)",
    .s_msx_Frequency = "場頻",
    .s_msx_Freq_Auto = "自動",
    .s_msx_Freq_50 = "50Hz",
    .s_msx_Freq_60 = "60Hz",
    .s_msx_A_Button = "Ａ鍵",
    .s_msx_B_Button = "Ｂ鍵",
    .s_msx_Press_Key = "模擬按鍵",
    //=====================================================================

    // Core\Src\porting\md\main_amstrad.c ================================
    .s_amd_Change_Dsk = "更換碟片",
    .s_amd_Controls = "控制??",
    .s_amd_Controls_Joystick = "?杆",
    .s_amd_Controls_Keyboard = "??",
    .s_amd_palette_Color = "彩色",
    .s_amd_palette_Green = "綠色",
    .s_amd_palette_Grey = "灰色",
    .s_amd_Press_Key = "模擬按鍵",
    //=====================================================================


    // Core\Src\porting\gw\main_gw.c =======================================
    .s_copy_RTC_to_GW_time = "從系統時間同步",
    .s_copy_GW_time_to_RTC = "同步時間到系統",
    .s_LCD_filter = "螢幕濾鏡",
    .s_Display_RAM = "顯示記憶體資訊",
    .s_Press_ACL = "重置遊戲",
    .s_Press_TIME = "模擬 TIME  鍵 [B+TIME]",
    .s_Press_ALARM = "模擬 ALARM 鍵 [B+GAME]",
    .s_filter_0_none = "關",
    .s_filter_1_medium = "中",
    .s_filter_2_high = "高",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",

    .s_No_Cover = "無封面",

    .s_Yes = "○ 是",
    .s_No = "× 否",
    .s_PlsChose = "請選擇：",
    .s_OK = "○ 確定",
    .s_Confirm = "資訊確認",
    .s_Brightness = "亮度",
    .s_Volume = "音量",
    .s_OptionsTit = "系統設定",
    .s_FPS = "幀頻",
    .s_BUSY = "負載（CPU）",
    .s_Scaling = "縮放",
    .s_SCalingOff = "關閉",
    .s_SCalingFit = "自動",
    .s_SCalingFull = "全螢幕",
    .s_SCalingCustom = "自訂",
    .s_Filtering = "濾鏡",
    .s_FilteringNone = "無",
    .s_FilteringOff = "關閉",
    .s_FilteringSharp = "銳利",
    .s_FilteringSoft = "柔和",
    .s_Speed = "速度",
    .s_Speed_Unit = "倍",
    .s_Save_Cont = "■ 儲存進度",
    .s_Save_Quit = "▲ 儲存後退出",
    .s_Reload = "∞ 重新載入",
    .s_Options = "◎ 遊戲設定",
    .s_Power_off = "ω 關機休眠",
    .s_Quit_to_menu = "× 離開遊戲",
    .s_Retro_Go_options = "遊戲選項",
    .s_Font = "字型樣式",
    .s_Colors = "配色方案",
    .s_Theme_Title = "介面樣式",
    .s_Theme_sList = "至簡列表",
    .s_Theme_CoverV = "垂直捲動",
    .s_Theme_CoverH = "水平捲動",
    .s_Theme_CoverLightV = "垂直滾動",
    .s_Theme_CoverLightH = "水平滾動",
    //=====================================================================
    // Core\Src\retro-go\rg_emulators.c ====================================
    .s_File = "名稱：",
    .s_Type = "類型：",
    .s_Size = "大小：",
    .s_ImgSize = "圖像：",
    .s_Close = "× 關閉",
    .s_GameProp = "遊戲屬性",
    .s_Resume_game = "＞ 載入存檔",
    .s_New_game = "◇ 開始遊戲",
    .s_Del_favorite = "☆ 移除收藏",
    .s_Add_favorite = "★ 添加收藏",
    .s_Delete_save = "□ 刪除進度",
    .s_Confiem_del_save = "您確認要刪除目前的遊戲存檔？",
#if CHEAT_CODES == 1
    .s_Cheat_Codes = "＆ 精靈代碼",
    .s_Cheat_Codes_Title = "金手指",
    .s_Cheat_Codes_ON = "\x6",
    .s_Cheat_Codes_OFF = "\x5",
#endif

    //=====================================================================
    // Core\Src\retro-go\rg_main.c =========================================
    .s_CPU_Overclock = "超頻",
    .s_CPU_Overclock_0 = "關閉",
    .s_CPU_Overclock_1 = "適度",
    .s_CPU_Overclock_2 = "極限",
    .s_CPU_OC_Upgrade_to = "升至",
    .s_CPU_OC_Downgrade_to = "降至",
    .s_CPU_OC_Stay_at = "保持",
    .s_Confirm_OC_Reboot = "超頻配置需要重啟後才能生效。您確認立即重啟？",
#if INTFLASH_BANK == 2
    .s_Reboot = "重啟",
    .s_Original_system = "原生系統",
    .s_Confirm_Reboot = "您確定要重啟設備？",
#endif
    .s_Second_Unit = "秒",
    .s_Version = "版　　本：",
    .s_Author = "特別貢獻：",
    .s_Author_ = "　　　　：",
    .s_UI_Mod = "介面美化：",
    .s_Lang = "繁體中文：",
    .s_LangAuthor = "撓漿糊的",
    .s_Debug_menu = "∞ 調試選項",
    .s_Reset_settings = "≡ 恢復預設",
    //.s_Close                  = "Close",
    .s_Retro_Go = "關於 Retro-Go",
    .s_Confirm_Reset_settings = "您確定要恢復所有設定數據？",

    .s_Flash_JEDEC_ID = "存儲 JEDEC ID",
    .s_Flash_Name = "存儲晶片",
    .s_Flash_SR = "存儲狀態",
    .s_Flash_CR = "存儲配置",
    .s_Smallest_erase = "最小抹除單位",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "開啟 DBGMCU CK",
    .s_Disable_DBGMCU_CK = "關閉 DBGMCU CK",
    //.s_Close                  = "Close",
    .s_Debug_Title = "調試選項",
    .s_Idle_power_off = "省電待機",

    .s_Time = "時間：",
    .s_Date = "日期：",
    .s_Time_Title = "時間",
    .s_Hour = "時：",
    .s_Minute = "分：",
    .s_Second = "秒：",
    .s_Time_setup = "時間設定",

    .s_Day = "日  ：",
    .s_Month = "月  ：",
    .s_Year = "年  ：",
    .s_Weekday = "星期：",
    .s_Date_setup = "日期設定",

    .s_Weekday_Mon = "一",
    .s_Weekday_Tue = "二",
    .s_Weekday_Wed = "三",
    .s_Weekday_Thu = "四",
    .s_Weekday_Fri = "五",
    .s_Weekday_Sat = "六",
    .s_Weekday_Sun = "日",

    .s_Turbo_Button = "連發",
    .s_Turbo_None = "關閉",
    .s_Turbo_A = "Ａ",
    .s_Turbo_B = "Ｂ",
    .s_Turbo_AB = "Ａ和Ｂ",    

    .s_Title_Date_Format = "%02d-%02d 周%s %02d:%02d:%02d",
    .s_Date_Format = "20%02d年%02d月%02d日 周%s",
    .s_Time_Format = "%02d:%02d:%02d",

    .fmt_Title_Date_Format = zh_tw_fmt_Title_Date_Format,
    .fmtDate = zh_tw_fmt_Date,
    .fmtTime = zh_tw_fmt_Time,
    //=====================================================================
};
#endif
