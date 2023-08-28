#if 0

// #include "bitmaps/font_basic.h"
#include "odroid_system.h"
#include "odroid_overlay.h"

int odroid_overlay_game_settings_menu(odroid_dialog_choice_t *extra_options)
{
    return 0;
}

int odroid_overlay_game_debug_menu(void)
{
    return 0;
}

int odroid_overlay_game_menu()
{
    return 0;
}

#else

#if !defined(COVERFLOW)
#define COVERFLOW 0
#endif /* COVERFLOW */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "gw_buttons.h"
#include "bitmaps/font_basic.h"
#include "gw_lcd.h"
#include "odroid_system.h"
#include "odroid_overlay.h"
#include "main.h"
#include "rom_manager.h"
#include "bitmaps.h"
#include "gui.h"
#include "rg_rtc.h"
#include "rg_i18n.h"
#include "main_msx.h"

static retro_emulator_file_t *CHOSEN_FILE = NULL;
// static uint16_t *overlay_buffer = NULL;
static uint16_t overlay_buffer[ODROID_SCREEN_WIDTH * 32 * 2] __attribute__((aligned(4)));
static short dialog_open_depth = 0;
static short font_size = 8;

void odroid_overlay_init()
{
    // overlay_buffer = (uint16_t *)rg_alloc(ODROID_SCREEN_WIDTH * 32 * 2, MEM_SLOW);
    odroid_overlay_set_font_size(odroid_settings_FontSize_get());
}

void odroid_overlay_set_font_size(int size)
{
    font_size = MAX(8, MIN(32, size));
    odroid_settings_FontSize_set(font_size);
}

int odroid_overlay_get_font_size()
{
    return font_size;
}

int odroid_overlay_get_font_width()
{
    return 8;
}

void odroid_overlay_draw_logo(uint16_t x_pos, uint16_t y_pos, const retro_logo_image *logo, uint16_t color)
{
    uint16_t *dst_img = lcd_get_active_buffer();
    int w = (logo->width + 7) / 8;
    for (int i = 0; i < w; i++)
        for (int y = 0; y < logo->height; y++)
        {
            const char glyph = logo->logo[y * w + i];
            //for (int x = 0; x < 8; x++)
            if (glyph & 0x80)
                dst_img[(y + y_pos) * 320 + i * 8 + 0 + x_pos] = color;
            if (glyph & 0x40)
                dst_img[(y + y_pos) * 320 + i * 8 + 1 + x_pos] = color;
            if (glyph & 0x20)
                dst_img[(y + y_pos) * 320 + i * 8 + 2 + x_pos] = color;
            if (glyph & 0x10)
                dst_img[(y + y_pos) * 320 + i * 8 + 3 + x_pos] = color;
            if (glyph & 0x08)
                dst_img[(y + y_pos) * 320 + i * 8 + 4 + x_pos] = color;
            if (glyph & 0x04)
                dst_img[(y + y_pos) * 320 + i * 8 + 5 + x_pos] = color;
            if (glyph & 0x02)
                dst_img[(y + y_pos) * 320 + i * 8 + 6 + x_pos] = color;
            if (glyph & 0x01)
                dst_img[(y + y_pos) * 320 + i * 8 + 7 + x_pos] = color;
        }
};

int odroid_overlay_draw_text_line(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg)
{
    int font_height = 8; //odroid_overlay_get_font_size();
    int font_width = 8;  //odroid_overlay_get_font_width();
    int x_offset = 0;
    //float scale = 1; //(float)font_height / 8;
    int text_len = strlen(text);

    for (int i = 0; i < (width / font_width); i++)
    {
        const char *glyph = font8x8_basic[(i < text_len) ? text[i] : ' '];
        for (int y = 0; y < font_height; y++)
        {
            int offset = x_offset + (width * y);
            for (int x = 0; x < 8; x++)
                overlay_buffer[offset + x] = (glyph[y] & (1 << x)) ? color : color_bg;
        }
        x_offset += font_width;
    }

    odroid_display_write(x_pos, y_pos, width, font_height, overlay_buffer);

    return font_height;
}

int odroid_overlay_draw_text(uint16_t x_pos, uint16_t y_pos, uint16_t width, const char *text, uint16_t color, uint16_t color_bg)
{
    int text_len = 1;
    int height = 0;

    if (text == NULL || text[0] == 0)
        text = " ";

    text_len = strlen(text);

    if (width < 1)
        width = text_len * odroid_overlay_get_font_width();

    if (width > (ODROID_SCREEN_WIDTH - x_pos))
        width = (ODROID_SCREEN_WIDTH - x_pos);

    int line_len = width / odroid_overlay_get_font_width();
    char buffer[ODROID_SCREEN_WIDTH / 8 + 1];

    for (int pos = 0; pos < text_len;)
    {
        sprintf(buffer, "%.*s", line_len, text + pos);
        if (strchr(buffer, '\n'))
            *(strchr(buffer, '\n')) = 0;
        height += odroid_overlay_draw_text_line(x_pos, y_pos + height, width, buffer, color, color_bg);
        pos += strlen(buffer);
        if (*(text + pos) == 0 || *(text + pos) == '\n')
            pos++;
    }

    return height;
}

void odroid_overlay_draw_rect(int x, int y, int width, int height, int border, uint16_t color)
{
    if (width == 0 || height == 0 || border == 0)
        return;

    int pixels = (width > height ? width : height) * border;
    for (int i = 0; i < pixels; i++)
        overlay_buffer[i] = color;
    odroid_display_write(x, y, width, border, overlay_buffer);                   // T
    odroid_display_write(x, y + height - border, width, border, overlay_buffer); // B
    odroid_display_write(x, y, border, height, overlay_buffer);                  // L
    odroid_display_write(x + width - border, y, border, height, overlay_buffer); // R
}

void odroid_overlay_draw_fill_rect(int x, int y, int width, int height, uint16_t color)
{
    if (width == 0 || height == 0)
        return;

    for (int i = 0; i < width * 16; i++)
        overlay_buffer[i] = color;

    int y_pos = y;
    int y_end = y + height;

    while (y_pos < y_end)
    {
        int thickness = (y_end - y_pos >= 16) ? 16 : (y_end - y_pos);
        odroid_display_write(x, y_pos, width, thickness, overlay_buffer);
        y_pos += 16;
    }
}


