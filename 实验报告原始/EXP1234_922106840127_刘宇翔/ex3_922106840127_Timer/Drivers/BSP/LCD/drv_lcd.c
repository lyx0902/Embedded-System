/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-28     unknow       copy by STemwin
 * 2021-12-29     xiangxistu   port for lvgl <lcd_fill_array>
 * 2022-6-26      solar        Improve the api required for resistive touch screen calibration
 * 2023-05-17     yuanjie      parallel driver improved
 */

#include "stdio.h"
#include "stdlib.h"
#include "drv_lcd.h"
#include "drv_lcd_font.h"

/* 管理LCD重要参数 */
_lcd_dev lcddev;
uint16_t BACK_COLOR = WHITE, FORE_COLOR = BLACK;

#define LCD_CLEAR_SEND_NUMBER 5760

#define LCD_BASE ((uint32_t)(0x68000000 | 0x0003FFFE)) // A18 link to DCX
#define LCD ((LCD_CONTROLLER_TypeDef *)LCD_BASE)

#define LCD_DEVICE(dev) (struct drv_lcd_device *)(dev)

// 写寄存器函数
// regval:寄存器值
void LCD_WR_REG(uint8_t regval)
{
    LCD->_u8_REG = regval; // 写入要写的寄存器序号
}
// 写LCD数据
// data:要写入的值
void LCD_WR_DATA16(uint16_t data)
{
    LCD->_u16_RAM = data;
}
void LCD_WR_DATA8(uint8_t data)
{
    LCD->_u8_RAM = data;
}
// 读LCD数据
// 返回值:读到的值
uint8_t LCD_RD_DATA8(void)
{
    return LCD->_u8_RAM;
}
// 写寄存器
// LCD_Reg:寄存器地址
// LCD_RegValue:要写入的数据
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
    LCD->_u8_REG = LCD_Reg;      // 写入要写的寄存器序号
    LCD->_u16_RAM = LCD_RegValue; // 写入数据
}
// 读寄存器
// LCD_Reg:寄存器地址
// 返回值:读到的数据
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
    LCD_WR_REG(LCD_Reg);  // 写入要读的寄存器序号
    return LCD_RD_DATA8(); // 返回读到的值
}
// 开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
    LCD->_u8_REG = lcddev.wramcmd;
}
// LCD写GRAM
// RGB_Code:颜色值
void LCD_WriteRAM(uint16_t RGB_Code)
{
    LCD->_u16_RAM = RGB_Code; // 写十六位GRAM
}

// 从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
// 通过该函数转换
// c:GBR格式的颜色值
// 返回值：RGB格式的颜色值
uint16_t LCD_BGR2RGB(uint16_t c)
{
    uint16_t r, g, b, rgb;
    b = (c >> 0) & 0x1f;
    g = (c >> 5) & 0x3f;
    r = (c >> 11) & 0x1f;
    rgb = (b << 11) + (g << 5) + (r << 0);
    return (rgb);
}

// 设置光标位置(对RGB屏无效)
// Xpos:横坐标
// Ypos:纵坐标
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA16(Xpos >> 8);
    LCD_WR_DATA16(Xpos & 0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA16(Ypos >> 8);
    LCD_WR_DATA16(Ypos & 0XFF);
}

// 读取个某点的颜色值
// x,y:坐标
// 返回值:此点的颜色
void LCD_ReadPoint(char *pixel, int x, int y)
{
    uint16_t *color = (uint16_t *)pixel;
    uint16_t r = 0, g = 0, b = 0;
    if (x >= lcddev.width || y >= lcddev.height)
    {
        *color = 0; // 超过了范围,直接返回
        return;
    }
    LCD_SetCursor(x, y);

    LCD_WR_REG(0X2E); // 9341/3510/1963 发送读GRAM指令

    r = LCD_RD_DATA8();      // dummy Read
    r = LCD_RD_DATA8(); // 实际坐标颜色
    b = LCD_RD_DATA8();

    g = r & 0XFF; // 对于9341/5310/5510,第一次读取的是RG的值,R在前,G在后,各占8位
    g <<= 8;
    *color = (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11)); // ILI9341/NT35310/NT35510需要公式转换一下
}
// LCD开启显示
void LCD_DisplayOn(void)
{
    LCD_WR_REG(0X29); // 开启显示
}

