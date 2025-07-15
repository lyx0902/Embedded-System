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

/* 配置 LED 灯引脚 */
#define PIN_LED_B              GET_PIN(F, 11)      // PF11 :  LED_B        --> LED
#define PIN_LED_R              GET_PIN(F, 12)      // PF12 :  LED_R        --> LED
  rt_thread_t tid1,tid2;



  /* 3 编写信号处理函数 */
void tid1_signal_handler(int sig)
{
    rt_kprintf("tid1 reciever signal %d\n",sig);
}

void tid1_entry(void* paramenter)
{
    int cnt =0;
    /* 1 安装信号  */
    rt_signal_install(SIGUSR1,tid1_signal_handler);

    /* 2 解除阻塞 */
    rt_signal_unmask(SIGUSR1);

    while(cnt<10)
    {
        cnt ++;
        rt_kprintf("cnt:%d\n",cnt);
        rt_thread_mdelay(100);
    }
}
void tid2_entry(void* paramenter)
{
    rt_thread_kill(tid1,SIGUSR1);
}


int main(void)
{


    tid1 = rt_thread_create("tid1", tid1_entry, RT_NULL, 512, 10, 10);
    tid2 = rt_thread_create("tid2", tid2_entry, RT_NULL, 512, 10, 10);
    rt_thread_startup(tid1);

    rt_thread_mdelay(300);
    rt_kprintf("start tid2,lauch sig to tid1\n");
    rt_thread_startup(tid2);

    return 0;
}

