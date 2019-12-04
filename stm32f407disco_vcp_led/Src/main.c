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
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
enum {
	SERIAL_PACKET__NONE,
	SERIAL_PACKET__FLASH_WRITE,
	SERIAL_PACKET__FLASH_READ,
	SERIAL_PACKET__LED,
};

struct serial_data_t {
	bool	 valid;
	uint32_t opcode;
	uint8_t  buf[560];
	uint32_t len;
};

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
static struct serial_data_t serial_data;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*
 * Convert ascii byte to on hex byte
 */
static uint8_t ascii2hex(uint8_t val) {
	uint8_t res;
	if (val >= '0' && val <= '9') {
		res = val - '0';
	} else if (val >= 'a' && val <= 'f') {
		res = val - 'a' + 10;
	} else if (val >= 'A' && val <= 'F') {
		res = val - 'A' + 10;
	}

	return res;
}

/*
* convert two bytes of ascii to one byte hex.
*/
static uint32_t s_ascii_to_hex(uint16_t index, uint16_t size) {
	uint32_t res=0;
	uint32_t val;

	for (int i = 0; i < size; i++) {
		val = ascii2hex(serial_data.buf[index+i]) << (i*4);
		res |= val;
	}
	return res;
}

/*
 *  return the opcode , opcode is first 4 bytes of the data.
 */
static uint16_t s_get_opcode(void) {
	return (uint16_t)s_ascii_to_hex(0, 2*2);
}

/*
 * return led number and state
 */
static void led_get(uint8_t *led, uint8_t *state) {
	uint16_t val = (uint16_t)s_ascii_to_hex(4, 2*2);
	*led = val & 0xff;
	*state = (val >> 8) & 0xff;
}

/*
 *  toggle the led based on led packet.
 */
static void s_led_packet(void) {
	uint8_t led;
	uint8_t state;
	led_get(&led, &state);
	switch(led) {
	case 0:
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
		break;
	case 1:
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13,  state ? GPIO_PIN_SET : GPIO_PIN_RESET);
		break;
	case 2:
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14,  state ? GPIO_PIN_SET : GPIO_PIN_RESET);
		break;
	case 3:
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15,  state ? GPIO_PIN_SET : GPIO_PIN_RESET);
		break;
	}
}

/*
 * parse the packet
 */
static void parse_packet(void) {
	switch(s_get_opcode()) {
		case SERIAL_PACKET__LED:
			s_led_packet();
			break;
		default:
			break;
	}
}

/*
 * Packet start with '[' and end with ']'
 * data in between is ascii.
 * first 4 bytes is opcode
 * next 8 bytes is address
 * 4 bytes after address is the size
 * [opcode(2) address(4) size(2) data....]
 * One byte of hex is two bytes of ascii.
 *   opcode is an uint16_t -- 2*2 = 4bytes
 *   address is an uint32_t -- 4*2 = 8bytes
 *   size is an uint16_t -- 2*2 = 4bytes
 * */
static void vcp_store_data(uint8_t *ptr, uint32_t len) {
	for (uint32_t i=0; i < len; i++){
		switch (ptr[i]) {
		case '[': //begin packet receiving
			serial_data.len = 0;
			serial_data.valid=true;
			continue;
		case ']': //endpacket receiving
			serial_data.valid=false;
			parse_packet();
			continue;
		default: //strore the data in the buffer
			if (serial_data.valid) {
				serial_data.buf[serial_data.len++] = ptr[i];
			}
			continue;
		}
	}
}

/*
 * Call by the usb interupt.
 */
void CDC_ReceiveCallBack(uint8_t *buf, uint32_t len) {
	vcp_store_data(buf, len);
}


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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
#if 0
  /*
   * create a flash instance to store the led data.
   * Passing a pointer to the led data and the size.
   */
  flash_create(&flash_user_data, (uint8_t *)&led, sizeof(struct led_t));
  /*
   * update the led based on the led data
   */
  update_led();
  /*
   * create a task to monitor the led data.
   */
  xTaskCreate(flash_update, "flash update", 128, NULL, 0, NULL);
#endif
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 5 */
  //osDelay(5000);
  /* Infinite loop */
  for(;;)
  {
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
    osDelay(200);
  }
  /* USER CODE END 5 */ 
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
