/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-5-10      ShiHao       first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define DBG_TAG "main"
#define DBG_LVL         DBG_LOG
#include <rtdbg.h>
#include <lvgl.h>
/* 配置 LED 灯引脚 */
// #define PIN_LED_B              GET_PIN(F, 11)      // PE7 :  LED_R        --> LED
// #define PIN_LED_R              GET_PIN(F, 12)      // PE7 :  LED_R        --> LED

LV_FONT_DECLARE(font)
LV_IMG_DECLARE(imglyx)

void rolling_thread_entry(void *ptr);
void changing_thread_entry(void *ptr);


void lv_user_gui_init(void)
{
    lv_obj_t *obj = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(obj, &font, 0);
    lv_label_set_text(obj, "刘宇翔");
    lv_obj_set_pos(obj, 0, 10);

    lv_obj_t *obj2 = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(obj2, &font, 0);
    lv_label_set_text(obj2, "922106840127");
    lv_obj_set_pos(obj2, 0, 40);



    lv_obj_t *obj3=lv_img_create(lv_scr_act());
    lv_img_set_src(obj3, &imglyx);
    lv_obj_set_pos(obj3,51,60);

    rt_thread_t thread = rt_thread_create("rolling",                         //线程名字
            rolling_thread_entry,              //线程入口函数
            obj,                        //线程入口参数
            1024,                            //线程堆栈大小
            1,                              //线程优先级
            5);                            //时间片参数
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        LOG_E("Failed to create rolling thread!");
    }

    rt_thread_t thread2 = rt_thread_create("changing",                         //线程名字
            changing_thread_entry,              //线程入口函数
            obj2,                        //线程入口参数
            1024,                            //线程堆栈大小
            1,                              //线程优先级
            5);                            //时间片参数
    if (thread2 != RT_NULL)
    {
        rt_thread_startup(thread2);
    }
    else
    {
        LOG_E("Failed to create changing thread!");
    }
}

void rolling_thread_entry(void *ptr)
{
    lv_obj_t* obj = (lv_obj_t*) ptr;
    int32_t start_pos = -60;             // 起始位置（完全左出）
    int32_t end_pos = 240;               // 结束位置（完全右出）
    const int interval = 5000 / 300; // 20ms刷新一次
    while (1)
    {
        // 从左侧移动到右侧
        for (int16_t x = start_pos; x <= end_pos; x += 1)
        {
            lv_obj_set_pos(obj, x, 20);
            HAL_Delay(interval);
        }

        // 重置位置到左侧外（保持无缝衔接）
        lv_obj_set_x(obj, start_pos);
    }
}

// 添加颜色定义
#define COLOR_BLACK   lv_color_make(0x00, 0x00, 0x00)
#define COLOR_RED     lv_color_make(0xFF, 0x00, 0x00)
#define COLOR_GREEN   lv_color_make(0x00, 0xFF, 0x00)
#define COLOR_BLUE    lv_color_make(0x00, 0x00, 0xFF)
#define COLOR_YELLOW  lv_color_make(0xFF, 0xFF, 0x00)
#define COLOR_PURPLE  lv_color_make(0x80, 0x00, 0x80)

void changing_thread_entry(void *ptr)
{
    lv_obj_t* obj = (lv_obj_t*) ptr;
    const lv_color_t colors[] = {
            COLOR_BLACK,
            COLOR_RED,
            COLOR_GREEN,
            COLOR_BLUE,
            COLOR_YELLOW,
            COLOR_PURPLE };
    uint8_t color_index = 0;
    const uint16_t color_count = sizeof(colors) / sizeof(colors[0]);

    while (1)
    {
        // 设置当前颜色
        lv_obj_set_style_text_color(obj, colors[color_index], LV_PART_MAIN);

        // 更新颜色索引
        color_index = (color_index + 1) % color_count;

        // 等待60秒（精确到分钟切换）
        rt_thread_mdelay(60 * 1000); // 使用延时函数
    }
}

int main(void)
{

    return 0;
}
