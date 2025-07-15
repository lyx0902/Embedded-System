#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"
#include "stdio.h"
#include "./BSP/LCD/drv_lcd.h"
#include "./BSP/LCD/rttlogo.h"

__IO float IC2ValuePWM = 0;
__IO float DutyCyclePWM = 0;
__IO float FrequencyPWM = 0;
void SystemClock_Config(void);
float FormatTo1Decimal(float value);
float duty = 4000;

// 重定向printf到串口
int fputc(int ch, FILE *f) {
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);
	return ch;
}

// 更新PWM测量值：计算占空比和频率
void UpdatePWMMeasurement(TIM_HandleTypeDef *htim) {
	IC2ValuePWM = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); // 获取周期
	if (IC2ValuePWM != 0) {
		DutyCyclePWM = ((HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2)) * 100) / IC2ValuePWM; // 占空比 = 高电平时间 / 周期
		FrequencyPWM = (HAL_RCC_GetPCLK2Freq() * 2 / (TIM9->PSC + 1)) / IC2ValuePWM; // 频率 = 定时器时钟 / 周期
	} else {
		DutyCyclePWM = 0;
		FrequencyPWM = 0;
	}
}

// 输入捕获中断回调函数，触发更新PWM测量值
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM9 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
		UpdatePWMMeasurement(htim);
	}
}

// 调整PWM占空比（通过左右按键控制）
void AdjustPWMDutyCycle(uint16_t GPIO_Pin) {
	if (HAL_GPIO_ReadPin(GPIOC, GPIO_Pin) == GPIO_PIN_RESET) {
		if (GPIO_Pin == LEFT_Pin && duty > 400) {
			duty -= 400;
			__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, duty);
		}
		if (GPIO_Pin == RIGHT_Pin && duty < 7600) {
			duty += 400;
			__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, duty);
		}
	}
}

// 外部中断回调函数：检测按键并调整PWM占空比
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	HAL_Delay(20); // 简单消抖
	AdjustPWMDutyCycle(GPIO_Pin);
}

// 初始化所有外设
void InitPeripherals(void) {
	MX_GPIO_Init();
	MX_FSMC_Init();
	MX_TIM14_Init();
	MX_USART1_UART_Init();
	MX_TIM2_Init();
	MX_TIM9_Init();

	// 启动定时器和PWM
	HAL_TIM_Base_Start_IT(&htim14);
	HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

	// 启动输入捕获
	HAL_TIM_IC_Start_IT(&htim9, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim9, TIM_CHANNEL_2);

	drv_lcd_init(); // 初始化LCD
}

//时刻检查是否为一位小数
float FormatTo1Decimal(float value) {
    return (float)((int)(value * 10 + 0.5)) / 10.0;
}


// 显示PWM占空比和频率信息
void DisplayPWMInfo(void) {
	char DutyCycle[20];
	char Frequency[20];
	sprintf(DutyCycle, "DutyCycle: %.1f", DutyCyclePWM);   // 保留1位小数
	sprintf(Frequency, "Frequency: %.1f", FrequencyPWM);
	lcd_show_string(30, 70, 24, DutyCycle);
	lcd_show_string(30, 120, 24, Frequency);
}

int main(void) {
	HAL_Init();               // 初始化HAL库
	SystemClock_Config();     // 配置系统时钟
	InitPeripherals();        // 初始化所有外设

	lcd_clear(WHITE);
	lcd_set_color(WHITE, BLACK); // 设置前景背景色

	while (1) {
		DisplayPWMInfo();     // 显示PWM信息
		HAL_Delay(200);       // 延时200ms
	}
}

void SystemClock_Config(void) {
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
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

void Error_Handler(void) {
	__disable_irq();
	while (1) {}
}
