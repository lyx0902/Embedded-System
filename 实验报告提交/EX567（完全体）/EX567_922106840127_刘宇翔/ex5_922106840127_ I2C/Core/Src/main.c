#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"
#include "stdio.h"
#include "./BSP/LCD/drv_lcd.h"
#include "./BSP/LCD/rttlogo.h"

#define AP3216C_CONFIG_REG      0x00
#define AP3216C_IR_DATA_L       0x0A
#define AP3216C_ALS_DATA_L      0x0C
#define AP3216C_PS_DATA_L       0x0E
#define AP3216C_ALS_CONFIG_REG  0x10
#define AP3216C_ADDR            0x3C

extern I2C_HandleTypeDef hi2c2;
void SystemClock_Config(void);

enum als_range {
    AP3216C_ALS_RANGE_20661, //Resolution = 0.35 lux/count(default)
    AP3216C_ALS_RANGE_5162,  //Resolution = 0.0788 lux/count
    AP3216C_ALS_RANGE_1291,  //Resolution = 0.0197 lux/count
    AP3216C_ALS_RANGE_323    //Resolution = 0.0049 lux/count
};

//fprintf()
int fputc(int ch, FILE *f) {
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);
    return ch;
}

uint8_t AP3216C_WriteOneByte(uint8_t reg, uint8_t data) {
    return HAL_I2C_Mem_Write(&hi2c2, AP3216C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

uint8_t AP3216C_ReadOneByte(uint8_t reg) {
    uint8_t data;
    HAL_I2C_Mem_Read(&hi2c2, AP3216C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    return data;
}

float RoundToOneDecimal(float value) {
    return ((int)(value * 10 + 0.5)) / 10.0f;
}

void AP3216C_Init(void) {
    AP3216C_WriteOneByte(AP3216C_CONFIG_REG, 0x04); // Reset
    HAL_Delay(50);
    AP3216C_WriteOneByte(AP3216C_CONFIG_REG, 0x03); // Enable ALS + PS + IR

    if (AP3216C_ReadOneByte(AP3216C_CONFIG_REG) == 0x03) {
        printf("AP3216C Init OK!\r\n");
    } else {
        printf("AP3216C Init Failed!\r\n");
    }
}

uint16_t AP3216C_Read_PS(void) {
    uint32_t buf[2];
    uint32_t read_data;
    uint16_t proximity = 0;

    for (int i = 0; i < 2; i++) {
        buf[i] = AP3216C_ReadOneByte(AP3216C_PS_DATA_L + i);
    }

    read_data = buf[0] + (buf[1] << 8);
    proximity = (read_data & 0x000F) + (((read_data >> 8) & 0x3F) << 4);

    return proximity;
}

float AP3216C_Read_ALS(void) {
    uint32_t buf[2];
    uint32_t read_data;
    float brightness = 0;

    for (int i = 0; i < 2; i++) {
        buf[i] = AP3216C_ReadOneByte(AP3216C_ALS_DATA_L + i);
    }
		
    read_data = buf[0] + (buf[1] << 8);
    uint8_t range = (AP3216C_ReadOneByte(AP3216C_ALS_CONFIG_REG) >> 4) & 0x03;
		
    switch (range) {
        case AP3216C_ALS_RANGE_20661: brightness = 0.35 * read_data; break;
        case AP3216C_ALS_RANGE_5162:  brightness = 0.0788 * read_data; break;
        case AP3216C_ALS_RANGE_1291:  brightness = 0.0197 * read_data; break;
        case AP3216C_ALS_RANGE_323:   brightness = 0.0049 * read_data; break;
        default: break;
    }
    return brightness;
}

void LCD_Init_Display(void) {
    drv_lcd_init();
    lcd_clear(WHITE);
    lcd_set_color(WHITE, BLACK);
}

void Display_Sensor_Data(uint16_t ps_data, float als_data) {
		char buffer[32];
    
    float als_formatted = RoundToOneDecimal(als_data);

    sprintf(buffer, "PS: %.1f", (float)ps_data);
    lcd_show_string(30, 70, 32, buffer);

    sprintf(buffer, "ALS: %.1f", als_formatted);
    lcd_show_string(30, 110, 32, buffer);

    printf("PS: %d\r\n", ps_data);
    printf("ALS: %.1f\r\n", als_formatted);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_FSMC_Init();
    MX_I2C2_Init();
    MX_TIM14_Init();
    MX_USART1_UART_Init();

    HAL_TIM_Base_Start_IT(&htim14);
    HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);

    LCD_Init_Display();
    AP3216C_Init();

    while (1) {
        uint16_t ps = AP3216C_Read_PS();
        float als = AP3216C_Read_ALS();
        Display_Sensor_Data(ps, als);
        HAL_Delay(2000);
    }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1){ }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line){}
#endif /* USE_FULL_ASSERT */
