/*
*********************************************************
*                Warning!!!!!!!                         *
*  This file must be saved with Windows 1252 Encoding   *
*********************************************************
*/
#if !defined (INCLUDED_ES_ES)
#define INCLUDED_ES_ES 1
#endif
#if INCLUDED_ES_ES == 1
//#include "rg_i18n_lang.h"
//Stand Spanish


int es_es_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, day, month, weekday, hour, minutes, seconds);
};

int es_es_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, day, month, year, weekday);
};

int es_es_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_es_es LANG_DATA = {
    .codepage = 1252,
    .extra_font = NULL,
    .s_LangUI = "Idioma",
    .s_LangName = "Spanish",

    // Core\Src\porting\nes-fceu\main_nes_fceu.c ===========================
    .s_Crop_Vertical_Overscan = "Cortar sobrebarrido Vertical",
    .s_Crop_Horizontal_Overscan = "Cortar sobrebarrido Horizontal",
    .s_Disable_Sprite_Limit = "Desactivar limite sprites",
    .s_NES_CPU_OC = "Overclocking de NES CPU",
    .s_NES_Eject_Insert_FDS = "Sacar/Insertar Disco",
    .s_NES_Eject_FDS = "Sacar Disco",
    .s_NES_Insert_FDS = "Insertar Disco",
    .s_NES_Swap_Side_FDS = "Cambiar cara Disco",
    .s_NES_FDS_Side_Format = "Disco %d cara %s",
    //=====================================================================

    // Core\Src\porting\gb\main_gb.c =======================================
    .s_Palette = "Paleta",
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette= "Palette" dul
    .s_Default = "Por defecto",
    //=====================================================================

    // Core\Src\porting\md\main_gwenesis.c ================================
    .s_md_keydefine = "Teclas: A-B-C",
    .s_md_Synchro = "Sincron�a",
    .s_md_Synchro_Audio = "AUDIO",
    .s_md_Synchro_Vsync = "VSYNC",
    .s_md_Dithering = "Tramado",
    .s_md_Debug_bar = "Barra depuraci�n",
    .s_md_Option_ON = "\x6",
    .s_md_Option_OFF = "\x5",
    .s_md_AudioFilter = "Filtro de audio",
    .s_md_VideoUpscaler = "Sobreescalar video",
    //=====================================================================

    // Core\Src\porting\md\main_wsv.c ================================
    .s_wsv_palette_Default = "Por defecto",
    .s_wsv_palette_Amber = "Ambar",
    .s_wsv_palette_Green = "Verde",
    .s_wsv_palette_Blue = "Azul",
    .s_wsv_palette_BGB = "BGB",
    .s_wsv_palette_Wataroo = "Wataroo",
    //=====================================================================

    // Core\Src\porting\md\main_msx.c ================================
    .s_msx_Change_Dsk = "Cambiar disco",
    .s_msx_Select_MSX = "Seleccionar MSX",
    .s_msx_MSX1_EUR = "MSX1 (EUR)",
    .s_msx_MSX2_EUR = "MSX2 (EUR)",
    .s_msx_MSX2_JP = "MSX2+ (JP)",
    .s_msx_Frequency = "Frecuencia",
    .s_msx_Freq_Auto = "Auto",
    .s_msx_Freq_50 = "50Hz",
    .s_msx_Freq_60 = "60Hz",
    .s_msx_A_Button = "Bot�n A",
    .s_msx_B_Button = "Bot�n B",
    .s_msx_Press_Key = "Pulsar tecla",
    //=====================================================================

    // Core\Src\porting\md\main_amstrad.c ================================
    .s_amd_Change_Dsk = "Cambiar disco",
    .s_amd_Controls = "Controles",
    .s_amd_Controls_Joystick = "Joystick",
    .s_amd_Controls_Keyboard = "Teclado",
    .s_amd_palette_Color = "Color",
    .s_amd_palette_Green = "Verde",
    .s_amd_palette_Grey = "Gris",
    .s_amd_Press_Key = "Pulsar tecla",
    //=====================================================================

    // Core\Src\porting\gw\main_gw.c =======================================
    .s_copy_RTC_to_GW_time = "Copiar RTC a hora G&W",
    .s_copy_GW_time_to_RTC = "Copiar hora G&W a RTC",
    .s_LCD_filter = "Filtro LCD",
    .s_Display_RAM = "Mostrar RAM",
    .s_Press_ACL = "Pulsar ACL o RESET",
    .s_Press_TIME = "Pulsar TIME [B+TIME]",
    .s_Press_ALARM = "Pulsar ALARM [B+GAME]",
    .s_filter_0_none = "0-Ninguno",
    .s_filter_1_medium = "1-Medio",
    .s_filter_2_high = "2-Alto",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",
    .s_No_Cover = "Sin imagen",
    .s_Yes = "Si",
    .s_No = "No",
    .s_PlsChose = "Pregunta",
    .s_OK = "OK",
    .s_Confirm = "Confirmar",
    .s_Brightness = "Brillo",
    .s_Volume = "Volumen",
    .s_OptionsTit = "Opciones",
    .s_FPS = "FPS",
    .s_BUSY = "OCUPADO",
    .s_Scaling = "Escalado",
    .s_SCalingOff = "Apagado",
    .s_SCalingFit = "Escala",
    .s_SCalingFull = "Completa",
    .s_SCalingCustom = "Personal",
    .s_Filtering = "Filtro",
    .s_FilteringNone = "Ninguno",
    .s_FilteringOff = "Apagado",
    .s_FilteringSharp = "Agudo",
    .s_FilteringSoft = "Suave",
    .s_Speed = "Velocidad",
    .s_Speed_Unit = "x",
    .s_Save_Cont = "Salvar y Continuar",
    .s_Save_Quit = "Salvar y Quitar",
    .s_Reload = "Recargar",
    .s_Options = "Opciones",
    .s_Power_off = "Apagar",
    .s_Quit_to_menu = "Volver al menu",
    .s_Retro_Go_options = "Retro-Go",
    .s_Font = "Tipo de letra",
    .s_Colors = "Colores",
    .s_Theme_Title = "UI Tema",
    .s_Theme_sList = "Listado",
    .s_Theme_CoverV = "Imagen Flow V",
    .s_Theme_CoverH = "Imagen Flow H",
    .s_Theme_CoverLightV = "Imagen simple V",
    .s_Theme_CoverLightH = "Imagen simple H",
    //=====================================================================

    // Core\Src\retro-go\rg_emulators.c ====================================
    .s_File = "Archivo",
    .s_Type = "Tipo",
    .s_Size = "Tama�o",
    .s_ImgSize = "Tama�o Imagen",
    .s_Close = "Cerrar",
    .s_GameProp = "Propiedades",
    .s_Resume_game = "Continuar",
    .s_New_game = "Nuevo juego",
    .s_Del_favorite = "Borrar favorito",
    .s_Add_favorite = "A�adir favorito",
    .s_Delete_save = "Borrar guardado",
    .s_Confiem_del_save = "�Borrar guardado?",
#if CHEAT_CODES == 1
    .s_Cheat_Codes = "C�digos Cheat",
    .s_Cheat_Codes_Title = "Opciones Cheat",
    .s_Cheat_Codes_ON = "\x6",
    .s_Cheat_Codes_OFF = "\x5",
#endif
    //=====================================================================

    // Core\Src\retro-go\rg_main.c =========================================
    .s_CPU_Overclock = "Overclock CPU",
    .s_CPU_Overclock_0 = "Stock",
    .s_CPU_Overclock_1 = "Medio",
    .s_CPU_Overclock_2 = "M�ximo",
    .s_CPU_OC_Upgrade_to = "Subir a ",
    .s_CPU_OC_Downgrade_to = "Bajar a ",
    .s_CPU_OC_Stay_at = "Permanecer en ",
    .s_Confirm_OC_Reboot = "Para cambiar la CPU se debe reiniciar �Est�s Seguro?",
#if INTFLASH_BANK == 2
    .s_Reboot = "Reiniciar",
    .s_Original_system = "Sistema original",
    .s_Confirm_Reboot = "�Confirmar Reinicio?",
#endif
    .s_Second_Unit = "s",
    .s_Version = "Ver.",
    .s_Author = "Por",
    .s_Author_ = "\t\t+",
    .s_UI_Mod = "UI Mod",
    .s_UI_Mod_ = "\t\t+",
    .s_Lang = "Espa�ol",
    .s_LangAuthor = "Ninoh-FOX",
    .s_Debug_menu = "Debug_menu",
    .s_Reset_settings = "Resetear configuraci�n",
    .s_Retro_Go = "Sobre Retro-Go",
    .s_Confirm_Reset_settings = "�Resetear?",
    .s_Flash_JEDEC_ID = "Flash JEDEC ID",
    .s_Flash_Name = "Flash Nombre",
    .s_Flash_SR = "Flash SR",
    .s_Flash_CR = "Flash CR",
    .s_Smallest_erase = "Menor borrado",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "Habilitar DBGMCU CK",
    .s_Disable_DBGMCU_CK = "Deshabilitar DBGMCU CK",
    .s_Debug_Title = "Debug",
    .s_Idle_power_off = "Apagado autom�tico",
    .s_Time = "Hora",
    .s_Date = "Fecha",
    .s_Time_Title = "Fecha y hora",
    .s_Hour = "Hora",
    .s_Minute = "Minuto",
    .s_Second = "Segundo",
    .s_Time_setup = "Conf. hora",
    .s_Day = "D�a",
    .s_Month = "Mes",
    .s_Year = "A�o",
    .s_Weekday = "D�a de la semana",
    .s_Date_setup = "Configurar fecha",
    .s_Weekday_Mon = "Lun",
    .s_Weekday_Tue = "Mar",
    .s_Weekday_Wed = "M�e",
    .s_Weekday_Thu = "Jue",
    .s_Weekday_Fri = "Vie",
    .s_Weekday_Sat = "S�b",
    .s_Weekday_Sun = "Dom",
    .s_Turbo_Button = "Turbo",
    .s_Turbo_None = "No",
    .s_Turbo_A = "A",
    .s_Turbo_B = "B",
    .s_Turbo_AB = "A & B",    
    .s_Title_Date_Format = "%02d-%02d %s %02d:%02d:%02d",
    .s_Date_Format = "%02d.%02d.20%02d %s",
    .s_Time_Format = "%02d:%02d:%02d",
    .fmt_Title_Date_Format = es_es_fmt_Title_Date_Format,
    .fmtDate = es_es_fmt_Date,
    .fmtTime = es_es_fmt_Time,
    //=====================================================================
    //           ------------ end ---------------
};

#endif