static void draw_clock_digit(uint16_t *fb, const uint8_t clock, uint16_t px, uint16_t py, uint16_t color)
{
    static const unsigned char *CLOCK_DIGITS[] = {img_clock_00, img_clock_01, img_clock_02, img_clock_03, img_clock_04, img_clock_05, img_clock_06, img_clock_07, img_clock_08, img_clock_09};
    const unsigned char *img = CLOCK_DIGITS[clock];
    for (uint8_t y = 0; y < 10; y++)
        for (uint8_t x = 0; x < 6; x++)
            if (img[y] & (1 << (7 - x)))
                fb[px + x + GW_LCD_WIDTH * (py + y)] = color;
};

void odroid_overlay_clock(int x_pos, int y_pos)
{
    uint16_t *dst_img = lcd_get_active_buffer();
    HAL_RTC_GetTime(&hrtc, &GW_currentTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &GW_currentDate, RTC_FORMAT_BIN);

    uint16_t color = curr_colors->sel_c;
    draw_clock_digit(dst_img, GW_currentTime.Minutes % 10, x_pos + 30, y_pos, color);
    draw_clock_digit(dst_img, GW_currentTime.Minutes / 10, x_pos + 22, y_pos, color);
    draw_clock_digit(dst_img, GW_currentTime.Hours % 10, x_pos + 8, y_pos, color);
    draw_clock_digit(dst_img, GW_currentTime.Hours / 10, x_pos, y_pos, color);

    if (GW_currentTime.SubSeconds < 100)
    {
        odroid_overlay_draw_fill_rect(x_pos + 17, y_pos + 2, 2, 2, color);
        odroid_overlay_draw_fill_rect(x_pos + 17, y_pos + 6, 2, 2, color);
    }
};


void odroid_overlay_draw_battery(int x_pos, int y_pos)
{
    uint16_t percentage = odroid_input_read_battery().percentage;
    odroid_battery_charge_state_t battery_state = odroid_input_read_battery().state;
    uint16_t color_fill = curr_colors->sel_c;
    uint16_t color_border = curr_colors->sel_c;
    uint16_t color_empty = curr_colors->main_c;
    uint16_t width_fill = 14 * percentage / 100;
    uint16_t width_empty = 16; // - width_fill;
    const unsigned char IMG_C_OUT[] = {
        // width8, height:10
        0x0e, //  ____###_
        0x1e, //  ___####_
        0x3c, //  __####__
        0x7f, //  _#######
        0xff, //  ########
        0xff, //  ########
        0xfe, //  #######_
        0x3c, //  __####__
        0x78, //  _####___
        0x70, //  _###____
    };
    const unsigned char IMG_C[] = {
        // width8, height:10
        0x00, //  ________
        0x04, //  _____#__
        0x08, //  ____#___
        0x18, //  ___##___
        0x3e, //  __#####_
        0x7c, //  _#####__
        0x18, //  ___##___
        0x10, //  ___#____
        0x20, //  __#_____
        0x00, //  ________
    };

    if (percentage < 20) color_fill = C_RED;
    else if (percentage < 40)
        color_fill = C_ORANGE;

    odroid_overlay_draw_rect(x_pos, y_pos, 18, 10, 1, color_border);
    odroid_overlay_draw_rect(x_pos + 17, y_pos + 3, 1, 4, 1, color_empty);
    odroid_overlay_draw_rect(x_pos + 18, y_pos + 2, 1, 6, 1, color_border);
    odroid_overlay_draw_rect(x_pos + 19, y_pos + 3, 1, 4, 1, color_border);
    //odroid_overlay_draw_fill_rect(x_pos + 1, y_pos + 1, width_empty, 8, color_empty);
    //odroid_overlay_draw_fill_rect(x_pos + 2, y_pos + 2, width_fill, 6, color_fill);
    pixel_t *dest = lcd_get_active_buffer();

    switch (battery_state)
    {
    case ODROID_BATTERY_CHARGE_STATE_BATTERY_MISSING:
    case ODROID_BATTERY_CHARGE_STATE_CHARGING:
        odroid_overlay_draw_fill_rect(x_pos + 2, y_pos + 2, width_fill, 6, (battery_state == ODROID_BATTERY_CHARGE_STATE_BATTERY_MISSING) ? 0x00 : 0x07E0);
        if ((get_elapsed_time() % 1000) < 800)
        {
            for (int y = 0; y < 10; y++)
            {
                for (int x = 0; x < 8; x++)
                {
                    if (IMG_C[y] & (0x80 >> x))
                    {
                        dest[(y_pos + y) * ODROID_SCREEN_WIDTH + x_pos + 5 + x] = 0x0FFFF;
                    }
                    else if (IMG_C_OUT[y] & (0x80 >> x))
                        dest[(y_pos + y) * ODROID_SCREEN_WIDTH + x_pos + 5 + x] = color_empty;
                };
            };
        };
        //
        break;
    case ODROID_BATTERY_CHARGE_STATE_DISCHARGING:
        odroid_overlay_draw_fill_rect(x_pos + 2, y_pos + 2, width_fill, 6, color_fill);
        //odroid_overlay_draw_fill_rect(x_pos + 22 / 2 - 3, y_pos + 10 / 2 - 1, 6, 2, color_battery);
        break;
    case ODROID_BATTERY_CHARGE_STATE_FULL:
        odroid_overlay_draw_fill_rect(x_pos + 2, y_pos + 2, width_empty - 2, 6, 0x07E0);
        break;
    }
}

static int get_dialog_items_count(odroid_dialog_choice_t *options)
{
    odroid_dialog_choice_t last = ODROID_DIALOG_CHOICE_LAST;

    if (options == NULL)
        return 0;

    for (int i = 0; i < 17; i++)
        if (options[i].id == last.id && options[i].enabled == last.enabled)
            return i;
    return 0;
}

