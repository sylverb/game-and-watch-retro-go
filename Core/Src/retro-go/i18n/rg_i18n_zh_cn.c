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

// Stand 简体中文

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
    .s_LangUI = "语言",
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
    .s_Palette = "调色板",
    //=====================================================================

    // Core\Src\porting\md\main_gwenesis.c ================================
    .s_md_keydefine = "按键映射 A-B-C",
    .s_md_Synchro = "同步方式",
    .s_md_Synchro_Audio = "音频",
    .s_md_Synchro_Vsync = "视频",
    .s_md_Dithering = "抖动显示",
    .s_md_Debug_bar = "测试信息",
    .s_md_Option_ON = "\x6",
    .s_md_Option_OFF = "\x5",
    .s_md_AudioFilter = "音频提升",
    .s_md_VideoUpscaler = "视频提升",
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette= "调色板" dul
    .s_Default = "默认",
    //=====================================================================

    // Core\Src\porting\md\main_wsv.c ================================
    .s_wsv_palette_Default = "默认",
    .s_wsv_palette_Amber = "琥珀",
    .s_wsv_palette_Green = "绿色",
    .s_wsv_palette_Blue = "蓝色",
    .s_wsv_palette_BGB = "蓝绿",
    .s_wsv_palette_Wataroo = "瓦塔罗",
    //=====================================================================

    // Core\Src\porting\md\main_msx.c ================================
    .s_msx_Change_Dsk = "更换盘片",
    .s_msx_Select_MSX = "选择版本",
    .s_msx_MSX1_EUR = "MSX1 (欧)",
    .s_msx_MSX2_EUR = "MSX2 (欧)",
    .s_msx_MSX2_JP = "MSX2+ (日)",
    .s_msx_Frequency = "场频",
    .s_msx_Freq_Auto = "自动",
    .s_msx_Freq_50 = "50Hz",
    .s_msx_Freq_60 = "60Hz",
    .s_msx_A_Button = "Ａ键",
    .s_msx_B_Button = "Ｂ键",
    .s_msx_Press_Key = "模拟按键",
    //=====================================================================

    // Core\Src\porting\md\main_amstrad.c ================================
    .s_amd_Change_Dsk = "更换盘片",
    .s_amd_Controls = "控制设备",
    .s_amd_Controls_Joystick = "摇杆",
    .s_amd_Controls_Keyboard = "键盘",
    .s_amd_palette_Color = "彩色",
    .s_amd_palette_Green = "绿色",
    .s_amd_palette_Grey = "灰色",
    .s_amd_Press_Key = "模拟按键",
    //=====================================================================

    // Core\Src\porting\gw\main_gw.c =======================================
    .s_copy_RTC_to_GW_time = "从系统时间同步",
    .s_copy_GW_time_to_RTC = "同步时间到系统",
    .s_LCD_filter = "屏幕抗锯齿",
    .s_Display_RAM = "显示内存信息",
    .s_Press_ACL = "重置游戏",
    .s_Press_TIME = "模拟 TIME  键 [B+TIME]",
    .s_Press_ALARM = "模拟 ALARM 键 [B+GAME]",
    .s_filter_0_none = "关",
    .s_filter_1_medium = "中",
    .s_filter_2_high = "高",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",

    .s_No_Cover = "无封面",

    .s_Yes = "○ 是",
    .s_No = "× 否",
    .s_PlsChose = "请选择：",
    .s_OK = "○ 确定",
    .s_Confirm = "信息确认",
    .s_Brightness = "亮度",
    .s_Volume = "音量",
    .s_OptionsTit = "系统设置",
    .s_FPS = "帧率",
    .s_BUSY = "负载（CPU）",
    .s_Scaling = "缩放",
    .s_SCalingOff = "关闭",
    .s_SCalingFit = "自适应",
    .s_SCalingFull = "全屏",
    .s_SCalingCustom = "自定义",
    .s_Filtering = "过滤",
    .s_FilteringNone = "无",
    .s_FilteringOff = "关闭",
    .s_FilteringSharp = "锐利",
    .s_FilteringSoft = "柔和",
    .s_Speed = "速度",
    .s_Speed_Unit = "倍",
    .s_Save_Cont = "■ 保存进度",
    .s_Save_Quit = "▲ 保存退出",
    .s_Reload = "∞ 重新加载",
    .s_Options = "◎ 游戏设置",
    .s_Power_off = "ω 关机休眠",
    .s_Quit_to_menu = "× 退出游戏",
    .s_Retro_Go_options = "游戏选项",

    .s_Font = "字体",
    .s_Colors = "色彩",
    .s_Theme_Title = "展示",
    .s_Theme_sList = "简单列表",
    .s_Theme_CoverV = "垂直滚动", // vertical
    .s_Theme_CoverH = "水平滚动", // horizontal
    .s_Theme_CoverLightV = "垂直欢滚",
    .s_Theme_CoverLightH = "水平欢滚",
    //=====================================================================

    // Core\Src\retro-go\rg_emulators.c ====================================

    .s_File = "名称：",
    .s_Type = "类型：",
    .s_Size = "大小：",
    .s_ImgSize = "图像：",
    .s_Close = "× 关闭",
    .s_GameProp = "游戏文件属性",
    .s_Resume_game = "＞ 继续游戏",
    .s_New_game = "◇ 开始游戏",
    .s_Del_favorite = "☆ 移除收藏",
    .s_Add_favorite = "★ 添加收藏",
    .s_Delete_save = "□ 删除进度",
    .s_Confiem_del_save = "您确认要删除已保存的游戏进度？",
