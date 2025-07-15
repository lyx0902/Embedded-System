#include "main.h"
#include "usart.h"
#include "gpio.h"
#include <string.h> 

#define MAX_RX_BUFFER_SIZE 200  // 接收缓冲区大小

uint8_t rx_buffer[MAX_RX_BUFFER_SIZE];  // 接收缓冲区
uint8_t rx_index = 0;                   // 当前接收位置

void SystemClock_Config(void);
uint8_t IsNewLineChar(uint8_t ch);
void ProcessReceivedData(void);

/**
  * @brief 判断是否为换行符
  * @param ch: 字符
  * @retval 1 是换行符，0 否
  */
uint8_t IsNewLineChar(uint8_t ch)
{
    return (ch == '\r' || ch == '\n');
}

/**
  * @brief 处理接收到的一行数据
  */
void ProcessReceivedData(void)
{
    rx_buffer[rx_index] = '\0';  // 添加结束符
    HAL_UART_Transmit(&huart1, rx_buffer, rx_index, HAL_MAX_DELAY);  // 回显
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY); // 添加换行
    rx_index = 0;  // 重置索引
}

/**
  * @brief 串口接收中断回调
  * @param huart: 串口句柄
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        if (rx_index < MAX_RX_BUFFER_SIZE - 1)
        {
            if (IsNewLineChar(rx_buffer[rx_index]))
            {
                ProcessReceivedData();  // 检测到换行符，处理数据
            }
            else
            {
                rx_index++;  // 等待下一字符
            }
        }
        else
        {
            rx_index = 0;  // 溢出保护，清空
        }

        // 启动下一次接收
        if (HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1) != HAL_OK)
        {
            Error_Handler(); 
        }
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    // 启动串口接收中断
    if (HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1) != HAL_OK)
    {
        Error_Handler();
    }
    while (1){}
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

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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
    while (1){}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
   
}
#endif /* USE_FULL_ASSERT */