// LCD关闭显示
void LCD_DisplayOff(void)
{
    LCD_WR_REG(0X28); // 关闭显示
}

// 初始化LCD背光定时器
void LCD_PWM_BackLightInit()
{
//    pwm_bl_dev = (struct rt_device_pwm *)rt_device_find(PWM_BL_NAME);
//    if(RT_NULL != pwm_bl_dev)
//    {
//        /* 设置PWM周期和脉冲宽度默认值 */
//        rt_pwm_set(pwm_bl_dev, PWM_BL_CHANNEL, PWM_BL_PERIOD, 0);

//    }
//    else
//    {
//        printf("pwm backlight error!");
//    }
}

// 设置LCD背光亮度
// pwm:背光等级,0~100.越大越亮.
void LCD_BackLightSet(uint8_t value)
{
//    value = value > 100 ? 100 : value;
//    if(RT_NULL != pwm_bl_dev)
//    {
//        /* 设置PWM周期和脉冲宽度默认值 */
//        rt_pwm_set(pwm_bl_dev, PWM_BL_CHANNEL, PWM_BL_PERIOD, (PWM_BL_PERIOD/100)*value);
//        /* 使能设备 */
//        rt_pwm_enable(pwm_bl_dev, PWM_BL_CHANNEL);
//        
//        printf("backlight %d percent", value);
//    }
//    else
//    {
//        printf("backlight set error!");
//    }
}
// 设置LCD的自动扫描方向(对RGB屏无效)
// 注意:其他函数可能会受到此函数设置的影响(尤其是9341),
// 所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
// dir:0~7,代表8个方向(具体定义见lcd.h)
// TODO 刷新方向
void LCD_Scan_Dir(uint8_t dir)
{

}

// 快速画点
// x,y:坐标
// color:颜色
static void LCD_Fast_DrawPoint(const char *pixel, int x, int y)
{
    uint16_t color = *((uint16_t *)pixel);

    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA16(x >> 8);
    LCD_WR_DATA16(x & 0XFF);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA16(y >> 8);
    LCD_WR_DATA16(y & 0XFF);
    LCD->_u8_REG = lcddev.wramcmd;
    LCD->_u16_RAM = color;
}

// 设置LCD显示方向
// dir:0,竖屏；1,横屏
void LCD_Display_Dir(uint8_t dir)
{
    lcddev.dir = dir; // 竖屏/横屏
    if (dir == 0)     // 竖屏
    {
        lcddev.width = 240;
        lcddev.height = 240;

        lcddev.wramcmd = 0X2C;
        lcddev.setxcmd = 0X2A;
        lcddev.setycmd = 0X2B;
    }
    else // 横屏
    {
        lcddev.width = 240;
        lcddev.height = 240;

        lcddev.wramcmd = 0X2C;
        lcddev.setxcmd = 0X2A;
        lcddev.setycmd = 0X2B;
    }
    // TODO scan dir settings
    // LCD_Scan_Dir(DFT_SCAN_DIR); //默认扫描方向
}

static void lcd_write_half_word(const uint16_t da)
{
    uint16_t data = 0;

    data = da >> 8;
    data += (da & 0xff) << 8;
    LCD_WR_DATA16(data);
}

static void lcd_write_data_buffer(const void *send_buf, size_t length)
{
    uint8_t *pdata = NULL;
    size_t len = 0;

    pdata = (uint8_t*)send_buf;
    len = length;

    if (pdata != NULL)
    {
        while (len -- )
        {
            LCD_WR_DATA8(*pdata);
            pdata ++;
        }
    }
}

