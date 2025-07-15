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
#include "aht10.h"
#include "drv_lcd.h"

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

int main(void)
{
    lcd_clear(WHITE);
    lcd_set_color(WHITE,BLACK);
    float humidity, temperature;
    aht10_device_t dev;

    /* 总线名称 */
    const char *i2c_bus_name = "i2c3";
    int count = 0;

    /* 等待传感器正常工作 */
    rt_thread_mdelay(2000);

    /* 初始化 aht10 */
    dev = aht10_init(i2c_bus_name);
    if (dev == RT_NULL)
    {
        LOG_E(" The sensor initializes failure");
        return 0;
    }

    while (count++ < 100)
    {
        /* 读取湿度 */
        humidity = aht10_read_humidity(dev);
        LOG_D("humidity: %d.%d %%", (int)humidity, (int)(humidity * 10) % 10);

        // 格式化湿度信息为字符串
        char humidity_str[16];
        sprintf(humidity_str, "Humidity: %d.%d%%", (int)humidity, (int)(humidity * 10) % 10);
        lcd_show_string(10, 100, 24, humidity_str);

        /* 读取温度 */
        temperature = aht10_read_temperature(dev);
        LOG_D("temperature: %d.%d", (int)temperature, (int)(temperature * 10) % 10);

        char temp_str[16];
        sprintf(temp_str, "Temperature: %d.%dC", (int)temperature, (int)(temperature * 10) % 10);
        lcd_show_string(10, 50, 24, temp_str);

        rt_thread_mdelay(300);
    }
    return 0;
}