uint16_t get_darken_pixel_d(uint16_t color, uint16_t color1, uint16_t darken)
{
    int16_t r = (color1 & 0xF800);
    if ((color & 0xF800) > (color1 & 0xF800)) 
        r = (int16_t)((color1 & 0xF800) + (((color & 0xF800) - (color1 & 0xF800)) * darken / 100)) & 0xF800;
    int16_t g = (color1 & 0x7E0);
    if ((color & 0x07E0) > (color1 & 0x07E0)) 
        g = (int16_t)((color1 & 0x07E0) + (((color & 0x07E0) - (color1 & 0x07E0)) * darken / 100)) & 0x07E0;
    int16_t b = (color1 & 0x1F);
    if ((color & 0x001F) > (color1 & 0x001F)) 
        b = (int16_t)((color1 & 0x001F) + (((color & 0x001F) - (color1 & 0x001F)) * darken / 100)) & 0x001F;
    return r | g | b;
}

uint16_t get_darken_pixel(uint16_t color, uint16_t darken)
{
    int16_t r = (int16_t)((color & 0b1111100000000000) * darken / 100) & 0b1111100000000000;
    int16_t g = (int16_t)((color & 0b0000011111100000) * darken / 100) & 0b0000011111100000;
    int16_t b = (int16_t)((color & 0b0000000000011111) * darken / 100) & 0b0000000000011111;
    return r | g | b;
}

uint16_t get_shined_pixel(uint16_t color, uint16_t shined)
{
    int16_t r = (int16_t)((color & 0b1111100000000000) + (0b1111100000000000 - (color & 0b1111100000000000)) / 100 * shined) & 0b1111100000000000;
    int16_t g = (int16_t)((color & 0b0000011111100000) + (0b0000011111100000 - (color & 0b0000011111100000)) / 100 * shined) & 0b0000011111100000;
    int16_t b = (int16_t)((color & 0b0000000000011111) + (0b0000000000011111 - (color & 0b0000000000011111)) * shined / 100) & 0b0000000000011111;
    return r | g | b;
}

void odroid_overlay_darken_all()
{
    if (dialog_open_depth <= 0)
    { //darken bg
        uint16_t mgic = 0b0000100000100001;
        uint16_t *dst_img = lcd_get_active_buffer();
        if ((dst_img[0] == mgic) || is_lcd_swap_pending())
            return;
        for (int y = 0; y < ODROID_SCREEN_HEIGHT; y++)
            for (int x = 0; x < ODROID_SCREEN_WIDTH; x++)
                dst_img[y * ODROID_SCREEN_WIDTH + x] = get_darken_pixel(dst_img[y * ODROID_SCREEN_WIDTH + x], 40);

        dst_img[0] = mgic;
        lcd_sync();
    }
}

