#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"


#include "flash.h"
#include "stdio.h"
//#include "stdlib.h"
#include "./BSP/LCD/drv_lcd.h"
#include "./BSP/LCD/rttlogo.h"

#define BUFFER_SIZE 27
#define KEY_LEFT_PIN GPIO_PIN_0
#define KEY_RIGHT_PIN GPIO_PIN_4
#define KEY_PORT GPIOC


#define SCK_PIN GPIO_PIN_5
#define SCK_PORT GPIOA
#define MOSI_PIN GPIO_PIN_7
#define MOSI_PORT GPIOA
#define MISO_PIN GPIO_PIN_6
#define MISO_PORT GPIOA
#define CS_PIN GPIO_PIN_12
#define CS_PORT GPIOB

#define W25Q64_CMD_WRITE_ENABLE   0x06
#define W25Q64_CMD_READ_STATUS    0x05
#define W25Q64_CMD_READ_DATA      0x03
#define W25Q64_CMD_PAGE_PROGRAM   0x02






uint16_t 		FlashID = 0;
uint8_t 		WriteBuf[BUFFER_SIZE]="",ReadBuf[BUFFER_SIZE]="";
uint8_t write_checksum, read_checksum;
uint8_t current_mode = 0;
uint8_t changed=1;

void SystemClock_Config(void);

// fprintf()???
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);
	return ch;
}


// ???????
uint8_t calculate_checksum(uint8_t *data, uint16_t length) {
    uint8_t checksum = 0;
    for (uint16_t i = 0; i < length; i++) {
        checksum += data[i];
    }
    return checksum;
}

uint32_t get_time(void) {
    return HAL_GetTick();
}

uint32_t getCurrentMicros(void)
{
  uint32_t m0 = HAL_GetTick();
  __IO uint32_t u0 = SysTick->VAL;
  uint32_t m1 = HAL_GetTick();
  __IO uint32_t u1 = SysTick->VAL;
  const uint32_t tms = SysTick->LOAD + 1;

  if (m1 != m0) {
    return (m1 * 1000 + ((tms - u1) * 1000) / tms);
  } else {
    return (m0 * 1000 + ((tms - u0) * 1000) / tms);
  }
}


uint8_t Key_Read(void) {
    uint8_t key = 0;
    if (HAL_GPIO_ReadPin(KEY_PORT, KEY_LEFT_PIN) == GPIO_PIN_RESET) {
        HAL_Delay(50); // ???
        if (HAL_GPIO_ReadPin(KEY_PORT, KEY_LEFT_PIN) == GPIO_PIN_RESET) {
            key = 0;
					changed=1;
        }
    } else if (HAL_GPIO_ReadPin(KEY_PORT, KEY_RIGHT_PIN) == GPIO_PIN_RESET) {
        HAL_Delay(50); // ???
        if (HAL_GPIO_ReadPin(KEY_PORT, KEY_RIGHT_PIN) == GPIO_PIN_RESET) {
            key = 1;
					changed=1;
        }
    }
    return key;
}

void Soft_SPI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* ??GPIOA?GPIOB?? */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();  // CS????

    /* ??SCK(PA5):???? */
    GPIO_InitStruct.Pin = SCK_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(SCK_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, GPIO_PIN_RESET);

    /* ??MOSI(PA7):???? */
    GPIO_InitStruct.Pin = MOSI_PIN;
    HAL_GPIO_Init(MOSI_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(MOSI_PORT, MOSI_PIN, GPIO_PIN_RESET);

    /* ??MISO(PA6):?? */
    GPIO_InitStruct.Pin = MISO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(MISO_PORT, &GPIO_InitStruct);
}

/**
  * @brief  ????,????SPI??,??1??
  */
static void Soft_SPI_Delay(void)
{
    HAL_Delay(1);
}

/**
  * @brief  ??SPI??1????(MSB??),????????
  * @param  data ??????
  * @retval ??????
  */
uint8_t Soft_SPI_Transfer(uint8_t data)
{
    uint8_t received = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        /* ??data?????MOSI */
        if(data & 0x80)
            HAL_GPIO_WritePin(MOSI_PORT, MOSI_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(MOSI_PORT, MOSI_PIN, GPIO_PIN_RESET);

        Soft_SPI_Delay();

        /* SCK??? */
        HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, GPIO_PIN_SET);
        Soft_SPI_Delay();

        /* ??MISO?? */
        received <<= 1;
        if(HAL_GPIO_ReadPin(MISO_PORT, MISO_PIN) == GPIO_PIN_SET)
            received |= 0x01;

        /* SCK??? */
        HAL_GPIO_WritePin(SCK_PORT, SCK_PIN, GPIO_PIN_RESET);
        Soft_SPI_Delay();

        data <<= 1;
    }
    return received;
}

/**
  * @brief  ????????W25Q64
  */
