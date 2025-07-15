#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "dma.h"
#include "string.h"
#include "stdio.h"

#define MAX_RX_BUFFER_SIZE 200  
#define LED_ON_CMD "*LED-ON*"     
#define LED_OFF_CMD "*LED-OFF*"   

uint8_t rx_buffer[MAX_RX_BUFFER_SIZE];
uint8_t rx_index = 0;

typedef enum {
    RX_MODE_IT = 0,
    RX_MODE_DMA = 1
} RxMode_t;
RxMode_t rx_mode = RX_MODE_IT;

void SystemClock_Config(void);
uint8_t IsNewLineChar(uint8_t ch);
void ProcessReceivedData(void);
void ControlLED_R(uint8_t state);
void ControlLED_B(uint8_t state);
void UART_AbortReceive(void);
void UART_RestartReceive(void);
void change_rx_mode(void);

uint8_t IsNewLineChar(uint8_t ch)
{
    return (ch == '\r' || ch == '\n');
}

void ProcessLEDOnCommand(void)
{
    if (rx_mode == RX_MODE_IT)
    {
        ControlLED_R(1);
        HAL_UART_Transmit(&huart1, (uint8_t *)"LED is ON (red)\r\n", strlen("LED is ON (red)\r\n"), HAL_MAX_DELAY);
    }
    else if (rx_mode == RX_MODE_DMA)
    {
        ControlLED_B(1);
        HAL_UART_Transmit(&huart1, (uint8_t *)"LED is ON (green)\r\n", strlen("LED is ON (green)\r\n"), HAL_MAX_DELAY);
    }
}

void ProcessLEDOffCommand(void)
{
    if (rx_mode == RX_MODE_IT)
    {
        ControlLED_R(0);
        HAL_UART_Transmit(&huart1, (uint8_t *)"LED is OFF (red)\r\n", strlen("LED is OFF (red)\r\n"), HAL_MAX_DELAY);
    }
    else if (rx_mode == RX_MODE_DMA)
    {
        ControlLED_B(0);
        HAL_UART_Transmit(&huart1, (uint8_t *)"LED is OFF (green)\r\n", strlen("LED is OFF (green)\r\n"), HAL_MAX_DELAY);
    }
}

void ProcessEchoCommand(void)
{
    HAL_UART_Transmit(&huart1, rx_buffer, rx_index, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
}


void ProcessReceivedData(void)
{
    rx_buffer[rx_index] = '\0';
    if (strstr((char *)rx_buffer, LED_ON_CMD) != NULL)
    {
        ProcessLEDOnCommand();
    }
    else if (strstr((char *)rx_buffer, LED_OFF_CMD) != NULL)
    {
        ProcessLEDOffCommand();
    }
    else
    {
        ProcessEchoCommand();
    }
    rx_index = 0;
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        if (rx_index < MAX_RX_BUFFER_SIZE - 1)
        {
            if (IsNewLineChar(rx_buffer[rx_index]))
            {
                ProcessReceivedData();
            }
            else
            {
                rx_index++;
            }
        }
        else
        {
            rx_index = 0;
        }
        
        if (rx_mode == RX_MODE_IT)
        {
            if (HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1) != HAL_OK)
            {
                Error_Handler();
            }
        }
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if ((huart == &huart1) && (rx_mode == RX_MODE_DMA))
    {
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        if (Size == 0)
        {
            HAL_UART_DMAStop(&huart1);
            if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer, MAX_RX_BUFFER_SIZE) != HAL_OK)
            {
                HAL_UART_Transmit(&huart1, (uint8_t *)"Restart DMA failed!\r\n", 
                                    strlen("Restart DMA failed!\r\n"), HAL_MAX_DELAY);
                Error_Handler();
            }
            return;
        }
        
        rx_index = Size; 
        rx_buffer[rx_index] = '\0'; 
        ProcessReceivedData();
        
        HAL_UART_DMAStop(&huart1);
        if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer, MAX_RX_BUFFER_SIZE) != HAL_OK)
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)"Restart DMA failed!\r\n", 
                                strlen("Restart DMA failed!\r\n"), HAL_MAX_DELAY);
            Error_Handler();
        }
    }
}

void UART_AbortReceive(void)
{
    if (rx_mode == RX_MODE_IT)
    {
        HAL_UART_AbortReceive_IT(&huart1);
    }
    else if (rx_mode == RX_MODE_DMA)
    {
        HAL_UART_DMAStop(&huart1);
    }
}

void UART_RestartReceive(void)
{
    if (rx_mode == RX_MODE_IT)
    {
        if (HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1) != HAL_OK)
        {
            Error_Handler();
        }
    }
    else if (rx_mode == RX_MODE_DMA)
    {
        if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer, MAX_RX_BUFFER_SIZE) != HAL_OK)
        {
            Error_Handler();
        }
    }
}

void change_rx_mode(void)
{
    UART_AbortReceive();
    if (rx_mode == RX_MODE_IT)
    {
        rx_mode = RX_MODE_DMA;
        char msg[] = "Switched to DMA receive mode\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    }
    else
    {
        rx_mode = RX_MODE_IT;
        char msg[] = "Switched to Interrupt receive mode\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    }
    rx_index = 0;
    UART_RestartReceive();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == LEFT_Pin)
    {
        static uint32_t last_tick = 0;
        uint32_t current_tick = HAL_GetTick();
        if (current_tick - last_tick > 500)
        {
            change_rx_mode();
        }
        last_tick = current_tick;
    }
}

void ControlLED_R(uint8_t state)
{
    if (state)
    {
        HAL_GPIO_WritePin(GPIOF, LED_R_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOF, LED_R_Pin, GPIO_PIN_SET);
    }
}

void ControlLED_B(uint8_t state)
{
    if (state)
    {
        HAL_GPIO_WritePin(GPIOF, LED_B_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(GPIOF, LED_B_Pin, GPIO_PIN_SET);
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();

    if (HAL_UART_Receive_IT(&huart1, &rx_buffer[rx_index], 1) != HAL_OK)
    {
        Error_Handler();
    }
    
    while (1)
    {
			
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
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