void odroid_overlay_draw_dialog(const char *header, odroid_dialog_choice_t *options, int sel)
{

    odroid_overlay_darken_all();

    int row_margin = 1;
    int row_height = i18n_get_text_height() + row_margin * 2;
    int box_width = 64;
    int box_height = 64;
    int box_padding = 6;
    int line_padding = 4;
    int cen_space = 12;
    int box_color = curr_colors->bg_c;
    int box_border_color = curr_colors->dis_c;
    int box_text_color = curr_colors->sel_c;
    odroid_dialog_choice_t separator = ODROID_DIALOG_CHOICE_SEPARATOR;

    int options_count = get_dialog_items_count(options);

    uint16_t *i_left = rg_alloc(options_count * 2, MEM_ANY);
    uint16_t *i_right = rg_alloc(options_count * 2, MEM_ANY);
    uint16_t *i_width = rg_alloc(options_count * 2, MEM_ANY);
    uint8_t *i_height = rg_alloc(options_count * 1, MEM_ANY);
    
    uint16_t max_left = 0;
    uint16_t max_right = 0;
    uint16_t max_width = 0;
    uint16_t max_height = 0; 
    bool had_right = false;
    bool had_extent = false;

    for (int i = 0; i < options_count; i++)
        if (options[i].update_cb != NULL)
            options[i].update_cb(&options[i], ODROID_DIALOG_INIT, sel); // A hack to transport currently selected item

    for (int i = 0; i < options_count; i++)
    {
        if (options[i].value[0])
        {
            i_left[i] = i18n_get_text_width(options[i].label, curr_lang);
            i_right[i] = i18n_get_text_width(options[i].value, curr_lang);
            i_width[i] = i_left[i] + i_right[i] + cen_space; //center space
            had_right = true;
            if (max_left < i_left[i])
                max_left = i_left[i];
            if (max_right < i_right[i])
                max_right = i_right[i];
            if (max_width < i_width[i])
                max_width = i_width[i];
            if (max_width < (max_left + max_right + cen_space))
                max_width = max_left + max_right +  cen_space;
        }
        else
        {
            i_left[i] = 0;
            i_right[i] = 0;
            i_width[i] = i18n_get_text_width(options[i].label, curr_lang);
            if (max_width < i_width[i])
                max_width = i_width[i];
        }
        i_height[i] = row_height;
    };

    if (max_width > (ODROID_SCREEN_WIDTH - 40)) 
    {
        max_width = ODROID_SCREEN_WIDTH - 40;
        if (had_right)
            max_width -= cen_space;
        if (max_left > (max_width * 2 / 5)) 
            max_left = max_width * 2 / 5;
        if (max_left < (max_width - max_right))
            max_left = (max_width - max_right);

        if (max_right > (max_width * 3 / 5)) 
            max_right = max_width * 3 / 5;
        if (max_right < (max_width - max_left))
            max_right = (max_width - max_left);
        if (had_right)
            max_width += cen_space;
    };

    for (int i = 0; i < options_count; i++)
    {
        if (options[i].value[0])
        {
            if (i_right[i] > max_right)
            {
                i_right[i] = max_right;
                int linecount = i18n_get_text_lines(options[i].value, max_right, curr_lang);
                if (linecount > 8)
                    linecount = 8;
                i_height[i] = linecount * i18n_get_text_height() + row_margin * 2; 
            }
        }
        else
        {
            if (i_width[i] > max_width)
            {
                i_width[i] = max_width;
                int linecount = i18n_get_text_lines(options[i].label, max_width, curr_lang);
                if (linecount > 8)
                    linecount = 8;
                i_height[i] = linecount * i18n_get_text_height() + row_margin * 2; 
            }
        }
        if (options[i].id == separator.id)
            i_height[i] = row_margin * 2 + 5;

        max_height += i_height[i];
    };

    box_width = max_width + box_padding * 2 + line_padding * 2 ;
    box_height = max_height + (header ? row_height + 8 : 0) + box_padding * 2;

    had_extent = box_height > (ODROID_SCREEN_HEIGHT - 10);

    int box_x = (ODROID_SCREEN_WIDTH - box_width) / 2;
    int box_y = (ODROID_SCREEN_HEIGHT - box_height) / 2;
    int start_index = 0;
    int end_index = options_count - 1;
    bool had_prior = false;
    bool had_next = false;

    if (had_extent) 
    {
        start_index = sel;
        if (start_index < 0)
            start_index = 0;
        int max_bh = ODROID_SCREEN_HEIGHT - 20 - (header ? row_height + 8 : 0) - box_padding * 2;
        max_bh -= i_height[start_index];
        box_height += i_height[start_index];
        while ((max_bh > 0) && (start_index > 0))
        {
            start_index --;
            max_bh -= i_height[start_index];
            box_height += i_height[start_index];
            if (max_bh < 0) 
            {
              box_height -= i_height[start_index];
              start_index ++;
            }
        };
        
        box_height = (header ? row_height + 8 : 0) + box_padding * 2;
        max_bh = ODROID_SCREEN_HEIGHT - 20;
        //max height;
        for (int i = start_index; i < options_count; i ++)
        {
            if ((box_height + i_height[i]) <= max_bh)
            {
                box_height += i_height[i];
                end_index = i;
            }
        }
        had_prior = start_index > 0;
        had_next = end_index < (options_count - 1);
        if (had_prior) 
            box_height += 5;
        if (had_next) 
            box_height += 5;
        box_y = (ODROID_SCREEN_HEIGHT - box_height) / 2;
    }; 

    int x = box_x + box_padding;
    int y = box_y + box_padding;


    uint16_t fg, bg, color, inner_width = box_width - (box_padding * 2);
    if (header)
    {
        odroid_overlay_draw_rect(box_x - 1, box_y - 1, box_width + 2, row_height + 8, 1, box_border_color);
        odroid_overlay_draw_fill_rect(box_x, box_y, box_width, row_height + 7, curr_colors->main_c);
        i18n_draw_text_line(x, box_y + 5, inner_width - 8, header, curr_colors->sel_c, curr_colors->main_c, 0, curr_lang);
        odroid_overlay_draw_fill_rect(x + inner_width - 2, box_y + 5, 4, 4, curr_colors->sel_c);
        odroid_overlay_draw_fill_rect(x + inner_width, box_y + 11, 2, 4, curr_colors->dis_c);
        y += row_height + 8;
    }

    if (had_prior) 
    {
        odroid_overlay_draw_fill_rect(x, y, inner_width, 5, curr_colors->bg_c);
        odroid_overlay_draw_fill_rect(x + 16, y + 2, inner_width - 32, 1, get_darken_pixel(curr_colors->main_c,70));
        odroid_overlay_draw_fill_rect(x + 10, y + 3, inner_width - 20, 1, curr_colors->main_c);
        odroid_overlay_draw_fill_rect(x + 6, y + 4, inner_width - 12, 1, get_darken_pixel(box_border_color, 80));
        y += 5;
    }

    for (int i = start_index; i <= end_index; i++)
    {
        color = options[i].enabled == 1 ? box_text_color : curr_colors->dis_c;
        if (options[i].enabled == 1)
        {
            fg = (i == sel) ? box_color : color;
            bg = (i == sel) ? color : box_color;
        }
        else
        {
            fg = color;
            bg = curr_colors->bg_c;
        }

        odroid_overlay_draw_fill_rect(x, y, inner_width, i_height[i] + 2 * row_margin, bg);
        if (options[i].id == separator.id)
        {
            odroid_overlay_draw_fill_rect(x, y, inner_width, i_height[i] + 2 * row_margin, curr_colors->bg_c);
            odroid_overlay_draw_fill_rect(x + 6, y + row_margin + 2, inner_width - 12, 1, box_border_color);
        }
        else
        {
            if (options[i].id == 0x0F0F0E0E) //color select
            {
                i18n_draw_text_line(x + line_padding, y + row_margin, max_left, options[i].label, fg, bg, 0, curr_lang);
                uint16_t *color = (uint16_t *)(options[i].value);
                int c_b_w = (max_right - 2) / 4;
                for (int j=0; j < 4; j++) {
                    odroid_overlay_draw_fill_rect(
                        x + inner_width - line_padding - (4 - j) * c_b_w - 1, 
                        y + row_margin, 
                        c_b_w, 
                        i_height[i] - 2 * row_margin, 
                        color[j + 1]);
                };
                odroid_overlay_draw_rect(x + inner_width - line_padding - 4 * c_b_w - 2, y + row_margin, 4 * c_b_w + 2, i_height[i] - 2 * row_margin, 1, fg);
            }
            else
            {
                if (options[i].value[0])
                {
                    i18n_draw_text_line(x + line_padding, y + row_margin, max_left, options[i].label, fg, bg, 0, curr_lang);
                    int txt_w = i18n_get_text_width(options[i].value, curr_lang);
                    txt_w = (txt_w > max_right) ? max_right : txt_w;  
                    i18n_draw_text(
                        x + inner_width - line_padding - txt_w, 
                        y + row_margin, 
                        txt_w, i_height[i],
                         options[i].value, fg, bg, 0, curr_lang);
                    //left,right paint;
                } 
                else  //single paint;
                    i18n_draw_text(
                        x + line_padding, 
                        y + row_margin, 
                        inner_width - 2 * line_padding, 
                        i_height[i], 
                        options[i].label, fg, bg, 0, curr_lang);

            }
        }
        y += i_height[i];
    };
    if (had_next) {
        odroid_overlay_draw_fill_rect(x, y, inner_width, 5, curr_colors->bg_c);
        odroid_overlay_draw_fill_rect(x + 6, y + 1, inner_width - 12, 1, get_darken_pixel(box_border_color, 80));
        odroid_overlay_draw_fill_rect(x + 10, y + 2, inner_width - 20, 1, curr_colors->main_c);
        odroid_overlay_draw_fill_rect(x + 16, y + 3, inner_width - 32, 1, get_darken_pixel(curr_colors->main_c, 70));
        y += 5;
    }

    box_y = header ? box_y + row_height + 8 : box_y;
    box_height = y - box_y + box_padding;
    odroid_overlay_draw_rect(box_x, box_y, box_width, box_height, box_padding, box_color);
    odroid_overlay_draw_rect(box_x - 1, box_y - 1, box_width + 2, box_height + 2, 1, box_border_color);

    rg_free(i_left);
    rg_free(i_right);
    rg_free(i_width);
    rg_free(i_height);
}

