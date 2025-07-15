/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-5-10      ShiHao       first version
 * 2025-04-11    YourName     添加按键中断翻转红LED，并添加中断消抖功能
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#define DBG_TAG "main"
#define DBG_LVL         DBG_LOG
#include <rtdbg.h>

/* 定义LED和按键对应的引脚 */
#define PIN_LED_R              GET_PIN(F, 12)
#define PIN_BTN_DOWN           GET_PIN(C, 1)

/* 中断回调函数，实现LED状态翻转，并增加消抖处理 */
static void btn_irq_callback(void *args)
{
    /* 静态变量记录上次中断触发时的系统节拍 */
    static rt_tick_t last_tick = 0;
    rt_tick_t cur_tick = rt_tick_get();

    /* 消抖：两次中断触发的间隔必须大于或等于50ms */
    if ((cur_tick - last_tick) < rt_tick_from_millisecond(200))
    {
        return;
    }
    last_tick = cur_tick;

    int state;

    /* 读取当前LED状态 */
    state = rt_pin_read(PIN_LED_R);
    LOG_D("按键触发中断，当前LED状态：%d", state);

    /* 翻转LED状态 */
    if (state == PIN_LOW)
    {
        rt_pin_write(PIN_LED_R, PIN_HIGH);
    }
    else
    {
        rt_pin_write(PIN_LED_R, PIN_LOW);
    }
}

int main(void)
{
    /* 配置LED引脚为输出模式 */
    rt_pin_mode(PIN_LED_R, PIN_MODE_OUTPUT);
    /* 配置按键引脚为输入模式，并开启上拉（假设按键按下时为低电平） */
    rt_pin_mode(PIN_BTN_DOWN, PIN_MODE_INPUT_PULLUP);

    /* 绑定中断回调函数，使用下降沿触发 */
    rt_pin_attach_irq(PIN_BTN_DOWN, PIN_IRQ_MODE_FALLING, btn_irq_callback, RT_NULL);
    /* 使能按键中断 */
    rt_pin_irq_enable(PIN_BTN_DOWN, PIN_IRQ_ENABLE);

    LOG_D("系统初始化完成，等待按键触发中断…");

    /* 主线程循环，可以在此添加其他任务 */
    while (1)
    {
        rt_thread_mdelay(50);
    }

    return 0;
}
