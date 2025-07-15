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
static struct rt_semaphore rx_sem;
/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断， 调用此回调函数， 然后发送接收信号量 */
    rt_sem_release(&rx_sem);
    return RT_EOK;
}
/* 串口接收数据处理线程 */
static void serial_thread_entry(void *parameter)
{
    char ch;
    while (1)
    {
        /* 从串口读取一个字节的数据， 没有读取到则等待接收信号量*/
        while (rt_device_read(serial, -1, &ch, 1) != 1)
        {
            /* 阻塞等待接收信号量， 等到信号量后再次读取数据*/
            rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        }
        /* 读取到的数据通过串口错位输出*/
        ch = ch + 1;
        rt_device_write(serial, 0, &ch, 1);
    }
}

/* 用户线程入口函数 */
int main(void)
{
    rt_err_t ret = RT_EOK;
    char uart_name[RT_NAME_MAX];
    char str[] = "hello RT-Thread!\r\n";

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT; /* 初始化配置参数 */
    /* 查找串口设备 */
    serial = rt_device_find(SAMPLE_UART_NAME);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", uart_name);
        return RT_ERROR;
    }
    /* 修改串口配置参数*/
    config.baud_rate = BAUD_RATE_115200; //修改波特率为115200
    config.data_bits = DATA_BITS_8;      //数据位8
    config.stop_bits = STOP_BITS_1;      //停止位1
    config.bufsz = 128;                  //修改缓冲区buff size 为128
    config.parity = PARITY_NONE;         //无奇偶校验位
    /* 控制串口设备。通过控制接口传入命令控制字， 与控制参数*/
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);

    /* 初始化信号量*/
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);
    /* 以中断接收及轮询发送模式打开串口设备*/
    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
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