void Soft_W25Q64_WriteEnable(void)
{
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
    Soft_SPI_Transfer(W25Q64_CMD_WRITE_ENABLE);
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  ??W25Q64?????
  * @retval ???????
  */
uint8_t Soft_W25Q64_ReadStatus(void)
{
    uint8_t status;
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
    Soft_SPI_Transfer(W25Q64_CMD_READ_STATUS);
    status = Soft_SPI_Transfer(0xFF);  // ????????????
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
    return status;
}

/**
  * @brief  ?W25Q64????
  * @param  addr ????(24?)
  * @param  buf ??????????
  * @param  len ??????
  */
void Soft_W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint16_t len)
{
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
    Soft_SPI_Transfer(W25Q64_CMD_READ_DATA);
    Soft_SPI_Transfer((addr >> 16) & 0xFF);
    Soft_SPI_Transfer((addr >> 8) & 0xFF);
    Soft_SPI_Transfer(addr & 0xFF);
    for(uint16_t i = 0; i < len; i++)
    {
        buf[i] = Soft_SPI_Transfer(0xFF);
    }
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  ?W25Q64???????
  * @param  addr ????(24?)
  * @param  buf ?????????
  * @param  len ?????(????????,???256??)
  */
void Soft_W25Q64_PageProgram(uint32_t addr, uint8_t *buf, uint16_t len)
{
    Soft_W25Q64_WriteEnable();
    
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
    Soft_SPI_Transfer(W25Q64_CMD_PAGE_PROGRAM);
    Soft_SPI_Transfer((addr >> 16) & 0xFF);
    Soft_SPI_Transfer((addr >> 8) & 0xFF);
    Soft_SPI_Transfer(addr & 0xFF);
    
    for(uint16_t i = 0; i < len; i++)
    {
        Soft_SPI_Transfer(buf[i]);
    }
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);

    /* ???????BUSY?(bit0),?????? */
    while(Soft_W25Q64_ReadStatus() & 0x01)
    {
        Soft_SPI_Delay();
    }
}

/* ????? */
void Soft_W25QXX_Write(uint8_t *buf, uint32_t addr, uint16_t len)
{
    Soft_W25Q64_PageProgram(addr, buf, len);
}

/* ????? */
void Soft_W25QXX_Read(uint8_t *buf, uint32_t addr, uint16_t len)
{
    Soft_W25Q64_ReadData(addr, buf, len);
}


void HD_SPI_Mode_Task(void){
	
	uint32_t i;
	lcd_clear(WHITE);
			lcd_show_string(15, 15, 32,"HD SPI Mode");
			
			//write
			for(i=0;i<26;i++) WriteBuf[i]=(i)%26+'A';		
			WriteBuf[BUFFER_SIZE - 1] = '\0';
	
	
  write_checksum = calculate_checksum(WriteBuf, BUFFER_SIZE);
	
	
	
	uint32_t write_start_time = getCurrentMicros();
	W25QXX_Write(WriteBuf,0,27);
	uint32_t write_end_time = getCurrentMicros();
  uint32_t write_duration = write_end_time - write_start_time;
	
	printf("Write duration: %u us\n", write_duration);
	
	
	HAL_Delay(1000);
	
	
	//read
	uint32_t read_start_time = getCurrentMicros();
	W25QXX_Read(ReadBuf,0,27);
	uint32_t read_end_time = getCurrentMicros();
	uint32_t read_duration = read_end_time - read_start_time;
	
	//printf("read_end_time: %u us\n", read_end_time);
	//printf("read_start_time: %u us\n", read_start_time);
	printf("Read duration: %u us\n", read_duration);
	
	HAL_Delay(1000);
	ReadBuf[BUFFER_SIZE - 1] = '\0';
	
  read_checksum = calculate_checksum(ReadBuf, BUFFER_SIZE);
	
	
	//check
	if (write_checksum == read_checksum) {
        //LCD_ShowString(10, 139, 16, (uint8_t *)"Data Check Passed");
			lcd_show_string(20, 55, 16, "Data Check Passed");
   } else {
        //LCD_ShowString(10, 139, 16, (uint8_t *)"Data Check Failed");
			lcd_show_string(20, 55, 16, "Data Check Failed");
   }
	 
	lcd_show_string(20, 75, 16,"Read Data:");
	lcd_show_string(20, 95, 16,(const char *)ReadBuf);
	 
	 //speed
	 float write_speed = (float)BUFFER_SIZE*1000 / write_duration ;
   float read_speed = (float)BUFFER_SIZE*1000 / read_duration ;

   char write_speed_str[20];
   char read_speed_str[20];
   sprintf(write_speed_str, "Write: %.2f KB/s", write_speed);
   sprintf(read_speed_str, "Read: %.2f KB/s", read_speed);
	 
	 lcd_show_string(20, 120, 16,"Speed showing activate:");
	 lcd_show_string(20, 140, 16,write_speed_str);
	 lcd_show_string(20, 170, 16,read_speed_str);
	 lcd_show_string(20,200,16,"922106840127_SPI");
	 
	 //printf
	 printf("Write Speed: %.2f KB/s\n", write_speed);
   printf("Read Speed: %.2f KB/s\n", read_speed);
	
	
	
	
	
}




