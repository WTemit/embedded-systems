/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "kb.h"
#include "sdk_uart.h"
#include "pca9538.h"
#include "oled.h"
#include "fonts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void KB_Test( void );
void OLED_KB( uint8_t OLED_Keys[]);
void oled_Reset( void );
void Timer( void );
void Buzzer_Init( void );
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  oled_Init();
  Buzzer_Init();

  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  Timer();
	  HAL_Delay(20);

  }
  /* USER CODE END 3 */
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
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
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

#include <stdio.h>
#include <string.h>

#define KEY_NONE   0xFF
#define KEY_CLEAR  10    // клавиша '*'
#define KEY_START  11    // клавиша '#'
#define BEEP_ON()   (TIM1->CCR1 = 250)
#define BEEP_OFF()  (TIM1->CCR1 = 0)

void Buzzer_Init( void )
{
	GPIO_InitTypeDef g = {0};

	__HAL_RCC_GPIOE_CLK_ENABLE();
	g.Pin       = GPIO_PIN_9;
	g.Mode      = GPIO_MODE_AF_PP;
	g.Pull      = GPIO_NOPULL;
	g.Speed     = GPIO_SPEED_FREQ_LOW;
	g.Alternate = GPIO_AF1_TIM1;               // PE9 = TIM1_CH1
	HAL_GPIO_Init(GPIOE, &g);

	__HAL_RCC_TIM1_CLK_ENABLE();
	TIM1->PSC   = 167;                         // 168 МГц / 168 = 1 МГц
	TIM1->ARR   = 499;                         // период 500 мкс → 2 кГц
	TIM1->CCR1  = 0;                           // сначала тишина
	TIM1->CCMR1 = TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE;  // PWM mode 1
	TIM1->CCER  = TIM_CCER_CC1E;               // выход канала 1 включён
	TIM1->BDTR  = TIM_BDTR_MOE;                // у TIM1 нужен главный выход
	TIM1->CR1   = TIM_CR1_ARPE | TIM_CR1_CEN;  // запуск
	TIM1->EGR   = TIM_EGR_UG;                  // применить значения
}

static uint8_t alarm_entered = 0;

typedef enum { INPUT, RUN, ALARM } state_t;

static state_t  state     = INPUT;
static uint8_t  digits[4] = {0};
static uint8_t  digit_cnt = 0;
static uint32_t total_sec = 0;
static uint8_t  redraw    = 1;
static uint32_t last_tick = 0;

void oled_Reset( void ) {
	oled_Fill(Black);
	oled_SetCursor(0, 0);
	oled_UpdateScreen();
}

static uint8_t key_get(void)
{
	static uint8_t prev = KEY_NONE;
	static const uint8_t rows[4]   = { ROW1, ROW2, ROW3, ROW4 };
	static const uint8_t keymap[4][3] = {
		{ 1,         2, 3         },
		{ 4,         5, 6         },
		{ 7,         8, 9         },
		{ KEY_CLEAR, 0, KEY_START }
	};

	uint8_t cur = KEY_NONE;
	for (int i = 0; i < 4 && cur == KEY_NONE; i++) {
		uint8_t k = Check_Row(rows[i]);
		if      (k == 0x04) cur = keymap[i][0];
		else if (k == 0x02) cur = keymap[i][1];
		else if (k == 0x01) cur = keymap[i][2];
	}

	if (cur != prev) {
		prev = cur;
		return cur;
	}
	return KEY_NONE;
}

static void show_time(uint8_t m, uint8_t s)
{
	char buf[8];
	snprintf(buf, sizeof(buf), "%02u:%02u", m, s);
	oled_Reset();
	oled_WriteString("Timer of Apocalypse", Font_7x10, White);
	oled_SetCursor(0, 14);
	oled_WriteString(buf, Font_11x18, White);
	oled_UpdateScreen();
}

void Timer( void )
{
	switch (state)
	{
	case INPUT:
	{
		uint8_t key = key_get();

		if (key <= 9)
		{
			if (digit_cnt < 4)
			{
				digits[0] = digits[1];
				digits[1] = digits[2];
				digits[2] = digits[3];
				digits[3] = key;
				digit_cnt++;
				redraw = 1;
			}
		}
		else if (key == KEY_CLEAR)
		{
			memset(digits, 0, sizeof(digits));
			digit_cnt = 0;
			redraw = 1;
		}
		else if (key == KEY_START)
		{
			total_sec = (uint32_t)(digits[0]*10 + digits[1]) * 60
			          + (digits[2]*10 + digits[3]);
			if (total_sec > 0)
			{
				last_tick = HAL_GetTick();
				state  = RUN;
				break;
			}

		}

		if (redraw)
		{
			show_time(digits[0]*10 + digits[1], digits[2]*10 + digits[3]);
			redraw = 0;
		}
		break;
	}

	case RUN:
	{
		uint8_t key = key_get();
		if (key == KEY_CLEAR)
		{
			memset(digits, 0, sizeof(digits));
			digit_cnt = 0;
			redraw = 1;
			state = INPUT;
			break;
		}
		if (HAL_GetTick() - last_tick >= 1000)      // прошла секунда
		{
			last_tick += 1000;
			total_sec--;
			show_time(total_sec / 60, total_sec % 60);

			if (total_sec == 0)
			{
				alarm_entered = 0;
				state = ALARM;
			}
		}
		break;

	}
		break;

	case ALARM:
	{
		static uint32_t beep_tick = 0;
		static uint8_t  beep_state = 0;

		if (!alarm_entered)
		{
			alarm_entered = 1;
			beep_state = 1;
			beep_tick = HAL_GetTick();
			BEEP_ON();
			oled_SetCursor(0, 40);
			oled_WriteString("TIME IS UP!", Font_7x10, White);
			oled_UpdateScreen();
		}

		if (HAL_GetTick() - beep_tick >= 300)
		{
			beep_tick += 300;
			beep_state ^= 1;
			if (beep_state) BEEP_ON(); else BEEP_OFF();
		}

		if (key_get() != KEY_NONE)
		{
			BEEP_OFF();
			memset(digits, 0, sizeof(digits));
			digit_cnt = 0;
			redraw = 1;
			state = INPUT;
		}
		break;
	}
	break;
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
