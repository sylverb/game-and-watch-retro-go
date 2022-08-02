/*
*********************************************************
*                Warning!!!!!!!                         *
*  This file must be saved with Windows 1252 Encoding   *
*********************************************************
*/
#if !defined (INCLUDED_RU_RU)
#define INCLUDED_RU_RU 1
#endif
#if INCLUDED_RU_RU == 1
//#include "rg_i18n_lang.h"
//Stand Russian

int ru_ru_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, day, month, weekday, hour, minutes, seconds);
};

int ru_ru_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, day, month, year, weekday);
};

int ru_ru_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_ru_ru LANG_DATA = {
  .codepage = 1252,
  .extra_font = NULL,
  .s_LangUI = "Язык интерфейса",
  .s_LangTitle = "Язык",
  .s_LangName = "Русский",
  // Core\Src\porting\gb\main_gb.c =======================================
  .s_Palette = "Палитра",
  //=====================================================================

  // Core\Src\porting\nes\main_nes.c
  //.s_Palette= "Palette" dul
  .s_Default = "По умолчанию",
  //=====================================================================

  // Core\Src\porting\gw\main_gw.c =======================================
  .s_copy_RTC_to_GW_time = "Копировать RTC в время G&W",
  .s_copy_GW_time_to_RTC = "Копировать время G&W в RTC",
  .s_LCD_filter = "Фильтр LCD",
  .s_Display_RAM = "Показать RAM",
  .s_Press_ACL = "Нажмите ACL или перезагрузите",
  .s_Press_TIME = "Нажмите TIME [B+TIME]",
  .s_Press_ALARM = "Нажмите ALARM [B+GAME]",
  .s_filter_0_none = "0-none",//?
  .s_filter_1_medium = "1-medium", //?
  .s_filter_2_high = "2-high", //?
  //=====================================================================
  
  // Core\Src\porting\odroid_overlay.c ==================================
  .s_Full = "\x7",
  .s_Fill = "\x8",

  .s_No_Cover = "Нет обложки",

  .s_Yes = "Да",
  .s_No = "Нет",
  .s_PlsChose = "Вопрос",
  .s_OK = "ОК",
  .s_Confirm = "Подтвердить",
  .s_Brightness = "Яркость",
  .s_Volume = "Громкость",
  .s_OptionsTit = "Опции",
  .s_FPS = "FPS",
  .s_BUSY = "BUSY",
  .s_Scaling = "Мастшабирование",
  .s_SCalingOff = "Выкл",
  .s_SCalingFit = "Заполнение", //?
  .s_SCalingFull = "Полное",
  .s_SCalingCustom = "Свое",
  .s_Filtering = "Фильтрация",
  .s_FilteringNone = "Нет",
  .s_FilteringOff = "Выкл",
  .s_FilteringSharp = "Резкая",
  .s_FilteringSoft = "Мягкая",
  .s_Speed = "Скорость",
  .s_Speed_Unit = "x",
  .s_Save_Cont = "Сохранить и продолжить",
  .s_Save_Quit = "Сохранить и выйти",
  .s_Reload = "Перезагрузить",
  .s_Options = "Опции",
  .s_Power_off = "Выключить",
  .s_Quit_to_menu = "Выйти в меню",
  .s_Retro_Go_options = "Retro-Go",

  .s_Font = "Шрифт",
  .s_Colors = "Цвета",
  .s_Theme_Title = "Тема интерфейса",
  .s_Theme_sList = "Простой список",
  .s_Theme_CoverV = "Coverflow V",
  .s_Theme_CoverH = "Coverflow H",
  .s_Theme_CoverLightV = "CoverLight V",
  .s_Theme_CoverLightH = "CoverLight H",
  //=====================================================================

  // Core\Src\retro-go\rg_emulators.c ====================================

  .s_File = "Файл",
  .s_Type = "Тип",
  .s_Size = "Размер",
  .s_ImgSize = "Размер изображения",
  .s_Close = "Закрыть",
  .s_GameProp = "Соотношение",
  .s_Resume_game = "Продолжить игру",
  .s_New_game = "Новая игра",
  .s_Del_favorite = "Удалить избранное",
  .s_Add_favorite = "Добавить избранное",
  .s_Delete_save = "Удалить сохранение",
  .s_Confiem_del_save = "Удалить файл сохранения?",
#if GAME_GENIE == 1
  .s_Game_Genie_Codes = "Game Genie Коды",
  .s_Game_Genie_Codes_Title = "GG Опции",
  .s_Game_Genie_Codes_ON = "\x6",
  .s_Game_Genie_Codes_OFF = "\x5",
#endif

  //=====================================================================
  
  // Core\Src\retro-go\rg_main.c =========================================
  .s_Second_Unit = "c",
  .s_Version = "Вер.",
  .s_Author = "от",
  .s_Author_ = "\t\t+",
  .s_UI_Mod = "UI Mod",
  .s_Lang = "Русский",
  .s_LangAuthor = "teuchezh",
  .s_Debug_menu = "Иеню отладки",
  .s_Reset_settings = "Сбросить настройки",
  //.s_Close                   = "Закрыть",
  .s_Retro_Go = "Об Retro-Go",
  .s_Confirm_Reset_settings = "Сбросить все настройки?",

  .s_Flash_JEDEC_ID = "Flash JEDEC ID",
  .s_Flash_Name = "Flash Name",
  .s_Flash_SR = "Flash SR",
  .s_Flash_CR = "Flash CR",
  .s_Smallest_erase = "Smallest erase",
  .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
  .s_Enable_DBGMCU_CK = "Enable DBGMCU CK",
  .s_Disable_DBGMCU_CK = "Disable DBGMCU CK",
  //.s_Close                   = "Close",
  .s_Debug_Title = "Отладка",
  .s_Idle_power_off = "Отключение в простое",
  .s_Splash_Option = "Splash animation",
  .s_Splash_On = "\x6",
  .s_Splash_Off = "\x5",

  .s_Time = "Время",
  .s_Date = "Дата",
  .s_Time_Title = "ВРЕМЯ",
  .s_Hour = "Часы",
  .s_Minute = "Минуты",
  .s_Second = "Секунды",
  .s_Time_setup = "Настройка времени",

  .s_Day = "День",
  .s_Month = "Месяц",
  .s_Year = "Год",
  .s_Weekday = "День недели",
  .s_Date_setup = "Настройка даты",

  .s_Weekday_Mon = "Пн",
  .s_Weekday_Tue = "Вт",
  .s_Weekday_Wed = "Ср",
  .s_Weekday_Thu = "Чт",
  .s_Weekday_Fri = "Пт",
  .s_Weekday_Sat = "Сб",
  .s_Weekday_Sun = "Вс",
  
  .s_Title_Date_Format = "%02d-%02d %s %02d:%02d:%02d",
  .s_Date_Format = "%02d.%02d.20%02d %s",
  .s_Time_Format = "%02d:%02d:%02d",
  
  .fmt_Title_Date_Format = ru_ru_fmt_Title_Date_Format,
  .fmtDate = ru_ru_fmt_Date,
  .fmtTime = ru_ru_fmt_Time,
  //=====================================================================
};

#endif