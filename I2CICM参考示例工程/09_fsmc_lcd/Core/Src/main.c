
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

#include "stdio.h"
//#include "stdlib.h"
#include "./BSP/LCD/drv_lcd.h"
#include "./BSP/LCD/rttlogo.h"

#include "math.h"


#define ICM20608_ADDR 0x68  // 假设AD0接地，地址为0x68
#define ICM20608_CONFIG_REG 0x1A        // configuration:fifo, ext sync and dlpf
#define ICM20608_GYRO_CONFIG_REG 0x1B   // gyroscope configuration
#define ICM20608_ACCEL_CONFIG1_REG 0x1C // accelerometer configuration
#define ICM20608_ACCEL_CONFIG2_REG 0x1D // accelerometer configuration
#define ICM20608_INT_ENABLE_REG 0x38    // interrupt enable
#define ICM20608_ACCEL_MEAS 0x3B        // accelerometer measurements
#define ICM20608_GYRO_MEAS 0x43         // gyroscope measurements
#define ICM20608_PWR_MGMT1_REG 0x6B     // power management 1
#define ICM20608_PWR_MGMT2_REG 0x6C     // power management 2

#define FILTER_SIZE 5
#define M_PI 3.1415927f
// 假设加速度传感器满量程为 ±2g，1g = 9.81 m/s²，量化值 16384
#define ACCEL_SENSITIVITY 16384.0f
// 陀螺仪转换因子（单位：度/秒），根据实际传感器调整
#define GYRO_SENSITIVITY 1.0f


int16_t accel[3];
int16_t gyro[3];

typedef struct {
    float buffer[FILTER_SIZE];
    uint8_t index;
} MeanFilter;

typedef struct {
    float prev_output;
    float alpha; // α = dt / (dt + 1/(2πf_cutoff))
} LPF;

float ax_filtered,ay_filtered,az_filtered;
LPF lpf_ax,lpf_ay,lpf_az;

	   // 初始化滤波器
MeanFilter accel_filter_x, accel_filter_y, accel_filter_z;
LPF lpf_gyro_z = { .alpha = 0.1f }; // 按需调整α
LPF lpf_gyro_x = { .alpha = 0.1f }; // 按需调整α
LPF lpf_gyro_y = { .alpha = 0.1f }; // 按需调整α


void ICM20608_Init(void) {
    // 退出睡眠模式，选择时钟源（PLL）
    uint8_t data = 0x01; // CLKSEL=1, SLEEP=0
    HAL_I2C_Mem_Write(&hi2c2, (ICM20608_ADDR << 1), ICM20608_PWR_MGMT1_REG, 1, &data, 1, 100);

    // 设置陀螺仪量程 ±250dps
    data = 0x00; // FS_SEL=0
    HAL_I2C_Mem_Write(&hi2c2, (ICM20608_ADDR << 1), ICM20608_GYRO_CONFIG_REG, 1, &data, 1, 100);

    // 设置加速度计量程 ±2g
    data = 0x00; // AFS_SEL=0
    HAL_I2C_Mem_Write(&hi2c2, (ICM20608_ADDR << 1), ICM20608_ACCEL_CONFIG1_REG, 1, &data, 1, 100);

    // 配置陀螺仪DLPF带宽 92Hz
    data = 0x06; // DLPF_CFG=6
    HAL_I2C_Mem_Write(&hi2c2, (ICM20608_ADDR << 1), ICM20608_CONFIG_REG, 1, &data, 1, 100);

    // 配置加速度计DLPF带宽 99Hz
    data = 0x02; // DLPF_CFG=2
    HAL_I2C_Mem_Write(&hi2c2, (ICM20608_ADDR << 1), ICM20608_ACCEL_CONFIG2_REG, 1, &data, 1, 100);
}
void ICM20608_EnableDataReadyInterrupt(void) {
    uint8_t data = 0x01; // DATA_RDY_EN=1
    HAL_I2C_Mem_Write(&hi2c2, (ICM20608_ADDR << 1), ICM20608_INT_ENABLE_REG, 1, &data, 1, 100);
}
void ICM20608_ReadRawData(int16_t *accel, int16_t *gyro) {
    uint8_t buffer[14];
    HAL_I2C_Mem_Read(&hi2c2, (ICM20608_ADDR << 1), ICM20608_ACCEL_MEAS, 0x00000001U, buffer, 14, 100);
		HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c2, (ICM20608_ADDR << 1), ICM20608_ACCEL_MEAS, I2C_MEMADD_SIZE_8BIT, buffer, 14, 100);
	
		if (status != HAL_OK) {
        printf("I2C Read Error: %d\r\n", status);
        return;
    }

    // 解析加速度和陀螺仪数据（高字节在前）
    accel[0] = (buffer[0] << 8) | buffer[1];  // X轴
    accel[1] = (buffer[2] << 8) | buffer[3];  // Y轴
    accel[2] = (buffer[4] << 8) | buffer[5];  // Z轴
    //printf("Accel X: %d, Y: %d, Z: %d\r\n", accel[0], accel[1], accel[2]);

    // 解析陀螺仪数据
    gyro[0] = (buffer[8]  << 8) | buffer[9];   // X轴
    gyro[1] = (buffer[10] << 8) | buffer[11];   // Y轴
    gyro[2] = (buffer[12] << 8) | buffer[13];   // Z轴
}