/**
 * Set background color and foreground color
 *
 * @param   back    background color
 * @param   fore    fore color
 *
 * @return  void
 */
void lcd_set_color(uint16_t back, uint16_t fore)
{
    BACK_COLOR = back;
    FORE_COLOR = fore;
}

/**
 * Set drawing area
 *
 * @param   x1      start of x position
 * @param   y1      start of y position
 * @param   x2      end of x position
 * @param   y2      end of y position
 *
 * @return  void
 */
void lcd_address_set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{

    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA8(x1 >> 8);
    LCD_WR_DATA8(x1 & 0xff);
    LCD_WR_DATA8(x2 >> 8);
    LCD_WR_DATA8(x2 & 0xff);
    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA8(y1 >> 8);
    LCD_WR_DATA8(y1 & 0xff);
    LCD_WR_DATA8(y2 >> 8);
    LCD_WR_DATA8(y2 & 0xff);

    LCD_WriteRAM_Prepare();      // 开始写入GRAM
}

/**
 * clear the lcd.
 *
 * @param   color       Fill color
 *
 * @return  void
 */
void lcd_clear(uint16_t color)
{
    uint32_t index = 0;
    uint32_t totalpoint = lcddev.width;
    totalpoint *= lcddev.height; // 得到总点数
    lcd_address_set(0, 0, lcddev.width - 1, lcddev.height - 1); // 设置光标位置
    for (index = 0; index < totalpoint; index++)
    {
        lcd_write_half_word(color);
    }
}

/**
 * display a point on the lcd.
 *
 * @param   x   x position
 * @param   y   y position
 *
 * @return  void
 */
void lcd_draw_point(uint16_t x, uint16_t y)
{
    lcd_address_set(x, y, x, y);
    lcd_write_half_word(FORE_COLOR);
}


/**
 * full color on the lcd.
 *
 * @param   x_start     start of x position
 * @param   y_start     start of y position
 * @param   x_end       end of x position
 * @param   y_end       end of y position
 * @param   color       Fill color
 *
 * @return  void
 */
void lcd_fill(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, uint16_t color)
{
    uint16_t i = 0, j = 0;
    uint32_t size = 0, size_remain = 0;
    uint8_t *fill_buf = NULL;

    size = (x_end - x_start) * (y_end - y_start) * 2;

    if (size > LCD_CLEAR_SEND_NUMBER)
    {
        /* the number of remaining to be filled */
        size_remain = size - LCD_CLEAR_SEND_NUMBER;
        size = LCD_CLEAR_SEND_NUMBER;
    }

    lcd_address_set(x_start, y_start, x_end, y_end);

    //fill_buf = (uint8_t *)rt_malloc(size);
	fill_buf = (uint8_t *)malloc(size);
    if (fill_buf)
    {
        /* fast fill */
        while (1)
        {
            for (i = 0; i < size / 2; i++)
            {
                fill_buf[2 * i] = color >> 8;
                fill_buf[2 * i + 1] = color;
            }
            lcd_write_data_buffer(fill_buf, size);

            /* Fill completed */
            if (size_remain == 0)
                break;

            /* calculate the number of fill next time */
            if (size_remain > LCD_CLEAR_SEND_NUMBER)
            {
                size_remain = size_remain - LCD_CLEAR_SEND_NUMBER;
            }
            else
            {
                size = size_remain;
                size_remain = 0;
            }
        }
        //rt_free(fill_buf);
	    free(fill_buf);
    }
    else
    {
        for (i = y_start; i <= y_end; i++)
        {
            for (j = x_start; j <= x_end; j++)lcd_write_half_word(color);
        }
    }
}

