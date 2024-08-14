#pragma GCC diagnostic ignored "-Wstack-usage="
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#pragma GCC diagnostic ignored "-Wchar-subscripts"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>


#if !defined (INCLUDED_ZH_CN)
#define INCLUDED_ZH_CN 0
#endif
#if !defined (INCLUDED_ZH_TW)
#define INCLUDED_ZH_TW 0
#endif
#if !defined (INCLUDED_JA_JP)
#define INCLUDED_JA_JP 0
#endif
#if !defined (INCLUDED_KO_KR)
#define INCLUDED_KO_KR 0
#endif
#if !defined (INCLUDED_ES_ES)
#define INCLUDED_ES_ES 1
#endif
#if !defined (INCLUDED_PT_PT)
#define INCLUDED_PT_PT 1
#endif
#if !defined (INCLUDED_FR_FR)
#define INCLUDED_FR_FR 1
#endif
#if !defined (INCLUDED_IT_IT)
#define INCLUDED_IT_IT 1
#endif
#if !defined (INCLUDED_DE_DE)
#define INCLUDED_DE_DE 1
#endif
#if !defined (INCLUDED_DE_DE)
#define INCLUDED_DE_DE 1
#endif
#if !defined (INCLUDED_RU_RU)
#define INCLUDED_RU_RU 1
#endif


#if !defined (BIG_BANK)
#define BIG_BANK 1
#endif

#include "rg_i18n.h"
#include "rg_i18n_lang.h"
#include "gw_lcd.h"
#include "main.h"
#include "odroid_system.h"
#include "odroid_overlay.h"

#if (BIG_BANK == 1) && (EXTFLASH_SIZE <= 16*1024*1024)
#define FONT_DATA
#else
#define FONT_DATA __attribute__((section(".extflash_font")))
#endif

#if (BIG_BANK == 1) && (EXTFLASH_SIZE <= 16*1024*1024)
#define LANG_DATA
#else
#define LANG_DATA __attribute__((section(".extflash_emu_data")))
#endif

#include "fonts/font_cp1252_Serif.h"

#if !SINGLE_FONT
#include "fonts/font_cp1252_Serif_Bold.h"
#include "fonts/font_cp1252_Serif_CJK.h"
#include "fonts/font_cp1252_Sans_serif.h"
#include "fonts/font_cp1252_Sans_serif_Bold.h"
#include "fonts/font_cp1252_Greybeard.h"
#include "fonts/font_cp1252_Unbalanced.h"
#include "fonts/font_cp1252_rock12.h"
#include "fonts/font_cp1252_haeberli12.h"
#endif

#if INCLUDED_JA_JP == 1
#include "fonts/font_cp932_ja_jp.h"
#endif
#if INCLUDED_ZH_CN == 1
#include "fonts/font_cp936_zh_cn.h"
#endif
#if INCLUDED_KO_KR == 1
#include "fonts/font_cp949_ko_kr.h"
#endif
#if INCLUDED_ZH_TW == 1
#include "fonts/font_cp950_zh_tw.h"
#endif
#if INCLUDED_RU_RU == 1
#include "fonts/font_cp1251_Serif.h"
#if !SINGLE_FONT
#include "fonts/font_cp1251_Serif_Bold.h"
#include "fonts/font_cp1251_Sans_serif.h"
#include "fonts/font_cp1251_Sans_serif_Bold.h"
#include "fonts/font_cp1251_Greybeard.h"
#endif
#endif

#if SINGLE_FONT
const char *gui_fonts[9] = {
    font_cp1252_Serif, font_cp1252_Serif, font_cp1252_Serif,
    font_cp1252_Serif, font_cp1252_Serif, font_cp1252_Serif,
    font_cp1252_Serif, font_cp1252_Serif, font_cp1252_Serif,
    };