#if CHEAT_CODES == 1
    .s_Cheat_Codes = "＆ 金手指码",
    .s_Cheat_Codes_Title = "金手指",
    .s_Cheat_Codes_ON = "\x6",
    .s_Cheat_Codes_OFF = "\x5",
#endif

    //=====================================================================

    // Core\Src\retro-go\rg_main.c =========================================
    .s_CPU_Overclock = "超频",
    .s_CPU_Overclock_0 = "关闭",
    .s_CPU_Overclock_1 = "适度",
    .s_CPU_Overclock_2 = "极限",
    .s_CPU_OC_Upgrade_to = "升至",
    .s_CPU_OC_Downgrade_to = "降至",
    .s_CPU_OC_Stay_at = "保持",
    .s_Confirm_OC_Reboot = "超频配置需要重启后才能生效。您确认立即重启？",
#if INTFLASH_BANK == 2
    .s_Reboot = "重启",
    .s_Original_system = "原生系统",
    .s_Confirm_Reboot = "您确定要重启设备？",
#endif
    .s_Second_Unit = "秒",
    .s_Version = "版　　本：",
    .s_Author = "特别贡献：",
    .s_Author_ = "　　　　：",
    .s_UI_Mod = "界面美化：",
    .s_Lang = "简体中文：",
    .s_LangAuthor = "挠浆糊的",
    .s_Debug_menu = "≈ 调试信息",
    .s_Reset_settings = "≡ 重置设定",
    //.s_Close                  = "Close",
    .s_Retro_Go = "关于 Retro-Go",
    .s_Confirm_Reset_settings = "您确定要重置所有设定信息？",

    .s_Flash_JEDEC_ID = "存储 JEDEC ID",
    .s_Flash_Name = "存储芯片",
    .s_Flash_SR = "存储状态",
    .s_Flash_CR = "存储配置",
    .s_Smallest_erase = "最小单位",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "启用 DBGMCU CK",
    .s_Disable_DBGMCU_CK = "禁用 DBGMCU CK",
    //.s_Close                  = "Close",
    .s_Debug_Title = "调试选项",
	.s_Idle_power_off = "空闲待机",
    .s_Splash_Option = "启动动画",
    .s_Splash_On = "\x6",
    .s_Splash_Off = "\x5",
	
    .s_Time = "时间：",
    .s_Date = "日期：",
    .s_Time_Title = "时间",
    .s_Hour = "时：",
    .s_Minute = "分：",
    .s_Second = "秒：",
    .s_Time_setup = "时间设置",

    .s_Day = "日  ：",
    .s_Month = "月  ：",
    .s_Year = "年  ：",
    .s_Weekday = "星期：",
    .s_Date_setup = "日期设置",

    .s_Weekday_Mon = "一",
    .s_Weekday_Tue = "二",
    .s_Weekday_Wed = "三",
    .s_Weekday_Thu = "四",
    .s_Weekday_Fri = "五",
    .s_Weekday_Sat = "六",
    .s_Weekday_Sun = "日",

    .s_Turbo_Button = "连发",
    .s_Turbo_None = "无",
    .s_Turbo_A = "Ａ",
    .s_Turbo_B = "Ｂ",
    .s_Turbo_AB = "Ａ和Ｂ",    

    .s_Date_Format = "20%02d年%02d月%02d日 周%s",
    .s_Title_Date_Format = "%02d-%02d 周%s %02d:%02d:%02d",
    .s_Time_Format = "%02d:%02d:%02d",

    .fmt_Title_Date_Format = zh_cn_fmt_Title_Date_Format,
    .fmtDate = zh_cn_fmt_Date,
    .fmtTime = zh_cn_fmt_Time,
    //=====================================================================
};

#endif