/**
 * display a line on the lcd.
 *
 * @param   x1      x1 position
 * @param   y1      y1 position
 * @param   x2      x2 position
 * @param   y2      y2 position
 *
 * @return  void
 */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint16_t t;
    uint32_t i = 0;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;

    if (y1 == y2)
    {
        /* fast draw transverse line */
        lcd_address_set(x1, y1, x2, y2);

        uint8_t line_buf[480] = {0};

        for (i = 0; i < x2 - x1; i++)
        {
            line_buf[2 * i] = FORE_COLOR >> 8;
            line_buf[2 * i + 1] = FORE_COLOR;
        }

        lcd_write_data_buffer(line_buf, (x2 - x1) * 2);

        return ;
    }

    delta_x = x2 - x1;
    delta_y = y2 - y1;
    row = x1;
    col = y1;
    if (delta_x > 0)incx = 1;
    else if (delta_x == 0)incx = 0;
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }
    if (delta_y > 0)incy = 1;
    else if (delta_y == 0)incy = 0;
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }
    if (delta_x > delta_y)distance = delta_x;
    else distance = delta_y;
    for (t = 0; t <= distance + 1; t++)
    {
        lcd_draw_point(row, col);
        xerr += delta_x ;
        yerr += delta_y ;
        if (xerr > distance)
        {
            xerr -= distance;
            row += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            col += incy;
        }
    }
}

/**
 * display a rectangle on the lcd.
 *
 * @param   x1      x1 position
 * @param   y1      y1 position
 * @param   x2      x2 position
 * @param   y2      y2 position
 *
 * @return  void
 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcd_draw_line(x1, y1, x2, y1);
    lcd_draw_line(x1, y1, x1, y2);
    lcd_draw_line(x1, y2, x2, y2);
    lcd_draw_line(x2, y1, x2, y2);
}

/**
 * display a circle on the lcd.
 *
 * @param   x       x position of Center
 * @param   y       y position of Center
 * @param   r       radius
 *
 * @return  void
 */
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r)
{
    int a, b;
    int di;
    a = 0;
    b = r;
    di = 3 - (r << 1);
    while (a <= b)
    {
        lcd_draw_point(x0 - b, y0 - a);
        lcd_draw_point(x0 + b, y0 - a);
        lcd_draw_point(x0 - a, y0 + b);
        lcd_draw_point(x0 - b, y0 - a);
        lcd_draw_point(x0 - a, y0 - b);
        lcd_draw_point(x0 + b, y0 + a);
        lcd_draw_point(x0 + a, y0 - b);
        lcd_draw_point(x0 + a, y0 + b);
        lcd_draw_point(x0 - b, y0 + a);
        a++;
        //Bresenham
        if (di < 0)di += 4 * a + 6;
        else
        {
            di += 10 + 4 * (a - b);
            b--;
        }
        lcd_draw_point(x0 + a, y0 + b);
    }
}