#else
const char *gui_fonts[9] = {
    font_cp1252_Serif,    font_cp1252_Serif_Bold,    font_cp1252_Serif_CJK,
    font_cp1252_Sans_serif,    font_cp1252_Sans_serif_Bold,    font_cp1252_Greybeard,
    font_cp1252_Unbalanced,    font_cp1252_rock12,    font_cp1252_haeberli12,
    };
#endif


#if INCLUDED_JA_JP == 1
const char *ja_jp_fonts[9] = {
    font_cp932_ja_jp,    font_cp932_ja_jp,    font_cp932_ja_jp,
    font_cp932_ja_jp,    font_cp932_ja_jp,    font_cp932_ja_jp,
    font_cp932_ja_jp,    font_cp932_ja_jp,    font_cp932_ja_jp,
    };
#endif
#if INCLUDED_ZH_CN == 1
const char *zh_cn_fonts[9] = {
    font_cp936_zh_cn,    font_cp936_zh_cn,    font_cp936_zh_cn,
    font_cp936_zh_cn,    font_cp936_zh_cn,    font_cp936_zh_cn,
    font_cp936_zh_cn,    font_cp936_zh_cn,    font_cp936_zh_cn,
    };
#endif
#if INCLUDED_KO_KR == 1
const char *ko_kr_fonts[9] = {
    font_cp949_ko_kr,    font_cp949_ko_kr,    font_cp949_ko_kr,
    font_cp949_ko_kr,    font_cp949_ko_kr,    font_cp949_ko_kr,
    font_cp949_ko_kr,    font_cp949_ko_kr,    font_cp949_ko_kr,
    };
#endif
#if INCLUDED_ZH_TW == 1
const char *zh_tw_fonts[9] = {
    font_cp950_zh_tw,    font_cp950_zh_tw,    font_cp950_zh_tw,
    font_cp950_zh_tw,    font_cp950_zh_tw,    font_cp950_zh_tw,
    font_cp950_zh_tw,    font_cp950_zh_tw,    font_cp950_zh_tw,
    };
#endif
#if INCLUDED_RU_RU == 1
#if SINGLE_FONT
const char *cp1251_fonts[9] = {
    font_cp1251_Serif, font_cp1251_Serif, font_cp1251_Serif,
    font_cp1251_Serif, font_cp1251_Serif, font_cp1251_Serif,
    font_cp1251_Serif, font_cp1251_Serif, font_cp1251_Serif,
    };
#else
const char *cp1251_fonts[9] = {
    font_cp1251_Serif,    font_cp1251_Serif_Bold,    font_cp1251_Serif,
    font_cp1251_Sans_serif,    font_cp1251_Sans_serif_Bold,    font_cp1251_Greybeard,
    font_cp1251_Serif_Bold,    font_cp1251_Serif_Bold,    font_cp1251_Serif_Bold,
    };
#endif
#endif


#include "rg_i18n_en_us.c"

#if INCLUDED_ES_ES == 1
#include "rg_i18n_es_es.c"
#endif

#if INCLUDED_PT_PT == 1
#include "rg_i18n_pt_pt.c"
#endif

#if INCLUDED_FR_FR == 1
#include "rg_i18n_fr_fr.c"
#endif

#if INCLUDED_IT_IT == 1
#include "rg_i18n_it_it.c"
#endif

#if INCLUDED_DE_DE == 1
#include "rg_i18n_de_de.c"
#endif

#if INCLUDED_ZH_CN == 1
#include "rg_i18n_zh_cn.c"
#endif

#if INCLUDED_ZH_TW == 1
#include "rg_i18n_zh_tw.c"
#endif

#if INCLUDED_KO_KR == 1
#include "rg_i18n_ko_kr.c"
#endif

#if INCLUDED_JA_JP == 1
#include "rg_i18n_ja_jp.c"
#endif

#if INCLUDED_RU_RU == 1
#include "rg_i18n_ru_ru.c"
#endif

static uint16_t overlay_buffer[ODROID_SCREEN_WIDTH * 12 * 2] __attribute__((aligned(4)));

uint8_t curr_font = 0;

const int gui_font_count = FONT_COUNT;

