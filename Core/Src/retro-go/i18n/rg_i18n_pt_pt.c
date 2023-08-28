/*
*********************************************************
*                Warning!!!!!!!                         *
*  This file must be saved with Windows 1252 Encoding   *
*********************************************************
*/
#if !defined (INCLUDED_PT_PT)
#define INCLUDED_PT_PT 1
#endif
#if INCLUDED_PT_PT==1
//#include "rg_i18n_lang.h"
// Stand Portuguese


int pt_pt_fmt_Title_Date_Format(char *outstr, const char *datefmt, uint16_t day, uint16_t month, const char *weekday, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, datefmt, day, month, weekday, hour, minutes, seconds);
};

int pt_pt_fmt_Date(char *outstr, const char *datefmt, uint16_t day, uint16_t month, uint16_t year, const char *weekday)
{
    return sprintf(outstr, datefmt, day, month, year, weekday);
};

int pt_pt_fmt_Time(char *outstr, const char *timefmt, uint16_t hour, uint16_t minutes, uint16_t seconds)
{
    return sprintf(outstr, timefmt, hour, minutes, seconds);
};

const lang_t lang_pt_pt LANG_DATA = {
    .codepage = 1252,
    .extra_font = NULL,
    .s_LangUI = "Idioma",
    .s_LangName = "Portuguese",

    // Core\Src\porting\nes-fceu\main_nes_fceu.c ===========================
    .s_Crop_Vertical_Overscan = "Cortar Overscan Vertical",
    .s_Crop_Horizontal_Overscan = "Cortar Overscan Horizontal",
    .s_Disable_Sprite_Limit = "Desativar o limite de sprites",
    .s_NES_CPU_OC = "NES CPU Overclocking",
    .s_NES_Eject_Insert_FDS = "Ejetar/Inserir Disco",
    .s_NES_Eject_FDS = "Ejetar Disco",
    .s_NES_Insert_FDS = "Inserir Disco",
    .s_NES_Swap_Side_FDS = "Trocar lado do Disco",
    .s_NES_FDS_Side_Format = "Disco %d Lado %s",
    //=====================================================================

    // Core\Src\porting\gb\main_gb.c =======================================
    .s_Palette = "Paleta",
    //=====================================================================

    // Core\Src\porting\nes\main_nes.c =====================================
    //.s_Palette= "Palette" dul
    .s_Default = "Padr�o",
    //=====================================================================

    // Core\Src\porting\md\main_gwenesis.c ================================
    .s_md_keydefine = "Bot�es: A-B-C",
    .s_md_Synchro = "Sincroniza��o",
    .s_md_Synchro_Audio = "AUDIO",
    .s_md_Synchro_Vsync = "VSYNC",
    .s_md_Dithering = "Ru�do Branco",
    .s_md_Debug_bar = "Barra de Debug",
    .s_md_Option_ON = "\x6",
    .s_md_Option_OFF = "\x5",
    .s_md_AudioFilter = "Filtro de Audio",
    .s_md_VideoUpscaler = "Resolu��o de Video",
    //=====================================================================
    
    // Core\Src\porting\md\main_wsv.c ================================
    .s_wsv_palette_Default = "Por defeito",
    .s_wsv_palette_Amber = "Ambar",
    .s_wsv_palette_Green = "Verde",
    .s_wsv_palette_Blue = "Azul",
    .s_wsv_palette_BGB = "BGB",
    .s_wsv_palette_Wataroo = "Wataroo",
    //=====================================================================

    // Core\Src\porting\md\main_msx.c ================================
    .s_msx_Change_Dsk = "Trocar Disco",
    .s_msx_Select_MSX = "Selecionar MSX",
    .s_msx_MSX1_EUR = "MSX1(EUR)",
    .s_msx_MSX2_EUR = "MSX2(EUR)",
    .s_msx_MSX2_JP = "MSX2+(JP)",
    .s_msx_Frequency = "Frequ�ncia",
    .s_msx_Freq_Auto = "Auto",
    .s_msx_Freq_50 = "50Hz",
    .s_msx_Freq_60 = "60Hz",
    .s_msx_A_Button = "Bot�o A",
    .s_msx_B_Button = "Bot�o B",
    .s_msx_Press_Key = "Pressione bot�o",
    //=====================================================================

    // Core\Src\porting\md\main_amstrad.c ================================
    .s_amd_Change_Dsk = "Trocar Disco",
    .s_amd_Controls = "Controlos",
    .s_amd_Controls_Joystick = "Joystick",
    .s_amd_Controls_Keyboard = "Teclado",
    .s_amd_palette_Color = "Cor",
    .s_amd_palette_Green = "Verde",
    .s_amd_palette_Grey = "Cinza",
    .s_amd_Press_Key = "Pressione bot�o",
    //=====================================================================

    // Core\Src\porting\gw\main_gw.c =======================================
    .s_copy_RTC_to_GW_time = "Copiar RTC para hora G&W",
    .s_copy_GW_time_to_RTC = "Copiar hora G&W para RTC",
    .s_LCD_filter = "Filtro LCD",
    .s_Display_RAM = "Mostrar RAM",
    .s_Press_ACL = "Pressione ACL ou Reset",
    .s_Press_TIME = "Pressione TIME [B+TIME]",
    .s_Press_ALARM = "Pressione ALARM [B+GAME]",
    .s_filter_0_none = "0-nenhum",
    .s_filter_1_medium = "1-m�dio",
    .s_filter_2_high = "2-alto",
    //=====================================================================

    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",
    .s_No_Cover = "Sem capa",
    .s_Yes = "Sim",
    .s_No = "N�o",
    .s_PlsChose = "Aten��o",
    .s_OK = "OK",
    .s_Confirm = "Confirmar",
    .s_Brightness = "Brilho",
    .s_Volume = "Volume",
    .s_OptionsTit = "Op��es",
    .s_FPS = "FPS",
    .s_BUSY = "OCUPA��O",
    .s_Scaling = "Escala",
    .s_SCalingOff = "Desligado",
    .s_SCalingFit = "Ajustar",
    .s_SCalingFull = "Preencher",
    .s_SCalingCustom = "Personalizado",
    .s_Filtering = "Filtro",
    .s_FilteringNone = "Nenhum",
    .s_FilteringOff = "Desligado",
    .s_FilteringSharp = "Afiado",
    .s_FilteringSoft = "Suave",
    .s_Speed = "Velocidade",
    .s_Speed_Unit = "x",
    .s_Save_Cont = "Gravar e Continuar",
    .s_Save_Quit = "Gravar e Sair",
    .s_Reload = "Recarregar",
    .s_Options = "Op��es",
    .s_Power_off = "Desligar",
    .s_Quit_to_menu = "Sair para o menu",
    .s_Retro_Go_options = "Retro-Go",
    .s_Font = "Tipo de letra",
    .s_Colors = "Cores",
    .s_Theme_Title = "Tema UI",
    .s_Theme_sList = "Lista Simples",
    .s_Theme_CoverV = "Deslizante V",
    .s_Theme_CoverH = "Deslizante H",
    .s_Theme_CoverLightV = "Encadeado V",
    .s_Theme_CoverLightH = "Encadeado H",
    //=====================================================================

    // Core\Src\retro-go\rg_emulators.c ====================================
    .s_File = "Ficheiro",
    .s_Type = "Tipo",
    .s_Size = "ROM",
    .s_ImgSize = "Capa",
    .s_Close = "Fechar",
    .s_GameProp = "Propriedades",
    .s_Resume_game = "Resumir jogo",
    .s_New_game = "Novo jogo",
    .s_Del_favorite = "Apagar favorito",
    .s_Add_favorite = "Adicionar favorito",
    .s_Delete_save = "Apagar grava��o",
    .s_Confiem_del_save = "Apagar grava��o?",
#if CHEAT_CODES == 1
    .s_Cheat_Codes = "C�digo de Batota",
    .s_Cheat_Codes_Title = "Op��es de Batota",
    .s_Cheat_Codes_ON = "\x6",
    .s_Cheat_Codes_OFF = "\x5",
#endif
    //=====================================================================

    // Core\Src\retro-go\rg_main.c =========================================
    .s_CPU_Overclock = "CPU Overclock",
    .s_CPU_Overclock_0 = "Zero",
    .s_CPU_Overclock_1 = "Interm�dio",
    .s_CPU_Overclock_2 = "M�ximo",
    .s_CPU_OC_Upgrade_to = "Aumentar para ",
    .s_CPU_OC_Downgrade_to = "Diminuir para ",
    .s_CPU_OC_Stay_at = "Manter a ",
    .s_Confirm_OC_Reboot = "A configura��o do Overclock do CPU mudou e � necess�rio reiniciar. Tem a certeza?",
#if INTFLASH_BANK == 2
    .s_Reboot = "Reiniciar",
    .s_Original_system = "Reposi��o de Sistema",
    .s_Confirm_Reboot = "Confirmar rein�cio?",
#endif
    .s_Second_Unit = "s",
    .s_Version = "Ver.",
    .s_Author = "Por",
    .s_Author_ = "\t\t+",
    .s_UI_Mod = "UI Mod",
    .s_UI_Mod_ = "\t\t+",
    .s_Lang = "Portugu�s",
    .s_LangAuthor = "Pollux",
    .s_Debug_menu = "Menu de depura��o",
    .s_Reset_settings = "Rep�r configura��es",
    .s_Retro_Go = "Sobre Retro-Go",
    .s_Confirm_Reset_settings = "Rep�r configura��es?",
    .s_Flash_JEDEC_ID = "Flash JEDEC ID",
    .s_Flash_Name = "Flash Ref",
    .s_Flash_SR = "Flash SR",
    .s_Flash_CR = "Flash CR",
    .s_Smallest_erase = "Menor apagamento",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "Ativar DBGMCU CK",
    .s_Disable_DBGMCU_CK = "Desativar DBGMCU CK",
    .s_Debug_Title = "Depura��o",
    .s_Idle_power_off = "Desligar se inativo",
    .s_Time = "Hora",
    .s_Date = "Data",
    .s_Time_Title = "Data e Hora",
    .s_Hour = "Horas",
    .s_Minute = "Minutos",
    .s_Second = "Segundos",
    .s_Time_setup = "Acertar hora",
    .s_Day = "Dia",
    .s_Month = "M�s",
    .s_Year = "Ano",
    .s_Weekday = "Dia da semana",
    .s_Date_setup = "Acertar data",
    .s_Weekday_Mon = "Seg",
    .s_Weekday_Tue = "Ter",
    .s_Weekday_Wed = "Qua",
    .s_Weekday_Thu = "Qui",
    .s_Weekday_Fri = "Sex",
    .s_Weekday_Sat = "S�b",
    .s_Weekday_Sun = "Dom",
    .s_Turbo_Button = "Turbo",
    .s_Turbo_None = "Nenhum",
    .s_Turbo_A = "A",
    .s_Turbo_B = "B",
    .s_Turbo_AB = "A & B",
    .s_Title_Date_Format = "%02d-%02d %s %02d:%02d:%02d",
    .s_Date_Format = "%02d.%02d.20%02d %s",
    .s_Time_Format = "%02d:%02d:%02d",
    .fmt_Title_Date_Format = pt_pt_fmt_Title_Date_Format,
    .fmtDate = pt_pt_fmt_Date,
    .fmtTime = pt_pt_fmt_Time,
    //=====================================================================
    //           ------------ end ---------------
};

#endif