static void lcd_show_char(uint16_t x, uint16_t y, uint8_t data, uint32_t size)
{
    uint8_t temp;
    uint8_t num = 0;;
    uint8_t pos, t;
    uint16_t colortemp = FORE_COLOR;
    uint8_t *font_buf = NULL;

    if (x > LCD_W - size / 2 || y > LCD_H - size)return;

    data = data - ' ';
#ifdef ASC2_1608
    if (size == 16)
    {
        lcd_address_set(x, y, x + size / 2 - 1, y + size - 1);//(x,y,x+8-1,y+16-1)

        //font_buf = (uint8_t *)rt_malloc(size * size);
        font_buf = (uint8_t *)malloc(size * size);
        if (!font_buf)
        {
            /* fast show char */
            for (pos = 0; pos < size * (size / 2) / 8; pos++)
            {
                temp = asc2_1608[(uint16_t)data * size * (size / 2) / 8 + pos];
                for (t = 0; t < 8; t++)
                {
                    if (temp & 0x80)colortemp = FORE_COLOR;
                    else colortemp = BACK_COLOR;
                    lcd_write_half_word(colortemp);
                    temp <<= 1;
                }
            }
        }
        else
        {
            for (pos = 0; pos < size * (size / 2) / 8; pos++)
            {
                temp = asc2_1608[(uint16_t)data * size * (size / 2) / 8 + pos];
                for (t = 0; t < 8; t++)
                {
                    if (temp & 0x80)colortemp = FORE_COLOR;
                    else colortemp = BACK_COLOR;
                    font_buf[2 * (8 * pos + t)] = colortemp >> 8;
                    font_buf[2 * (8 * pos + t) + 1] = colortemp;
                    temp <<= 1;
                }
            }
            lcd_write_data_buffer(font_buf, size * size);
            //rt_free(font_buf);
			free(font_buf);
        }
    }
    else
#endif

#ifdef ASC2_2412
        if (size == 24)
        {
            lcd_address_set(x, y, x + size / 2 - 1, y + size - 1);

            //font_buf = (uint8_t *)rt_malloc(size * size);
			font_buf = (uint8_t *)malloc(size * size);
            if (!font_buf)
            {
                /* fast show char */
                for (pos = 0; pos < (size * 16) / 8; pos++)
                {
                    temp = asc2_2412[(uint16_t)data * (size * 16) / 8 + pos];
                    if (pos % 2 == 0)
                    {
                        num = 8;
                    }
                    else
                    {
                        num = 4;
                    }

                    for (t = 0; t < num; t++)
                    {
                        if (temp & 0x80)colortemp = FORE_COLOR;
                        else colortemp = BACK_COLOR;
                        lcd_write_half_word(colortemp);
                        temp <<= 1;
                    }
                }
            }
            else
            {
                for (pos = 0; pos < (size * 16) / 8; pos++)
                {
                    temp = asc2_2412[(uint16_t)data * (size * 16) / 8 + pos];
                    if (pos % 2 == 0)
                    {
                        num = 8;
                    }
                    else
                    {
                        num = 4;
                    }

                    for (t = 0; t < num; t++)
                    {
                        if (temp & 0x80)colortemp = FORE_COLOR;
                        else colortemp = BACK_COLOR;
                        if (num == 8)
                        {
                            font_buf[2 * (12 * (pos / 2) + t)] = colortemp >> 8;
                            font_buf[2 * (12 * (pos / 2) + t) + 1] = colortemp;
                        }
                        else
                        {
                            font_buf[2 * (8 + 12 * (pos / 2) + t)] = colortemp >> 8;
                            font_buf[2 * (8 + 12 * (pos / 2) + t) + 1] = colortemp;
                        }
                        temp <<= 1;
                    }
                }
                lcd_write_data_buffer(font_buf, size * size);
                //rt_free(font_buf);
				free(font_buf);
            }
        }
        else
#endif

#ifdef ASC2_3216
            if (size == 32)
            {
                lcd_address_set(x, y, x + size / 2 - 1, y + size - 1);

                //font_buf = (uint8_t *)rt_malloc(size * size);
				font_buf = (uint8_t *)malloc(size * size);
                if (!font_buf)
                {
                    /* fast show char */
                    for (pos = 0; pos < size * (size / 2) / 8; pos++)
                    {
                        temp = asc2_3216[(uint16_t)data * size * (size / 2) / 8 + pos];
                        for (t = 0; t < 8; t++)
                        {
                            if (temp & 0x80)colortemp = FORE_COLOR;
                            else colortemp = BACK_COLOR;
                            lcd_write_half_word(colortemp);
                            temp <<= 1;
                        }
                    }
                }
                else
                {
                    for (pos = 0; pos < size * (size / 2) / 8; pos++)
                    {
                        temp = asc2_3216[(uint16_t)data * size * (size / 2) / 8 + pos];
                        for (t = 0; t < 8; t++)
                        {
                            if (temp & 0x80)colortemp = FORE_COLOR;
                            else colortemp = BACK_COLOR;
                            font_buf[2 * (8 * pos + t)] = colortemp >> 8;
                            font_buf[2 * (8 * pos + t) + 1] = colortemp;
                            temp <<= 1;
                        }
                    }
                    lcd_write_data_buffer(font_buf, size * size);
                    //rt_free(font_buf);
					free(font_buf);
                }
            }
            else
#endif
            {
                printf("There is no any define ASC2_1208 && ASC2_2412 && ASC2_2416 && ASC2_3216 !");
            }
}

