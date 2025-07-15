/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --no-compress --font simhei.ttf --symbols 刘宇翔1234567890 --format lvgl -o font.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif

#ifndef FONT
#define FONT 1
#endif

#if FONT

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0030 "0" */
    0x38, 0xc9, 0xa, 0x1c, 0x38, 0x50, 0xa1, 0x42,
    0xcc, 0xf0, 0xc0,

    /* U+0031 "1" */
    0x2f, 0xd2, 0x49, 0x24, 0x90,

    /* U+0032 "2" */
    0x7b, 0xb8, 0x41, 0xc, 0x21, 0x84, 0x31, 0x8c,
    0x3f,

    /* U+0033 "3" */
    0x0, 0x79, 0x1a, 0x30, 0x61, 0x87, 0x3, 0x3,
    0x8d, 0x11, 0xc0,

    /* U+0034 "4" */
    0x4, 0xc, 0xc, 0x14, 0x34, 0x24, 0x44, 0xc4,
    0xff, 0x4, 0x4, 0x4,

    /* U+0035 "5" */
    0x3e, 0x40, 0x83, 0x7, 0xcc, 0x80, 0x81, 0x2,
    0x85, 0xb1, 0xc0,

    /* U+0036 "6" */
    0x8, 0x30, 0xc1, 0x7, 0x8f, 0xb1, 0xc1, 0x83,
    0x5, 0x11, 0xc0,

    /* U+0037 "7" */
    0xfe, 0xc, 0x10, 0x60, 0x83, 0x4, 0x8, 0x30,
    0x40, 0x81, 0x0,

    /* U+0038 "8" */
    0x79, 0x9a, 0x14, 0x2c, 0xcf, 0x33, 0x43, 0x87,
    0xf, 0xf3, 0xe0,

    /* U+0039 "9" */
    0x38, 0x8b, 0xe, 0x1c, 0x78, 0xdf, 0x6, 0x8,
    0x30, 0x41, 0x80,

    /* U+5218 "刘" */
    0x10, 0x8, 0xc0, 0x42, 0x3, 0xfe, 0x90, 0x64,
    0x82, 0x24, 0x91, 0x26, 0x89, 0x1c, 0x48, 0xe2,
    0x45, 0x92, 0x44, 0x16, 0x20, 0xe0, 0x1c, 0x0,
    0x0,

    /* U+5B87 "宇" */
    0x6, 0x0, 0x10, 0x3f, 0xff, 0x0, 0x18, 0x0,
    0x9f, 0xf0, 0x8, 0x0, 0x40, 0xff, 0xf8, 0x10,
    0x0, 0x80, 0x4, 0x0, 0x20, 0x7, 0x0,

    /* U+7FD4 "翔" */
    0x58, 0x1, 0x4e, 0x71, 0x8, 0x7e, 0x21, 0x22,
    0xb4, 0x8a, 0x5f, 0xa9, 0x48, 0x21, 0x21, 0x9f,
    0xee, 0xd2, 0x48, 0x50, 0x21, 0x40, 0x87, 0x6,
    0x30, 0x0, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 11, .adv_w = 128, .box_w = 3, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 16, .adv_w = 128, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 25, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 36, .adv_w = 128, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 48, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 59, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 70, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 81, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 128, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 256, .box_w = 13, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 128, .adv_w = 256, .box_w = 13, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 151, .adv_w = 256, .box_w = 14, .box_h = 15, .ofs_x = 1, .ofs_y = -2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x96f, 0x2dbc
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 48, .range_length = 10, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 21016, .range_length = 11709, .glyph_id_start = 11,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 3, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 2,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t font = {
#else
lv_font_t font = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 15,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if FONT*/