const lang_t *gui_lang[] = {
    &lang_en_us,
#if INCLUDED_ES_ES == 1
    &lang_es_es,
#endif
#if INCLUDED_PT_PT == 1
    &lang_pt_pt,
#endif
#if INCLUDED_FR_FR == 1
    &lang_fr_fr,
#endif
#if INCLUDED_IT_IT == 1
    &lang_it_it,
#endif
#if INCLUDED_DE_DE == 1
    &lang_de_de,
#endif
#if INCLUDED_RU_RU == 1
    &lang_ru_ru,
#endif
#if INCLUDED_ZH_CN == 1
    &lang_zh_cn,
#endif
#if INCLUDED_ZH_TW == 1
    &lang_zh_tw,
#endif
#if INCLUDED_KO_KR == 1
    &lang_ko_kr,
#endif
#if INCLUDED_JA_JP == 1
    &lang_ja_jp,
#endif
};

lang_t *curr_lang = &lang_en_us;
const int gui_lang_count = sizeof(gui_lang) / sizeof(*gui_lang);

int i18n_get_text_height()
{
    return 12;
}

bool IS_CJK(const lang_t* lang)
{
    return (lang->codepage == 932) || (lang->codepage == 936) || (lang->codepage == 949) || (lang->codepage == 950);
};

int i18n_get_text_width(const char *text, const lang_t* lang)
{
    if (text == NULL || text[0] == 0)
        return 0;
    int text_len = strlen(text);
    int ret = 0;
    char *font = gui_fonts[curr_font];
    char *extra_font = gui_fonts[curr_font];
    if ((lang->extra_font != NULL) && (lang->extra_font[curr_font] != NULL))
            extra_font = lang->extra_font[curr_font];
    bool is_cjk = IS_CJK(lang);
    for (int i = 0; i < text_len; i++)
    {
        if (text[i] > 0x80)
        {
            if (is_cjk)
            {
                if ((lang->codepage == 932) && (text[i] > 0xa0) && (text[i] < 0xe0))
                    ret += 6;
                else if (text[i] < 0xa1)
                    ret += font[text[i]];
                else
                {
                    ret += 12;
                    i++;
                }
            }
            else
                ret += extra_font[text[i]];
        }
        else
            ret += font[text[i]];
    }
    return ret;
};

int i18n_get_text_lines(const char *text, const int fix_width, const lang_t* lang)
{
    if (text == NULL || text[0] == 0)
        return 0;
    int text_len = strlen(text);
    int w = 0;
    int ret = (text[0]) ? 1 : 0;
    int chr_width = 0;
    bool is_cjk = IS_CJK(lang);
    char *font = gui_fonts[curr_font];
    char *extra_font = gui_fonts[curr_font];
    if ((lang->extra_font != NULL) && (lang->extra_font[curr_font] != NULL))
            extra_font = lang->extra_font[curr_font];
    for (int i = 0; i < text_len; i++)
    {
        if (text[i] == 13)
            ret += 1;
        else if (text[i] == 10)
            w = 0;
        else
        {
            chr_width = 0;
            if (text[i] > 0x80)
            {
                if (is_cjk)
                {
                    if ((lang->codepage == 932) && (text[i] > 0xa0) && (text[i] < 0xe0))
                        chr_width = 6;
                    else if (text[i] < 0xa1)
                        ret += font[text[i]];
                    else
                    {
                        chr_width = 12;
                        i++;
                    }
                }
                else
                    chr_width = extra_font[text[i]];
            }
            else
                chr_width = font[text[i]];

            if ((fix_width - w) < chr_width)
            {
                w = 0;
                ret += 1;
            };
        }
        w += chr_width;
    }
    return ret;
}

void odroid_overlay_read_screen_rect(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t height)
{
    uint16_t *dst_img = (uint16_t *)(lcd_get_active_buffer());
    for (int x = 0; x < width; x++)
        for (int y = 0; y < height; y++)
            overlay_buffer[x + y * width] = dst_img[(y + y_pos) * ODROID_SCREEN_WIDTH + x_pos + x];
}