/**
 * @brief       平方函数, m^n
 * @param       m: 底数
 * @param       n: 指数
 * @retval      m的n次方
 */
static uint32_t lcd_pow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;

    while (n--)
    {
        result *= m;
    }

    return result;
}

/**
 * display the number on the lcd.
 *
 * @param   x       x position
 * @param   y       y position
 * @param   num     number
 * @param   len     length of number
 * @param   size    size of font
 *
 * @return  void
 */
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint32_t size)
{
    uint8_t t, temp;
    uint8_t enshow = 0;

    for (t = 0; t < len; t++)   /* 按总显示位数循环 */
    {
        temp = (num / lcd_pow(10, len - t - 1)) % 10;   /* 获取对应位的数字 */

        if (enshow == 0 && t < (len - 1))               /* 没有使能显示,且还有位要显示 */
        {
            if (temp == 0)
            {
                lcd_show_char(x + (size / 2)*t, y, ' ', size);    /* 显示空格,占位 */
                continue;       /* 继续下个一位 */
            }
            else
            {
                enshow = 1;     /* 使能显示 */
            }
        }

        lcd_show_char(x + (size / 2)*t, y, temp + '0', size);     /* 显示字符 */
    }
}

/**
 * display the string on the lcd.
 *
 * @param   x       x position
 * @param   y       y position
 * @param   size    size of font
 * @param   p       the string to be display
 *
 * @return   0: display success
 *          -1: size of font is not support
 */
void lcd_show_string(uint16_t x, uint16_t y, uint32_t size, const char *p)
{
    if (size != 16 && size != 24 && size != 32)
    {
        printf("font size(%d) is not support!", size);
    }

    while (*p != '\0')
    {
        if (x > LCD_W - size / 2)
        {
            x = 0;
            y += size;
        }
        if (y > LCD_H - size)
        {
            y = x = 0;
            lcd_clear(RED);
        }
        lcd_show_char(x, y, *p, size);
        x += size / 2;
        p++;
    }
}

/**
 * display the image on the lcd.
 *
 * @param   x       x position
 * @param   y       y position
 * @param   length  length of image
 * @param   wide    wide of image
 * @param   p       image
 *
 * @return   0: display success
 *          -1: the image is too large
 */
void lcd_show_image(uint16_t x, uint16_t y, uint16_t length, uint16_t wide, const uint8_t *p)
{
    if (x + length > LCD_W || y + wide > LCD_H)
    {
        printf("The image is too large!");
    }

    lcd_address_set(x, y, x + length - 1, y + wide - 1);
    lcd_write_data_buffer(p, length * wide * 2);
}

void lcd_fill_array(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, void *pcolor)
{
    uint16_t *pixel = NULL;
    uint16_t cycle_y, x_offset = 0;

    pixel = (uint16_t *)pcolor;

    lcd_address_set(x_start, y_start, x_end, y_end);
    for (cycle_y = y_start; cycle_y <= y_end;)
    {
        for (x_offset = 0; x_start + x_offset <= x_end; x_offset++)
        {
            LCD->_u8_RAM = (*pixel)>>8;
            LCD->_u8_RAM = *pixel++;
        }
        cycle_y++;
    }
}

void LCD_DrawLine(const char *pixel, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1; // 计算坐标增量
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if (delta_x > 0)
        incx = 1; // 设置单步方向
    else if (delta_x == 0)
        incx = 0; // 垂直线
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)
        incy = 1;
    else if (delta_y == 0)
        incy = 0; // 水平线
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if (delta_x > delta_y)
        distance = delta_x; // 选取基本增量坐标轴
    else
        distance = delta_y;

    for (t = 0; t <= distance + 1; t++) // 画线输出
    {
        // LCD_DrawPoint(uRow, uCol); //画点
        LCD_Fast_DrawPoint(pixel, uRow, uCol);
        xerr += delta_x;
        yerr += delta_y;

        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}
