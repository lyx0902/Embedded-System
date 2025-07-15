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

#define SAMPLE_UART_NAME "uart1" /* 串口设备名称*/
static rt_device_t serial;       /* 串口设备句柄*/

/* 用于接收消息的信号量 */
//static struct rt_semaphore rx_sem;
rt_sem_t rx_sem;
/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断， 调用此回调函数， 然后发送接收信号量 */
    //rt_sem_release(&rx_sem);
    rt_sem_release(rx_sem);
    return RT_EOK;
}

/* 串口接收数据处理线程 */
static void serial_thread_entry(void *parameter)
{
    rt_uint32_t rx_length;
    static char rx_buffer[RT_SERIAL_RB_BUFSZ + 1];
    while (1)
    {
        /* 阻塞等待接收信号量， 等到信号量后读取一包数据*/
        //rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        rt_sem_take(rx_sem, RT_WAITING_FOREVER);
        rx_length = rt_device_read(serial, 0, rx_buffer, RT_SERIAL_RB_BUFSZ);
        rx_buffer[rx_length] = '\0';
        /* 通过串口设备serial 输出读取到的消息*/
        rt_device_write(serial, 0, rx_buffer, rx_length);
    }
}

/* 用户线程入口函数 */
int main(void)
{
    rt_err_t ret = RT_EOK;
    char str[] = "hello RT-Thread!\r\n";

    /* 初始化信号量*/
    //rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
    rx_sem = rt_sem_create("rx_sem", 0, RT_IPC_FLAG_FIFO);
    /* 查找串口设备 */
    serial = rt_device_find(SAMPLE_UART_NAME);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", SAMPLE_UART_NAME);
        return RT_ERROR;
    }
    /* 以DMA接收及轮询发送模式打开串口设备*/
    rt_device_open(serial, RT_DEVICE_FLAG_DMA_RX);
    /* 设置接收回调函数*/
    rt_device_set_rx_indicate(serial, uart_input);
    /* 发送字符串*/
    rt_device_write(serial, 0, str, (sizeof(str) - 1));
    /* 创建serial 线程*/
    rt_thread_t thread = rt_thread_create("serial", serial_thread_entry, RT_NULL,
                                          1024, 25, 10);
    /* 创建成功则启动线程*/
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }
    return ret;
}