void Soft_SPI_Mode_Task(void)
{
    uint32_t i;
    // ??????
    lcd_clear(0xFFFF);  // ??WHITE????0xFFFF
    lcd_show_string(15, 15, 32, "GPIO SPI Mode");

    /* ????? */
    for(i = 0; i < 26; i++)
    {
        WriteBuf[i] = (i % 26) + 'A';
    }
    WriteBuf[BUFFER_SIZE - 1] = '\0';
    write_checksum = calculate_checksum(WriteBuf, BUFFER_SIZE);

		
		//lcd_show_string(10, 30, 16, (const char *)WriteBuf);
		
		
    /* ????? */
    uint32_t write_start_time = getCurrentMicros();
    Soft_W25QXX_Write(WriteBuf, 0, 27);
    uint32_t write_end_time = getCurrentMicros();
    uint32_t write_duration = write_end_time - write_start_time;
    printf("Write duration: %u us\n", write_duration);
    HAL_Delay(1000);

    /* ????? */
    uint32_t read_start_time = getCurrentMicros();
    Soft_W25QXX_Read(ReadBuf, 0, 27);
    uint32_t read_end_time = getCurrentMicros();
    uint32_t read_duration = read_end_time - read_start_time;
    printf("Read duration: %u us\n", read_duration);
		W25QXX_Read(ReadBuf,0,27);
		
    HAL_Delay(1000);
    ReadBuf[BUFFER_SIZE - 1] = '\0';
    read_checksum = calculate_checksum(ReadBuf, BUFFER_SIZE);

    /* ???? */
    if (write_checksum == read_checksum)
    {
        lcd_show_string(20, 55, 16, "Data Check Passed");
    }
    else
    {
        lcd_show_string(20, 55, 16, "Data Check Failed");
    }

    lcd_show_string(20, 75, 16, "Read Data:");
    lcd_show_string(20, 95, 16, (const char *)ReadBuf);

    /* ???? */
    float write_speed = (float)BUFFER_SIZE * 1000 / write_duration;
    float read_speed  = (float)BUFFER_SIZE * 1000 / read_duration;
    char write_speed_str[20];
    char read_speed_str[20];
    sprintf(write_speed_str, "Write: %.2f KB/s", write_speed);
    sprintf(read_speed_str, "Read: %.2f KB/s", read_speed);
    lcd_show_string(20, 120, 16, "Speed:");
    lcd_show_string(20, 140, 16, write_speed_str);
    lcd_show_string(20, 160, 16, read_speed_str);
		lcd_show_string(20,200,16,"922106840127_SPI");
    printf("Write Speed: %.2f KB/s\n", write_speed);
    printf("Read Speed: %.2f KB/s\n", read_speed);
}



int main(void)
{

	//uint8_t 		i;
  HAL_Init();
  SystemClock_Config();
	
  MX_GPIO_Init();
	MX_SPI2_Init();
	MX_USART1_UART_Init();
	
  MX_FSMC_Init();
  MX_TIM14_Init();
	
	HAL_TIM_Base_Start_IT(&htim14);
	HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
  
	drv_lcd_init();		// ???LCD
	
	lcd_clear(WHITE);
	lcd_set_color(WHITE, BLACK);
	
	//lcd_show_string(10, 69, 16, "Hello, RT-Thread!");
	
	
	
	FlashID = W25QXX_ReadID();
	while((FlashID = W25QXX_ReadID()) != W25Q64)
	{
		//printf("W25Q128 Check Failed!!!!!!!!!" );
		printf("W25Q128 Check Failed!!!!!!!!!! FlashID = 0x%X\n", FlashID); // ??????FlashID
		HAL_Delay(1000);
		FlashID = W25QXX_ReadID();
	}
	printf("Flash_ID = 0x%X\n", FlashID);
	//lcd_show_num(10,89,FlashID,10,32);
	
	//lcd_show_string(10, 99, 32, "Hello, RT-Thread!");
	//for(i=0;i<26;i++) WriteBuf[i]=(i+2)%26+'A';		
	
	
	
  while (1)
  {
		uint8_t key = Key_Read();
		if(changed==1 && key==0){
			
			
	 
	 
	 
	 HD_SPI_Mode_Task();
	 changed=0;
			
			
			
			
		}else if(changed==1 && key==1){
			Soft_SPI_Mode_Task();
      changed = 0;
			
			
			
		
		
		}
		
		
		
		
		
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