int odroid_overlay_dialog(const char *header, odroid_dialog_choice_t *options, int selected)
{
    int options_count = get_dialog_items_count(options);
    int sel = selected < 0 ? (options_count + selected) : selected;
    int sel_old = sel;
    int last_key = -1;
    int repeat = 0;
    bool select = false;
    odroid_gamepad_state_t joystick;

    lcd_sync();
    lcd_swap();
    HAL_Delay(20);
    odroid_overlay_draw_dialog(header, options, sel);
    dialog_open_depth++;

    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY))
        wdog_refresh();

    while (1)
    {
        wdog_refresh();
        odroid_input_read_gamepad(&joystick);
        if (last_key < 0 || ((repeat >= 30) && (repeat % 5 == 0)))
        {
            if (joystick.values[ODROID_INPUT_UP])
            {
                last_key = ODROID_INPUT_UP;
                if (--sel < 0)
                    sel = options_count - 1;
                repeat++;
            }
            else if (joystick.values[ODROID_INPUT_DOWN])
            {
                last_key = ODROID_INPUT_DOWN;
                if (++sel > options_count - 1)
                    sel = 0;
                repeat++;
            }
            else if (joystick.values[ODROID_INPUT_B])
            {
                last_key = ODROID_INPUT_B;
                sel = -1;
                break;
            }
            else if (joystick.values[ODROID_INPUT_VOLUME])
            {
                last_key = ODROID_INPUT_VOLUME;
                sel = -1;
                break;
            }
            else if (joystick.values[ODROID_INPUT_MENU])
            {
                last_key = ODROID_INPUT_MENU;
                sel = -1;
                break;
            }
            else if (joystick.values[ODROID_INPUT_POWER])
            {
                sel = -1;
#if OFF_SAVESTATE == 1
                // Slot 1 is a common slot used only for power off/power on
                odroid_system_emu_save_state(1);
#else
                odroid_system_emu_save_state(0);
#endif
                odroid_system_sleep();
                break;
            }
            if (options[sel].enabled)
            {
                select = false;
                if (joystick.values[ODROID_INPUT_LEFT])
                {
                    last_key = ODROID_INPUT_LEFT;
                    if (options[sel].update_cb != NULL)
                    {
                        select = options[sel].update_cb(&options[sel], ODROID_DIALOG_PREV, repeat);
                        sel_old = -1;
                    }
                    repeat++;
                }
                else if (joystick.values[ODROID_INPUT_RIGHT])
                {
                    last_key = ODROID_INPUT_RIGHT;
                    if (options[sel].update_cb != NULL)
                    {
                        select = options[sel].update_cb(&options[sel], ODROID_DIALOG_NEXT, repeat);
                        sel_old = -1;
                    }
                    repeat++;
                }
                else if (joystick.values[ODROID_INPUT_A])
                {
                    last_key = ODROID_INPUT_A;
                    if (options[sel].update_cb != NULL)
                    {
                        select = options[sel].update_cb(&options[sel], ODROID_DIALOG_ENTER, 0);
                        sel_old = -1;
                    }
                    else
                        select = true;
                }

                if (select)
                    break;
            }
        }
        if (repeat > 0)
            repeat++;
        if (last_key >= 0)
        {
            if (!joystick.values[last_key])
            {
                last_key = -1;
                repeat = 0;
            }
        }
        if (sel_old != sel)
        {
            int dir = sel - sel_old;
            while (options[sel].enabled == -1 && sel_old != sel)
                sel = (sel + dir) % options_count;

            sel_old = sel;
        }

        odroid_overlay_draw_dialog(header, options, sel);
        lcd_swap();
        HAL_Delay(20);
    }

    odroid_input_wait_for_key(last_key, false);

    odroid_display_force_refresh();

    dialog_open_depth--;

    return sel < 0 ? sel : options[sel].id;
}