int i18n_draw_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg, char transparent, const lang_t* lang)
{
    if (text == NULL || text[0] == 0)
        return 0;
    int font_height = 12;
    int x_offset = 0;
    char realtxt[161];
    uint8_t cc;
    bool is_cjk = IS_CJK(lang);
    char *font = gui_fonts[curr_font];
    char *extra_font = gui_fonts[curr_font];
    if ((lang->extra_font != NULL) && (lang->extra_font[curr_font] != NULL))
            extra_font = lang->extra_font[curr_font];
    
    if (transparent)
        odroid_overlay_read_screen_rect(x_pos, y_pos, width, font_height);
    else
    {
        for (int x = 0; x < width; x++)
            for (int y = 0; y < font_height; y++)
                overlay_buffer[x + y * width] = color_bg;
    }
    int w = i18n_get_text_width(text, lang);
    sprintf(realtxt, "%.*s", 160, text);
    bool dByte = false;
    if (w > width)
    {
        w = 0;
        int i = 0;
        while (w < width)
        {
            dByte = false;
            if (realtxt[i] > 0x80)
            {
                if (is_cjk)
                {
                    if ((lang->codepage == 932) && (realtxt[i] > 0xa0) && (realtxt[i] < 0xe0))
                        w += 6;
                    else if (realtxt[i] < 0xa1)
                        w += font[realtxt[i]];
                    {
                        w += 12;
                        dByte = true;
                        i++;
                    }
                }
                else
                    w += extra_font[realtxt[i]];
            }
            else
                w += font[realtxt[i]];
            i++;
        }
        realtxt[i - (dByte ? 2 : 1)] = 0;
        // paint end point
        overlay_buffer[width * (font_height - 3) - 1] = get_darken_pixel(color, 80);
        overlay_buffer[width * (font_height - 3) - 3] = get_darken_pixel(color, 80);
        overlay_buffer[width * (font_height - 3) - 6] = get_darken_pixel(color, 80);
    };

    int text_len = strlen(realtxt);

    for (int i = 0; i < text_len; i++)
    {
        uint8_t c1 = realtxt[i];
        bool ofont = ((c1 > 0x80) && (lang->codepage == 932)) || ((c1 > 0xA0) && is_cjk); 
        if (! ofont)
        {
            char *draw_font = (c1 >= 0x80) ? (is_cjk ? font :extra_font) : font;
            int cw = draw_font[c1]; // width;
            if ((x_offset + cw) > width)
                break;
            if (cw != 0)
            {
                int d_pos = draw_font[c1 * 2 + 0x100] + draw_font[c1 * 2 + 0x101] * 0x100; // data pos
                int line_bytes = (cw + 7) / 8;
                for (int y = 0; y < font_height; y++)
                {
                    uint32_t *pixels_data = (uint32_t *)&(draw_font[0x300 + d_pos + y * line_bytes]);
                    int offset = x_offset + (width * y);

                    for (int x = 0; x < cw; x++)
                    {
                        if (pixels_data[0] & (1 << x))
                            overlay_buffer[offset + x] = color;
                    }
                }
            }
            x_offset += cw;
        }
        else
        {
            uint32_t location = 0;
            uint8_t c2 = text[i + 1];
            bool half = false;
            if (lang->codepage == 950) // zh_tw
                location = ((c1 - 0xa1) * 157 + ((c2 > 0xa0) ? (c2 - 0x62) : (c2 - 0x40))) * 24;
            else if (lang->codepage == 932) // ja_jp
            {
                half = (c1 > 0xa0) && (c1 < 0xe0);
                location = half ? ((c1 - 0xa1) * 12) : ((((c1 <= 0x9F) ? (c1 - 0x81) : (c1 - 0x41)) * 188 + ((c2 >= 0x80) ? (c2 - 0x41) : (c2 - 0x40)) + 32)  * 24 - 12);
            }
            else
                location = ((c1 - 0xa1) * 94 + (c2 - 0xa1)) * 24;
            for (int y = 0; y < font_height; y++)
            { // height :12;
                int offset = x_offset + (width * y);
                cc = extra_font[location + y * (half ? 1 : 2)];

                if (cc & 0x04)
                    overlay_buffer[offset + 5] = color;
                if (cc & 0x08)
                    overlay_buffer[offset + 4] = color;
                if (cc & 0x10)
                    overlay_buffer[offset + 3] = color;
                if (cc & 0x20)
                    overlay_buffer[offset + 2] = color;
                if (cc & 0x40)
                    overlay_buffer[offset + 1] = color;
                if (cc & 0x80)
                    overlay_buffer[offset + 0] = color;
                
                if (!half)
                {
                    if (cc & 0x01)
                        overlay_buffer[offset + 7] = color;
                    if (cc & 0x02)
                        overlay_buffer[offset + 6] = color;

                    cc = extra_font[location + y * 2 + 1];

                    if (cc & 0x10)
                        overlay_buffer[offset + 11] = color;
                    if (cc & 0x20)
                        overlay_buffer[offset + 10] = color;
                    if (cc & 0x40)
                        overlay_buffer[offset + 9] = color;
                    if (cc & 0x80)
                        overlay_buffer[offset + 8] = color;
                }
            }
            x_offset += half ? 6 : 12;
            if (! half)
                i++;
        }
    }
    odroid_display_write(x_pos, y_pos, width, font_height, overlay_buffer);
    return font_height;
}

