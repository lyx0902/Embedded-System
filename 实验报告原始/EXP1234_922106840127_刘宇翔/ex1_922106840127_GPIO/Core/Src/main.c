#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f4xx_hal.h"

// Pin definitions for buttons, LEDs, and buzzer
#define BTN_LEFT_PIN        GPIO_PIN_0
#define BTN_RIGHT_PIN       GPIO_PIN_4
#define BTN_UP_PIN          GPIO_PIN_5
#define BTN_DOWN_PIN        GPIO_PIN_1
#define LED_R_PIN           GPIO_PIN_12
#define LED_B_PIN           GPIO_PIN_11
#define BUZZER_PIN          GPIO_PIN_0

// Port definitions for buttons, LEDs, and buzzer
#define BTN_GPIO_PORT       GPIOC
#define LED_GPIO_PORT       GPIOF
#define BUZZER_PORT         GPIOB

// Frequency definitions (used in sound generation)
#define SIREN_FREQ_1        800   
#define SIREN_FREQ_2        1200  
#define EV_FREQ             1000  

// Alarm duration in milliseconds
#define ALARM_DURATION      5000  

#endif /* __MAIN_H */

#include "main.h"
#include "gpio.h"

// Function prototypes
void SystemClock_Config(void);

volatile uint8_t btn_left_flag = 0;
volatile uint8_t btn_right_flag = 0;
volatile uint8_t btn_up_flag = 0;
volatile uint8_t btn_down_flag = 0;

void sound1(void);
void sound2(void);
void delay(uint32_t);

//----------------------------------------------------------------------------
// @brief  Simple software delay loop
// @param  i: Number of iterations for delay
// @note   This delay is not precise and is dependent on the system clock.
//----------------------------------------------------------------------------
void delay(uint32_t i) {
    while(i--);
}

//----------------------------------------------------------------------------
// @brief  Generate a dynamic sound pattern using a buzzer.
// @note   The function toggles the buzzer pin with variable delay to produce a varying tone.
//         The tone changes frequency by decrementing the delay value (i) gradually.
//         It runs for 5000 milliseconds (ALARM_DURATION).
//----------------------------------------------------------------------------
void sound1(void)
{
    uint32_t i = 30000;                         // Initial delay value for sound frequency control
    uint32_t sound1_start = HAL_GetTick();      // Record start time
    while((HAL_GetTick() - sound1_start) <= 5000) // Loop for 5000 ms
    {
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET); // Activate buzzer (start tone)
        delay(i);                                                    // Delay controls tone duration
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);    // Deactivate buzzer (stop tone)
        delay(i);
        i = i - 6;                         // Gradually decrease delay to change frequency
        if(i <= 0)
            i = 30000;                     // Reset delay value if it becomes too small
    }
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET); // Ensure buzzer is off after sound
}

//----------------------------------------------------------------------------
// @brief  Generate another dynamic sound pattern using a buzzer.
// @note   Similar to sound1(), but with different initial delay value, resulting in a different tone.
//         Runs for 5000 milliseconds.
//----------------------------------------------------------------------------
void sound2(void)
{
    uint32_t i = 6000;                          // Initial delay value for sound frequency control
    uint32_t sound2_start = HAL_GetTick();      // Record start time
    while((HAL_GetTick() - sound2_start) <= 5000) // Loop for 5000 ms
    {
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET); // Activate buzzer
        delay(i);                                                    // Delay to control tone frequency
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);    // Deactivate buzzer
        delay(i);
        i = i - 6;                         // Decrease delay to modify the tone frequency gradually
        if(i <= 0)
            i = 6000;                      // Reset delay value when it reaches zero
    }
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET); // Ensure buzzer is off after sound
}