// 使用加速度计数据计算姿态角
void CalculateAngles(float ax, float ay, float az, float *pitch, float *roll) {
    *pitch = atan2f(ay, sqrtf(ax * ax + az * az)) * 180.0f / M_PI;
    *roll = atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / M_PI;
}


float MeanFilter_Update(MeanFilter *filter, float new_val) {
    filter->buffer[filter->index] = new_val;
    filter->index = (filter->index + 1) % FILTER_SIZE;
    float sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++) sum += filter->buffer[i];
    return sum / FILTER_SIZE;
}



float LPF_Update(LPF *filter, float input) {
    filter->prev_output = filter->alpha * input + (1 - filter->alpha) * filter->prev_output;
    return filter->prev_output;
}

void LCD_Display(float pitch, float roll, float yaw, float ax, float ay, float az, float gx, float gy, float gz) {
    char buffer[32];

    // 清屏
   // lcd_clear(WHITE);

    // 显示加速度
    sprintf(buffer, "Ax: %.3f ", ax);
    lcd_show_string(10, 0,24,buffer);
	sprintf(buffer, "Ay: %.3f ", ay);
   lcd_show_string(10, 25,24,buffer);
	sprintf(buffer, "Az: %.3f ", az);
   lcd_show_string(10, 50,24,buffer);

    // 显示陀螺仪
    sprintf(buffer, "Gx: %.1f ", gx);
    lcd_show_string(10, 75,24,buffer);
	sprintf(buffer, "Gy: %.1f ", gy);
    lcd_show_string(10, 100,24,buffer);
	sprintf(buffer, "Gz: %.1f ", gz);
    lcd_show_string(10, 125,24,buffer);
	

    // 显示姿态角
    sprintf(buffer, "Pitch: %.1f", pitch);
    lcd_show_string(10, 150,24,buffer);
    sprintf(buffer, "Roll: %.1f", roll);
    lcd_show_string(10, 170,24,buffer);
    sprintf(buffer, "Yaw: %.1f", yaw);
    lcd_show_string(10, 190,24,buffer);
		
		lcd_show_string(10,210,24,"922106840127_I2CICM");
}



/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

// fprintf()重定向
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);
	return ch;
}



int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_FSMC_Init();
  MX_TIM14_Init();
	HAL_TIM_Base_Start_IT(&htim14);
	HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
	drv_lcd_init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();
	ICM20608_Init();
	
	lcd_clear(WHITE);
	

	    // 初始化均值滤波器的缓冲区为0，并设置初始索引
  for (int i = 0; i < FILTER_SIZE; i++) {
      accel_filter_x.buffer[i] = 0.0f;
      accel_filter_y.buffer[i] = 0.0f;
      accel_filter_z.buffer[i] = 0.0f;
  }
	
  accel_filter_x.index = 0;
  accel_filter_y.index = 0;
  accel_filter_z.index = 0;
    
	    // 初始化低通滤波器参数和初始输出
  lpf_ax.alpha = 0.1f; lpf_ax.prev_output = 0.0f;
  lpf_ay.alpha = 0.1f; lpf_ay.prev_output = 0.0f;
  lpf_az.alpha = 0.1f; lpf_az.prev_output = 0.0f;
    
    // 定义其他局部变量
  float pitch, roll, yaw = 0.0f;
	
	
  while (1)
  {
		// 读取传感器原始数据
    ICM20608_ReadRawData(accel, gyro);
		
		// 转换为物理量
    // 将加速度原始数据转换为物理量，假设传感器量程为 ±2g，且 1g = 9.81 m/s²
    float ax = accel[0] * 9.81f / ACCEL_SENSITIVITY;
    float ay = accel[1] * 9.81f / ACCEL_SENSITIVITY;
    float az = accel[2] * 9.81f / ACCEL_SENSITIVITY;

		
    // 滤波处理
		// 加速度数据滤波处理：先均值滤波，再低通滤波
    ax_filtered = LPF_Update(&lpf_ax, MeanFilter_Update(&accel_filter_x, ax));
    ay_filtered = LPF_Update(&lpf_ay, MeanFilter_Update(&accel_filter_y, ay));
    az_filtered = LPF_Update(&lpf_az, MeanFilter_Update(&accel_filter_z, az));
		

        // 计算姿态角
		CalculateAngles(ax_filtered, ay_filtered, az_filtered, &pitch, &roll);
		
		
		// 将陀螺仪原始数据转换为物理量（单位：度/秒）
    float gx = gyro[0] * GYRO_SENSITIVITY;
    float gy = gyro[1] * GYRO_SENSITIVITY;
    float gz = gyro[2] * GYRO_SENSITIVITY;
		
		// 对陀螺仪Z轴数据进行低通滤波处理
    float gz_filtered = LPF_Update(&lpf_gyro_z, gz);
		float gx_filtered = LPF_Update(&lpf_gyro_x, gx);
		float gy_filtered = LPF_Update(&lpf_gyro_y, gy);

     // 计算航向角变化：积分陀螺仪Z轴角速度（假设采样间隔 dt = 10ms）
     yaw += gz_filtered * 0.01f;
		 
		 printf("Yaw: %.2f\r\n", yaw);  // 保留两位小数

		 // 显示到LCD
     LCD_Display(pitch, roll, yaw, ax_filtered, ay_filtered, az_filtered, gx_filtered, gy_filtered, gz_filtered);
		 
		 HAL_Delay(50); // 控制采样率
		
		
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
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

  /** Initializes the CPU, AHB and APB buses clocks
  */
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

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