int i18n_draw_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, uint16_t max_height, const char *text, uint16_t color, uint16_t color_bg, char transparent, const lang_t* lang)
{
    int text_len = 1;
    int height = 0;
    bool is_cjk = IS_CJK(lang);
    char *font = gui_fonts[curr_font];
    char *extra_font = gui_fonts[curr_font];
    if ((lang->extra_font != NULL) && (lang->extra_font[curr_font] != NULL))
            extra_font = lang->extra_font[curr_font];

    if (text == NULL || text[0] == 0)
        text = " ";

    text_len = strlen(text);
    if (x_pos < 0)
        x_pos = ODROID_SCREEN_WIDTH + x_pos;

    if (width < 1)
        width = i18n_get_text_width(text, lang);

    if (width > (ODROID_SCREEN_WIDTH - x_pos))
        width = (ODROID_SCREEN_WIDTH - x_pos);

    int line_len = 160; // min width is 2, max 160 char everline;
    char buffer[line_len + 1];

    for (int pos = 0; pos < text_len;)
    {
        if ((height + i18n_get_text_height()) > max_height)
            break;
        sprintf(buffer, "%.*s", line_len, text + pos);
        if (strchr(buffer, '\n'))
            *(strchr(buffer, '\n')) = 0;
        int w = 0;
        for (int x = 0; x < line_len; x++)
        {
            if (buffer[x] == 0)
                break;

            bool dByte = false;
            int chr_width = 0;
            if (buffer[x] > 0x80)
            {
                if (is_cjk)
                {
                    if ((lang->codepage == 932) && (buffer[x] > 0xa0) && (buffer[x] < 0xe0))
                        chr_width = 6;
                    else if (buffer[x] < 0xa1)
                        chr_width = font[buffer[x]];
                    {
                        chr_width = 12;
                        dByte = true;
                    }
                }
                else
                    chr_width = extra_font[buffer[x]];
            }
            else
                chr_width = font[buffer[x]];

            if ((width - w) < chr_width)
            {
                buffer[x] = 0;
                break;
            }
            w += chr_width;
            if (dByte)
                x ++;
        }
        height += i18n_draw_text_line(x_pos, y_pos + height, width, buffer, color, color_bg, transparent, lang);
        pos += strlen(buffer);
        if (*(text + pos) == 0 || *(text + pos) == '\n')
            pos++;
    }
    return height;
}