//----------------------------------------------------------------------------
// @brief  Simulate an ambulance siren using the buzzer.
// @note   This function toggles the buzzer with specific delays to mimic an ambulance siren sound.
//----------------------------------------------------------------------------
void AmbulanceSiren(void) {
    for (int i = 0; i < 40; i++) {
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);  // Activate buzzer
        HAL_Delay(30);                                              // Delay for 30 ms
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);// Deactivate buzzer
        HAL_Delay(30);                                              // Delay for 30 ms

        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);  // Activate buzzer again
        HAL_Delay(20);                                              // Shorter delay of 20 ms
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);// Deactivate buzzer
        HAL_Delay(20);                                              // Shorter delay of 20 ms
    }
}

//----------------------------------------------------------------------------
// @brief  Simulate an electric siren using the buzzer.
// @note   This function toggles the buzzer with equal delays to create a periodic on-off sound.
//----------------------------------------------------------------------------
void ElectricSiren(void) {
    for (int i = 0; i < 20; i++) {
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);  // Turn on buzzer
        HAL_Delay(100);                                             // Keep buzzer on for 100 ms
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);// Turn off buzzer
        HAL_Delay(100);                                             // Delay for 100 ms before next cycle
    }
}

//----------------------------------------------------------------------------
// @brief  Main program entry point.
// @note   Initializes the HAL, system clock, and GPIOs. In the main loop, it checks for button flags
//         and toggles LEDs or plays sounds accordingly.
//----------------------------------------------------------------------------
int main(void)
{
    HAL_Init();                  // Initialize the HAL Library
    SystemClock_Config();        // Configure the system clock
    MX_GPIO_Init();              // Initialize all configured peripherals (GPIO)

    while (1)
    {
        // Check if left button flag is set and toggle the red LED
        if (btn_left_flag)
        {
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_R_PIN);
            btn_left_flag = 0;
        }
        // Check if right button flag is set and toggle the blue LED
        if (btn_right_flag)
        {
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_B_PIN);
            btn_right_flag = 0;
        }
        // Check if up button flag is set and play sound pattern 1
        if (btn_up_flag)
        {
            sound1();
            btn_up_flag = 0;
        }
        // Check if down button flag is set and play sound pattern 2
        if (btn_down_flag)
        {
            sound2();
            btn_down_flag = 0;
        }
    }
}

//----------------------------------------------------------------------------
// @brief  Callback function for external GPIO interrupts.
// @param  GPIO_Pin: Specifies the pin connected to the external interrupt.
// @note   This function debounces the button press (using a delay) and sets the corresponding flag
//         based on which button was pressed.
//----------------------------------------------------------------------------
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    HAL_Delay(20);  // Simple debounce delay (20 ms)
    if (HAL_GPIO_ReadPin(BTN_GPIO_PORT, GPIO_Pin) == GPIO_PIN_RESET)
    {
        switch (GPIO_Pin)
        {
        case BTN_LEFT_PIN:
            btn_left_flag = 1;
            break;
        case BTN_RIGHT_PIN:
            btn_right_flag = 1;
            break;
        case BTN_UP_PIN:
            btn_up_flag = 1;
            break;
        case BTN_DOWN_PIN:
            btn_down_flag = 1;
            break;
        default:
            break;
        }
    }
}

//----------------------------------------------------------------------------
// @brief  EXTI line interrupt handler for BTN_UP_PIN.
// @note   Calls the HAL GPIO EXTI IRQ handler to process the up button interrupt.
//----------------------------------------------------------------------------
void EXTI2_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(BTN_UP_PIN);
}

//----------------------------------------------------------------------------
// @brief  EXTI line interrupt handler for BTN_DOWN_PIN.
// @note   Calls the HAL GPIO EXTI IRQ handler to process the down button interrupt.
//----------------------------------------------------------------------------
void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(BTN_DOWN_PIN);
}

//----------------------------------------------------------------------------
// @brief  Configure the system clock for STM32F407.
// @note   Sets up the PLL and clock dividers to achieve the desired system clock frequency.
//         This configuration uses the HSE oscillator as the clock source.
//----------------------------------------------------------------------------
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();                          // Enable power control clock
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1); // Configure voltage scaling

    // Configure the main oscillator (HSE) and PLL
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    // Select PLL as system clock source and configure bus clocks dividers
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}