int odroid_overlay_confirm(const char *text, bool yes_selected)
{
    odroid_dialog_choice_t choices[] = {
        {0, text, "", -1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {1, curr_lang->s_Yes, "", 1, NULL},
        {0, curr_lang->s_No, "", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST,
    };
    return odroid_overlay_dialog(curr_lang->s_PlsChose, choices, yes_selected ? 2 : 3);
}

void odroid_overlay_alert(const char *text)
{
    odroid_dialog_choice_t choices[] = {
        {0, text, "", -1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {1, curr_lang->s_OK, "", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST,
    };
    odroid_overlay_dialog(curr_lang->s_Confirm, choices, 2);
}

bool odroid_overlay_dialog_is_open(void)
{
    return dialog_open_depth > 0;
}

static bool volume_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t level = odroid_audio_volume_get();
    int8_t min = ODROID_AUDIO_VOLUME_MIN;
    int8_t max = ODROID_AUDIO_VOLUME_MAX;
    char volume_value[ODROID_AUDIO_VOLUME_MAX - ODROID_AUDIO_VOLUME_MIN + 2]; // One extra byte for the null byte

    if (event == ODROID_DIALOG_PREV && level > min)
        odroid_audio_volume_set(--level);

    if (event == ODROID_DIALOG_NEXT && level < max)
        odroid_audio_volume_set(++level);

    // The swapping of colors is done for the selected option only to ensure a visual pleasing bar graph while being inverted
    char a = event == ODROID_DIALOG_INIT && option->id == repeat ? curr_lang->s_Fill[0] : curr_lang->s_Full[0];
    char b = event == ODROID_DIALOG_INIT && option->id == repeat ? curr_lang->s_Full[0] : curr_lang->s_Fill[0];

    for (int i = ODROID_AUDIO_VOLUME_MIN; i <= ODROID_AUDIO_VOLUME_MAX; i++)
        // Please note that #0 volume shows NO bars and that full volume sets the last two bars.
        // This is a bit of a visual hack to ensure both brightness and volume bars are visually aligned while still
        // ensuring that no volume shows no bars when volume is off. A better way would be to add one more step (than brightness) to the volume
        volume_value[i - ODROID_AUDIO_VOLUME_MIN] = i < level || (i == level && level == ODROID_AUDIO_VOLUME_MAX) ? a : b;

    volume_value[ODROID_AUDIO_VOLUME_MAX + 1] = 0;
    sprintf(option->value, "%s", (char *)volume_value);
    return event == ODROID_DIALOG_ENTER;
}

static bool brightness_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t level = odroid_display_get_backlight();
    int8_t max = ODROID_BACKLIGHT_LEVEL_COUNT - 1;
    char bright_value[max + 2];

    if (event == ODROID_DIALOG_PREV && level > 0)
        odroid_display_set_backlight(--level);

    if (event == ODROID_DIALOG_NEXT && level < max)
        odroid_display_set_backlight(++level);

    char a = event == ODROID_DIALOG_INIT && option->id == repeat ? curr_lang->s_Fill[0] : curr_lang->s_Full[0];
    char b = event == ODROID_DIALOG_INIT && option->id == repeat ? curr_lang->s_Full[0] : curr_lang->s_Fill[0];

    for (int i = ODROID_BACKLIGHT_LEVEL0; i <= ODROID_BACKLIGHT_LEVEL9; i++)
        bright_value[i - ODROID_BACKLIGHT_LEVEL0] = (i - ODROID_BACKLIGHT_LEVEL0) <= level ? a : b;

    bright_value[max + 1] = 0;
    sprintf(option->value, "%s", (char *)bright_value);
    return event == ODROID_DIALOG_ENTER;
}

static bool filter_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t max = ODROID_DISPLAY_FILTER_COUNT - 1;
    int8_t mode = odroid_display_get_filter_mode();
    int8_t prev = mode;

    if (event == ODROID_DIALOG_PREV && --mode < 0)
        mode = max; // 0;
    if (event == ODROID_DIALOG_NEXT && ++mode > max)
        mode = 0; // max;

    if (mode != prev)
    {
        odroid_display_set_filter_mode(mode);
    }

    if (mode == ODROID_DISPLAY_FILTER_OFF)
        strcpy(option->value, curr_lang->s_FilteringOff);
    if (mode == ODROID_DISPLAY_FILTER_SHARP)
        strcpy(option->value, curr_lang->s_FilteringSharp);
    if (mode == ODROID_DISPLAY_FILTER_SOFT)
        strcpy(option->value, curr_lang->s_FilteringSoft);

    return event == ODROID_DIALOG_ENTER;
}

static bool scaling_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    int8_t max = ODROID_DISPLAY_SCALING_COUNT - 1;
    int8_t mode = odroid_display_get_scaling_mode();
    int8_t prev = mode;

    if (event == ODROID_DIALOG_PREV && --mode < 0)
        mode = max; // 0;
    if (event == ODROID_DIALOG_NEXT && ++mode > max)
        mode = 0; // max;

    if (mode != prev)
    {
        odroid_display_set_scaling_mode(mode);
    }

    if (mode == ODROID_DISPLAY_SCALING_OFF)
        strcpy(option->value, curr_lang->s_SCalingOff);
    if (mode == ODROID_DISPLAY_SCALING_FIT)
        strcpy(option->value, curr_lang->s_SCalingFit);
    if (mode == ODROID_DISPLAY_SCALING_FULL)
        strcpy(option->value, curr_lang->s_SCalingFull);
    if (mode == ODROID_DISPLAY_SCALING_CUSTOM)
        strcpy(option->value, curr_lang->s_SCalingCustom);

    return event == ODROID_DIALOG_ENTER;
}

bool speedup_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    rg_app_desc_t *app = odroid_system_get_app();
    if (event == ODROID_DIALOG_PREV && --app->speedupEnabled <= SPEEDUP_MIN)
        app->speedupEnabled = SPEEDUP_MAX - 1;
    if (event == ODROID_DIALOG_NEXT && ++app->speedupEnabled >= SPEEDUP_MAX)
        app->speedupEnabled = SPEEDUP_MIN + 1;

    switch (app->speedupEnabled)
    {
    case SPEEDUP_0_5x:
        sprintf(option->value, "0.5%s", curr_lang->s_Speed_Unit);
        break;
    case SPEEDUP_0_75x:
        sprintf(option->value, "0.75%s", curr_lang->s_Speed_Unit);
        break;
    case SPEEDUP_1x:
        sprintf(option->value, "1%s", curr_lang->s_Speed_Unit);
        break;
    case SPEEDUP_1_25x:
        sprintf(option->value, "1.25%s", curr_lang->s_Speed_Unit);
        break;
    case SPEEDUP_1_5x:
        sprintf(option->value, "1.5%s", curr_lang->s_Speed_Unit);
        break;
    case SPEEDUP_2x:
        sprintf(option->value, "2%s", curr_lang->s_Speed_Unit);
        break;
    case SPEEDUP_3x:
        sprintf(option->value, "3%s", curr_lang->s_Speed_Unit);
        break;
    }

    return event == ODROID_DIALOG_ENTER;
}



static bool turbo_buttons_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    const char *GW_Turbo_Buttons[] = {curr_lang->s_Turbo_None, curr_lang->s_Turbo_A, curr_lang->s_Turbo_B, curr_lang->s_Turbo_AB};
    int8_t turbo_buttons = odroid_settings_turbo_buttons_get();

    if (event == ODROID_DIALOG_PREV)
    {
        if (turbo_buttons > 0)
            odroid_settings_turbo_buttons_set(--turbo_buttons);
        else
        {
            turbo_buttons = 4 - 1;
            odroid_settings_turbo_buttons_set(4 - 1);
        }
    }
    else if (event == ODROID_DIALOG_NEXT)
    {
        if (turbo_buttons < 4 - 1)
            odroid_settings_turbo_buttons_set(++turbo_buttons);
        else
        {
            turbo_buttons = 0;
            odroid_settings_turbo_buttons_set(0);
        }
    }
    sprintf(option->value, "%s", (char *)GW_Turbo_Buttons[turbo_buttons]);
    return event == ODROID_DIALOG_ENTER;
}

