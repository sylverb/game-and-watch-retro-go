/*
*********************************************************
*                Warning!!!!!!!                         *
*  This file must be saved with Windows 1252 Encoding   *
*********************************************************
*/
//#include "rg_i18n_lang.h"
// Stand Portuguese
#if !defined (INCLUDED_PT_PT)
#define INCLUDED_PT_PT 1
#endif

#if INCLUDED_PT_PT==1

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
    .s_LangUI = "Língua",
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
    .s_Default = "Padrão",
    //=====================================================================

    // Core\Src\porting\md\main_gwenesis.c ================================
    .s_md_keydefine = "botões: A-B-C",
    .s_md_Synchro = "Sincronização",
    .s_md_Synchro_Audio = "AUDIO",
    .s_md_Synchro_Vsync = "VSYNC",
    .s_md_Dithering = "Ruído Branco",
    .s_md_Debug_bar = "Barra de Debug",
    .s_md_Option_ON = "\x6",
    .s_md_Option_OFF = "\x5",
    .s_md_AudioFilter = "Filtro de Audio",
    .s_md_VideoUpscaler = "Resolução de Video",
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
    .s_msx_Frequency = "Frequência",
    .s_msx_Freq_Auto = "Auto",
    .s_msx_Freq_50 = "50Hz",
    .s_msx_Freq_60 = "60Hz",
    .s_msx_A_Button = "Botão A",
    .s_msx_B_Button = "Botão B",
    .s_msx_Press_Key = "Pressione botão",
    //=====================================================================

    // Core\Src\porting\md\main_amstrad.c ================================
    .s_amd_Change_Dsk = "Trocar Disco",
    .s_amd_Controls = "Controlos",
    .s_amd_Controls_Joystick = "Joystick",
    .s_amd_Controls_Keyboard = "Teclado",
    .s_amd_palette_Color = "Cor",
    .s_amd_palette_Green = "Verde",
    .s_amd_palette_Grey = "Cinza",
    .s_amd_Press_Key = "Pressione botão",
    //=====================================================================

    // Core\Src\porting\gw\main_gw.c =======================================
    .s_copy_RTC_to_GW_time = "copiar RTC para hora G&W",
    .s_copy_GW_time_to_RTC = "copiar hora G&W para RTC",
    .s_LCD_filter = "Filtro LCD",
    .s_Display_RAM = "Mostrar RAM",
    .s_Press_ACL = "Pressione ACL ou reset",
    .s_Press_TIME = "Pressione TIME [B+TIME]",
    .s_Press_ALARM = "Pressione ALARM [B+GAME]",
    .s_filter_0_none = "0-nenhum",
    .s_filter_1_medium = "1-médio",
    .s_filter_2_high = "2-alto",
    //=====================================================================
    // Core\Src\porting\odroid_overlay.c ===================================
    .s_Full = "\x7",
    .s_Fill = "\x8",
    .s_No_Cover = "sem capa",
    .s_Yes = "Sim",
    .s_No = "Não",
    .s_PlsChose = "Pergunta",
    .s_OK = "OK",
    .s_Confirm = "Confirmar",
    .s_Brightness = "Brilho",
    .s_Volume = "Volume",
    .s_OptionsTit = "Opções",
    .s_FPS = "FPS",
    .s_BUSY = "OCUPADO",
    .s_Scaling = "Escala",
    .s_SCalingOff = "Desligado",
    .s_SCalingFit = "Ajustar",
    .s_SCalingFull = "Completo",
    .s_SCalingCustom = "Personalizado",
    .s_Filtering = "Filtro",
    .s_FilteringNone = "Nenhum",
    .s_FilteringOff = "Desligado",
    .s_FilteringSharp = "Afiado",
    .s_FilteringSoft = "Suave",
    .s_Speed = "Velocidade",
    .s_Speed_Unit = "x",
    .s_Save_Cont = "Gravar & Continuar",
    .s_Save_Quit = "Gravar & Sair",
    .s_Reload = "Recarregar",
    .s_Options = "Opções",
    .s_Power_off = "Desligar",
    .s_Quit_to_menu = "Sair para o menu",
    .s_Retro_Go_options = "Retro-Go",
    .s_Font = "Tipo de letra",
    .s_Colors = "Cores",
    .s_Theme_Title = "Tema UI",
    .s_Theme_sList = "Lista Simples",
    .s_Theme_CoverV = "Capas deslizantes V",
    .s_Theme_CoverH = "Capas deslizantes H",
    .s_Theme_CoverLightV = "Capas encadeadas V",
    .s_Theme_CoverLightH = "Capas encadeadas H",
    //=====================================================================
    .s_File = "Ficheiro",
    .s_Type = "Tipo",
    .s_Size = "Tamanho",
    .s_ImgSize = "Tamanho da Imagem",
    .s_Close = "Fechar",
    .s_GameProp = "Propriedades",
    .s_Resume_game = "Resumir jogo",
    .s_New_game = "Novo jogo",
    .s_Del_favorite = "Apagar favorito",
    .s_Add_favorite = "Adicionar favorito",
    .s_Delete_save = "Apagar gravação",
    .s_Confiem_del_save = "Apagar ficheiro de gravação?",
