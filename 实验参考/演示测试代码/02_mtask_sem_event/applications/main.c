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

/* 获取LED_B,LED_R,BUZZER引脚编号 */
#define LED_B       GET_PIN(F,11)
#define LED_R       GET_PIN(F,12)
#define BUZZER      GET_PIN(B,0)

/* 3.1 定义事件 */
#define E_LED_B (1<<0)
#define E_LED_R (1<<1)

/* 2.1 定义信号量句柄 */
rt_sem_t sled_br;

/* 3.2 定义事件集句柄 */
rt_event_t e_led_buzzer;

/* 1.4 线程tled_b入口函数 */
void tled_b_entry(void *parameter)
{
    int i=0; //LED闪烁计数

    /* 设置LED_B引脚为推挽输出模式 */
    rt_pin_mode(LED_B, PIN_MODE_OUTPUT);
    while(1)
    {
        /* 引脚电平翻转 */
        rt_pin_write(LED_B, 1-rt_pin_read(LED_B));
        rt_thread_mdelay(500);
        i++;
        if(i==6 && sled_br!=RT_NULL)
        {
            /* 2.3 释放信号量 */
            rt_sem_release(sled_br);
            rt_kprintf("tled_b release sem!\n");
        }
        if(i==10 && e_led_buzzer!=RT_NULL)
        {
            /* 3.4 发送事件E_LED_B */
            rt_event_send(e_led_buzzer, E_LED_B);
            rt_kprintf("tled_b send event\n");
        }
    }
}

/* 1.5 线程tled_r入口函数 */
void tled_r_entry(void *parameter)
{
    int i=0; //LED闪烁计数

    /* 设置LED_R引脚为推挽输出模式 */
    rt_pin_mode(LED_R, PIN_MODE_OUTPUT);
    while(1)
    {
        if(sled_br!=RT_NULL)
        {
            /* 2.4 获取信号量 */
            rt_kprintf("tled_r Running before sem……\n");
            rt_sem_take(sled_br, RT_WAITING_FOREVER);
            rt_kprintf("tled_r Running aften sem !\n");
            rt_sem_delete(sled_br);//只获取一次，获取到后删除
            sled_br = RT_NULL;
        }
        /* 引脚电平翻转 */
        rt_pin_write(LED_R, 1-rt_pin_read(LED_R));
        rt_thread_mdelay(1000);
        if(++i==10 && e_led_buzzer!=RT_NULL)
        {
            /* 3.4 发送事件E_LED_R */
            rt_event_send(e_led_buzzer, E_LED_R);
            rt_kprintf("tled_r send event\n");
        }
    }
}

/* 1.6 线程tbuzzer入口函数 */
void tbuzzer_entry(void *parameter)
{
    rt_uint32_t e;
    rt_uint8_t  count = 5;

    /* 设置BUZZER引脚为推挽输出模式 */
    rt_pin_mode(BUZZER, PIN_MODE_OUTPUT);
    if(e_led_buzzer!=RT_NULL)
    {
        /* 3.5 接收事件 */
        rt_event_recv(e_led_buzzer, E_LED_B | E_LED_R, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, RT_WAITING_FOREVER, &e);
        rt_kprintf("tbuzzer received event rec e:%d\n",e);
        rt_event_delete(e_led_buzzer);
        e_led_buzzer = RT_NULL;
    }

    /* 蜂鸣器响count次 */
    while(count>0)
    {
        /* 打开蜂鸣器 */
        rt_pin_write(BUZZER, PIN_HIGH);
        rt_thread_mdelay(500);
        /* 关闭蜂鸣器 */
        rt_pin_write(BUZZER, PIN_LOW);
        rt_thread_mdelay(500);
        count--;
    }
}

/* 编译，下载观察现象 */
int main(void)
{
    /* 利用信号量实现tled_b和tled_r同步 */
    /* 2.2 创建信号量 */
    sled_br = rt_sem_create("sled_br", 0, RT_IPC_FLAG_PRIO);

    /* 利用事件集实现tled_b、tled_r和tbuzzer同步 */
    /* 3.3 创建事件集 */
    e_led_buzzer = rt_event_create("e_led_buzzer", RT_IPC_FLAG_PRIO);

    /* 1.1 定义3个线程句柄： tled_b, tled_r, tbuzzer */
    rt_thread_t tled_b, tled_r, tbuzzer;

    /* 1.2 创建3个线程：tled_b, tled_r, tbuzzer */
    tled_b = rt_thread_create("tled_b", tled_b_entry, RT_NULL, 1024, 10, 10);
    tled_r = rt_thread_create("tled_r", tled_r_entry, RT_NULL, 1024, 11, 10);
    tbuzzer = rt_thread_create("tbuzzer", tbuzzer_entry, RT_NULL, 1024, 12, 10);

    /* 1.3 启动3个线程：tled_b, tled_r, tbuzzer */
    rt_thread_startup(tled_b);
    rt_thread_startup(tled_r);
    rt_thread_startup(tbuzzer);

    return RT_EOK;
}