int odroid_overlay_settings_menu(odroid_dialog_choice_t *extra_options)
{
    static char bright_value[25];
    static char volume_value[25];
    static char turbo_value[25];

    odroid_dialog_choice_t options[16] = {                         //
        {0, curr_lang->s_Brightness, bright_value, 1, &brightness_update_cb}, //
        {1, curr_lang->s_Volume, volume_value, 1, &volume_update_cb},
        {2, curr_lang->s_Turbo_Button, turbo_value, 1, &turbo_buttons_update_cb},

        ODROID_DIALOG_CHOICE_LAST, //
    };

    if (extra_options)
    {
        int options_count = get_dialog_items_count(options);
        int extra_options_count = get_dialog_items_count(extra_options);
        memcpy(&options[options_count], extra_options, (extra_options_count + 1) * sizeof(odroid_dialog_choice_t));
    }

    int ret = odroid_overlay_dialog(curr_lang->s_OptionsTit, options, 0);

    odroid_settings_commit();

    return ret;
}

static void draw_game_status_bar(runtime_stats_t stats)
{
    int width = ODROID_SCREEN_WIDTH - 48, height = 16;
    int pad_text = (height - i18n_get_text_height()) / 2;
    char bottom[80], header[60];

    snprintf(header, 60, "%s: %d.%d (%d.%d) / %s: %d.%d%%",
             curr_lang->s_FPS,
             (int)stats.totalFPS, (int)fmod(stats.totalFPS * 10, 10),
             (int)stats.skippedFPS, (int)fmod(stats.skippedFPS * 10, 10),
             curr_lang->s_BUSY,
             (int)stats.busyPercent, (int)fmod(stats.busyPercent * 10, 10));
    snprintf(bottom, 80, "%s", ACTIVE_FILE ? (ACTIVE_FILE->name) : "N/A");

    odroid_overlay_draw_fill_rect(0, 0, ODROID_SCREEN_WIDTH, height, curr_colors->main_c);
    odroid_overlay_draw_fill_rect(0, ODROID_SCREEN_HEIGHT - height, ODROID_SCREEN_WIDTH, height, curr_colors->main_c);
    i18n_draw_text_line(48, pad_text, width, header, curr_colors->sel_c, curr_colors->main_c, 0, curr_lang);
    i18n_draw_text_line(0, ODROID_SCREEN_HEIGHT - height + pad_text, ODROID_SCREEN_WIDTH, bottom, curr_colors->sel_c, curr_colors->main_c, 0, curr_lang);
    odroid_overlay_clock(2, 3);
    odroid_overlay_draw_battery(ODROID_SCREEN_WIDTH - 22, ODROID_SCREEN_HEIGHT - 13);
}

int odroid_overlay_game_settings_menu(odroid_dialog_choice_t *extra_options)
{
    char speedup_value[15];
    char scaling_value[15];
    char filtering_value[15];
    strcpy(filtering_value, curr_lang->s_FilteringOff);
    strcpy(scaling_value, curr_lang->s_SCalingFull);

    odroid_dialog_choice_t options[16] = {
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {200, curr_lang->s_Scaling, scaling_value, 1, &scaling_update_cb},
        {210, curr_lang->s_Filtering, filtering_value, 1, &filter_update_cb}, // Interpolation
        {220, curr_lang->s_Speed, speedup_value, 1, &speedup_update_cb},

        ODROID_DIALOG_CHOICE_LAST,
    };

    if (extra_options)
    {
        int options_count = get_dialog_items_count(options);
        int extra_options_count = get_dialog_items_count(extra_options);
        memcpy(&options[options_count], extra_options, (extra_options_count + 1) * sizeof(odroid_dialog_choice_t));
    }

    odroid_audio_mute(true);
    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY))
        wdog_refresh();

    int r = odroid_overlay_settings_menu(options);

    odroid_audio_mute(false);

    return r;
}

int odroid_overlay_game_debug_menu(void)
{
    odroid_dialog_choice_t options[12] = {
        {10, "Screen Res", "A", 1, NULL},
        {10, "Game Res", "B", 1, NULL},
        {10, "Scaled Res", "C", 1, NULL},
        {10, "Cheats", "C", 1, NULL},
        {10, "Rewind", "C", 1, NULL},
        {10, "Registers", "C", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST,
    };

    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY))
        wdog_refresh();
    return odroid_overlay_dialog("Debugging", options, 0);
}

#if CHEAT_CODES == 1
static bool cheat_update_cb(odroid_dialog_choice_t *option, odroid_dialog_event_t event, uint32_t repeat)
{
    bool is_on = odroid_settings_ActiveGameGenieCodes_is_enabled(CHOSEN_FILE->id, option->id);
    if (event == ODROID_DIALOG_PREV || event == ODROID_DIALOG_NEXT) 
    {
        is_on = is_on ? false : true;
        odroid_settings_ActiveGameGenieCodes_set(CHOSEN_FILE->id, option->id, is_on);
    }
    strcpy(option->value, is_on ? curr_lang->s_Cheat_Codes_ON : curr_lang->s_Cheat_Codes_OFF);
#ifdef ENABLE_EMULATOR_MSX
    if (event == ODROID_DIALOG_ENTER) {
        retro_emulator_t *emu = file_to_emu(CHOSEN_FILE);
        if(strcmp(emu->system_name, "MSX") == 0) {
            update_cheats_msx();
        }
    }
#endif
    return event == ODROID_DIALOG_ENTER;
}