#if CHEAT_CODES == 1
    .s_Cheat_Codes = "Código de Batota",
    .s_Cheat_Codes_Title = "Opções de Batota",
    .s_Cheat_Codes_ON = "\x6",
    .s_Cheat_Codes_OFF = "\x5",
#endif

    //=====================================================================
    // Core\Src\retro-go\rg_main.c =========================================
    .s_CPU_Overclock = "CPU Overclock",
    .s_CPU_Overclock_0 = "NÃO",
    .s_CPU_Overclock_1 = "Intermédio",
    .s_CPU_Overclock_2 = "Máximo",
    .s_CPU_OC_Upgrade_to = "Aumentar para ",
    .s_CPU_OC_Downgrade_to = "Diminuir para ",
    .s_CPU_OC_Stay_at = "Manter a ",
    .s_Confirm_OC_Reboot = "A configuração do Overclock do CPU mudou e é necessário reiniciar. Tem a certeza?",
#if INTFLASH_BANK == 2
    .s_Reboot = "Reinicial",
    .s_Original_system = "Sistema Original",
    .s_Confirm_Reboot = "Confirma o reinicio?",
#endif
    .s_Second_Unit = "s",
    .s_Version = "Ver.",
    .s_Author = "Por",
    .s_Author_ = "\t\t+",
    .s_UI_Mod = "UI Mod",
    .s_Lang = "Português",
    .s_LangAuthor = "DefKorns & PolluxPT",
    .s_Debug_menu = "Menu de Depuração",
    .s_Reset_settings = "Reiniciar configurações",
    //.s_Close                  = "Fechar",
    .s_Retro_Go = "Sobre Retro-Go",
    .s_Confirm_Reset_settings = "Reiniciar todas as configurações?",
    .s_Flash_JEDEC_ID = "Flash JEDEC ID",
    .s_Flash_Name = "Flash Name",
    .s_Flash_SR = "Flash SR",
    .s_Flash_CR = "Flash CR",
    .s_Smallest_erase = "Menor apagamento",
    .s_DBGMCU_IDCODE = "DBGMCU IDCODE",
    .s_Enable_DBGMCU_CK = "Ativar DBGMCU CK",
    .s_Disable_DBGMCU_CK = "Desativar DBGMCU CK",
    //.s_Close                  = "Fechar",
    .s_Debug_Title = "Depuração",
    .s_Idle_power_off = "Desligar por inatividade",

    .s_Time = "Horas",
    .s_Date = "Data",
    .s_Time_Title = "HORAS",
    .s_Hour = "Hora",
    .s_Minute = "Minutos",
    .s_Second = "Segundos",
    .s_Time_setup = "Acertar hora",
    .s_Day = "Dia",
    .s_Month = "Mês",
    .s_Year = "Ano",
    .s_Weekday = "Dia da semana",
    .s_Date_setup = "Acertar Data",
    .s_Weekday_Mon = "Seg",
    .s_Weekday_Tue = "Ter",
    .s_Weekday_Wed = "Qua",
    .s_Weekday_Thu = "Qui",
    .s_Weekday_Fri = "Sex",
    .s_Weekday_Sat = "Sáb",
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
