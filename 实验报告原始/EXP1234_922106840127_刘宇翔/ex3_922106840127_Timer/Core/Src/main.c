#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"
#include "stdio.h"
#include "./BSP/LCD/drv_lcd.h"
#include "./BSP/LCD/rttlogo.h"

/* 配置系统时钟函数声明 */
void SystemClock_Config(void);
volatile uint8_t g_hour = 8, g_min = 10, g_sec = 0;
volatile uint8_t g_key_flag = 0;

/* 按键状态结构体，用于消抖处理 */
typedef struct {
  uint8_t debounce_cnt;
  uint8_t stable_state;
  uint8_t last_state;
} KeyState;
KeyState keys[3] = {0};  // [0] Left, [1] Down, [2] Right

/* 将printf重定向到UART */
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);
    return ch;
}

/* 更新系统时间：秒、分、时累加 */
void Update_Time(void)
{
  if (++g_sec >= 60) {
    g_sec = 0;
    if (++g_min >= 60) {
      g_min = 0;
      if (++g_hour >= 24)
        g_hour = 0;
    }
  }
}

/* 处理按键消抖，检测按键事件 */
void Process_Key_Debounce(void)
{
  const uint16_t pins[3] = {LEFT_Pin, DOWN_Pin, RIGHT_Pin};
  for (int i = 0; i < 3; i++) {
    uint8_t current = HAL_GPIO_ReadPin(GPIOC, pins[i]);
    if (current == GPIO_PIN_RESET) {
      if (keys[i].debounce_cnt < 2) {
        keys[i].debounce_cnt++;
        if (keys[i].debounce_cnt == 2) { // 消抖：连续检测2次有效按下
          if (keys[i].stable_state == 0) {
            keys[i].stable_state = 1;
            g_key_flag |= (1 << i);
          }
        }
      }
    } else {
      keys[i].debounce_cnt = 0;
      keys[i].stable_state = 0;
    }
  }
}

/* 定时器中断回调函数 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6) {
    Update_Time();
  }
  if (htim->Instance == TIM7) {
    Process_Key_Debounce();
  }
}

/* 初始化各个外设 */
void Init_Peripherals(void)
{
  MX_GPIO_Init();
  MX_FSMC_Init();
  MX_TIM14_Init();
  MX_USART1_UART_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
}

/* 启动定时器及PWM输出 */
void Start_Timers(void)
{
  HAL_TIM_Base_Start_IT(&htim14);
  HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim7);
}

/* 初始化LCD显示 */
void Init_LCD(void)
{
  drv_lcd_init();
  lcd_clear(WHITE);
  lcd_set_color(WHITE, BLACK);
}

/* 处理按键事件，根据按键调整时、分、秒 */
void Handle_Key_Event(void)
{
  if (g_key_flag) {
    if (g_key_flag & 0x01) {  // 左键：调整小时
      g_hour = (g_hour + 1) % 24;
    }
    if (g_key_flag & 0x02) {  // 下键：调整分钟
      g_min = (g_min + 1) % 60;
    }
    if (g_key_flag & 0x04) {  // 右键：调整秒
      g_sec = (g_sec + 1) % 60;
    }
    g_key_flag = 0;
  }
}

/* 更新LCD显示当前时间 */
void Update_LCD_Display(void)
{
  char time_str[20];
  sprintf(time_str, "%02d:%02d:%02d", g_hour, g_min, g_sec);
  lcd_show_string(40, 100, 32, time_str);
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  Init_Peripherals();
  Start_Timers();
  Init_LCD();

  if (HAL_UART_Receive_IT(&huart1, (uint8_t *)&g_key_flag, 1) != HAL_OK)
  {
    Error_Handler();
  }

  while (1)
  {
    Handle_Key_Event();
    Update_LCD_Display();
    HAL_Delay(200);
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

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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
#endif /* USE_FULL_ASSERT */