void LCD_HLine(const char *pixel, int x1, int x2, int y)
{
    LCD_DrawLine(pixel, x1, y, x2, y);
}

void LCD_VLine(const char *pixel, int x, int y1, int y2)
{
    LCD_DrawLine(pixel, x, y1, x, y2);
}

void LCD_BlitLine(const char *pixel, int x, int y, size_t size)
{
    LCD_SetCursor(x, y);
    LCD_WriteRAM_Prepare();
    uint16_t *p = (uint16_t *)pixel;
    for (; size > 0; size--, p++)
        LCD->_u16_RAM = *p;
}


void drv_lcd_init(void)
{
		// 复位LCD
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);
		HAL_Delay(200);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_SET);
		HAL_Delay(100);
	
    // 尝试st7789v3 ID的读取
    LCD_WR_REG(0x04);
    lcddev.id = LCD_RD_DATA8(); // dummy read
    lcddev.id = LCD_RD_DATA8(); // ID2
    lcddev.id = LCD_RD_DATA8(); // ID3
    lcddev.id <<= 8;
    lcddev.id |= LCD_RD_DATA8();

    printf("LCD ID: %x", lcddev.id); // 打印LCD ID
    
    //************* Start Initial Sequence **********//
    /* Memory Data Access Control */
    LCD_WR_REG(0x36);
    LCD_WR_DATA8(0x00);
	
    /* RGB 5-6-5-bit  */
    LCD_WR_REG(0x3A); 
    //LCD_WR_DATA8(0x65);
		LCD_WR_DATA8(0x05);
	
    /* Porch Setting */
    LCD_WR_REG(0xB2);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x33); 
	
    /*  Gate Control */
    LCD_WR_REG(0xB7); 
    LCD_WR_DATA8(0x35);
	
    /* VCOM Setting */
    LCD_WR_REG(0xBB);
    LCD_WR_DATA8(0x37);
	
    /* LCM Control */
    LCD_WR_REG(0xC0);
    LCD_WR_DATA8(0x2C);
	
    /* VDV and VRH Command Enable */
    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x01);
	
    /* VRH Set */
    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x12);   
	
    /* VDV Set */
    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x20);  
	
    /* Frame Rate Control in Normal Mode */
    LCD_WR_REG(0xC6); 
    LCD_WR_DATA8(0x0F);   
	
    /* Power Control 1 */
    LCD_WR_REG(0xD0); 
    LCD_WR_DATA8(0xA4);
    LCD_WR_DATA8(0xA1);
	
    /* Positive Voltage Gamma Control */
    LCD_WR_REG(0xE0);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x2B);
    LCD_WR_DATA8(0x3F);
    LCD_WR_DATA8(0x54);
    LCD_WR_DATA8(0x4C);
    LCD_WR_DATA8(0x18);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x0B);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x23);
	
    /* Negative Voltage Gamma Control */
    LCD_WR_REG(0xE1);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x04);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x13);
    LCD_WR_DATA8(0x2C);
    LCD_WR_DATA8(0x3F);
    LCD_WR_DATA8(0x44);
    LCD_WR_DATA8(0x51);
    LCD_WR_DATA8(0x2F);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x20);
    LCD_WR_DATA8(0x23);
	
    /* Display Inversion On */
    LCD_WR_REG(0x21);       // 开启反色
    /* Sleep Out */
    LCD_WR_REG(0x11); 

    HAL_Delay(120);
    /* display on */
    LCD_WR_REG(0x29);       // 开启显示

    LCD_Display_Dir(0); 		// 默认为横屏
    lcd_clear(WHITE);
}