static bool show_cheat_dialog()
{
    static odroid_dialog_choice_t last = ODROID_DIALOG_CHOICE_LAST;
    
    printf("count = %d\n",CHOSEN_FILE->cheat_count);
    // +1 for the terminator sentinel
    odroid_dialog_choice_t *choices = rg_alloc((CHOSEN_FILE->cheat_count + 1) * sizeof(odroid_dialog_choice_t), MEM_ANY);
    char svalues[MAX_CHEAT_CODES][10];

    for(int i=0; i<CHOSEN_FILE->cheat_count; i++) 
    {
        const char *label = CHOSEN_FILE->cheat_descs[i];
        if (label == NULL) {
            label = CHOSEN_FILE->cheat_codes[i];
        }
        choices[i].id = i;
        choices[i].label = label;
        choices[i].value = svalues[i];
        choices[i].enabled = 1;
        choices[i].update_cb = cheat_update_cb;
    }
    choices[CHOSEN_FILE->cheat_count] = last;
    odroid_overlay_dialog(curr_lang->s_Cheat_Codes_Title, choices, 0);

    rg_free(choices);
    odroid_settings_commit();
    return false;
}
#endif
int odroid_overlay_game_menu(odroid_dialog_choice_t *extra_options)
{
#if CHEAT_CODES == 1
    odroid_dialog_choice_t choices[12];
    bool cheat_update_support = false;
    CHOSEN_FILE = ACTIVE_FILE;
    retro_emulator_t *emu = file_to_emu(CHOSEN_FILE);
    if(strcmp(emu->system_name, "MSX") == 0) {
        cheat_update_support = true;
    }

    int index=0;
    choices[index].id = 10;
    choices[index].label = curr_lang->s_Save_Cont;
    choices[index].value = "";
    choices[index].enabled = (ACTIVE_FILE->save_address != 0);
    choices[index].update_cb = NULL;
    index++;
    choices[index].id = 20;
    choices[index].label = curr_lang->s_Save_Quit;
    choices[index].value = "";
    choices[index].enabled = (ACTIVE_FILE->save_address != 0);
    choices[index].update_cb = NULL;
    index++;
    choices[index].id = 0x0F0F0F0E;
    choices[index].label = "-";
    choices[index].value = "-";
    choices[index].enabled = -1;
    choices[index].update_cb = NULL;
    index++;
    choices[index].id = 30;
    choices[index].label = curr_lang->s_Reload;
    choices[index].value = "";
    choices[index].enabled = 1;
    choices[index].update_cb = NULL;
    index++;
    choices[index].id = 40;
    choices[index].label = curr_lang->s_Options;
    choices[index].value = "";
    choices[index].enabled = 1;
    choices[index].update_cb = NULL;
    index++;
    if ((ACTIVE_FILE->cheat_count != 0) && (cheat_update_support)) {
        choices[index].id = 60;
        choices[index].label = curr_lang->s_Cheat_Codes;
        choices[index].value = "";
        choices[index].enabled = 1;
        choices[index].update_cb = NULL;
        index++;
    }
    choices[index].id = 0x0F0F0F0E;
    choices[index].label = "-";
    choices[index].value = "-";
    choices[index].enabled = -1;
    choices[index].update_cb = NULL;
    index++;
    choices[index].id = 90;
    choices[index].label = curr_lang->s_Power_off;
    choices[index].value = "";
    choices[index].enabled = 1;
    choices[index].update_cb = NULL;
    index++;
    choices[index].id = 0x0F0F0F0E;
    choices[index].label = "-";
    choices[index].value = "-";
    choices[index].enabled = -1;
    choices[index].update_cb = NULL;
    index++;
    choices[index].id = 100;
    choices[index].label = curr_lang->s_Quit_to_menu;
    choices[index].value = "";
    choices[index].enabled = 1;
    choices[index].update_cb = NULL;
    index++;
    choices[index].id = 0x0F0F0F0F;
    choices[index].label = "LAST";
    choices[index].value = "LAST";
    choices[index].enabled = 0xFFFF;
    choices[index].update_cb = NULL;
#else
    odroid_dialog_choice_t choices[] = {
        // {0, "Continue", "",  1, NULL},
        {10, curr_lang->s_Save_Cont, "", (ACTIVE_FILE->save_address != 0), NULL},
        {20, curr_lang->s_Save_Quit, "", (ACTIVE_FILE->save_address != 0), NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {30, curr_lang->s_Reload, "", 1, NULL},
        {40, curr_lang->s_Options, "", 1, NULL},
        // {50, "Tools", "", 1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {90, curr_lang->s_Power_off, "", 1, NULL},
        ODROID_DIALOG_CHOICE_SEPARATOR,
        {100, curr_lang->s_Quit_to_menu, "", 1, NULL},
        ODROID_DIALOG_CHOICE_LAST,
    };
#endif
    //Del Some item
    if (ACTIVE_FILE->save_address == 0)
    {
        for (int i = 0; i < 7; i++)
        {
            choices[i] = choices[i + 3];
        }
    }

    // Collect stats before freezing emulation with wait_all_keys_released()
    runtime_stats_t stats = odroid_system_get_stats();

    odroid_audio_mute(true);
    while (odroid_input_key_is_pressed(ODROID_INPUT_ANY))
        wdog_refresh();
    draw_game_status_bar(stats);

    lcd_sync();

    int r = odroid_overlay_dialog(curr_lang->s_Retro_Go_options, choices, 0);

    // Clear startup file so we boot into the retro-go gui
    odroid_settings_StartupFile_set(NULL);

    switch (r)
    {
    case 10:
        odroid_system_emu_save_state(0);
        break;
    case 20:
        odroid_system_emu_save_state(0);
        odroid_system_switch_app(0);
        break;
    case 30:
        odroid_system_emu_load_state(0);
        break; // TODO: Reload emulator?
    case 40:
        odroid_overlay_game_settings_menu(extra_options);
        break;
    case 50:
        odroid_overlay_game_debug_menu();
        break;
#if CHEAT_CODES == 1
    case 60:
        show_cheat_dialog();
        break;
#endif
    case 90:
#if OFF_SAVESTATE == 1
        // Slot 1 is a common slot used only for power off/power on
        odroid_system_emu_save_state(1);
#else
        odroid_system_emu_save_state(0);
#endif
        odroid_system_sleep();
        break;
    case 100:
        odroid_system_switch_app(0);
        break;
    }

    // Required to reset the timestamps (we don't run a background stats task)
    (void)odroid_system_get_stats();

    odroid_audio_mute(false);

    return r;
}

#endif
