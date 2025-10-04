/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body - DHT11 + USB CDC + LED control (AUTO/MANUAL)
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "DHT11.h"
#include "usbd_cdc_if.h"
#include "mode.h"

#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
uint8_t temp = 0, humi = 0;
char msg[128];
uint32_t lastTick = 0;   // m?c th?i gian cho AUTO mode
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void CDC_Send_String(char *s);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void CDC_Send_String(char *s)
{
  uint16_t len = (uint16_t)strlen(s);
  if (len == 0) return;

  while (CDC_Transmit_FS((uint8_t*)s, len) == USBD_BUSY)
  {
    HAL_Delay(1);
  }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_USB_DEVICE_Init();

  HAL_TIM_Base_Start(&htim1);

  CDC_Send_String("STM32 DHT11 USB CDC ready\r\n");

  while (1)
  {
    if (mode == MODE_AUTO)
    {
      if (HAL_GetTick() - lastTick >= 10000) 
      {
        lastTick = HAL_GetTick();

        uint8_t success = 0;
        for (int i = 0; i < 3; ++i)
        {
          if (DHT11_Read(&temp, &humi) == 0)
          {
            success = 1;
            break;
          }
          HAL_Delay(100); 
        }

        if (success)
        {
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, (humi < 40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, (humi >= 40 && humi <= 70) ? GPIO_PIN_SET : GPIO_PIN_RESET);
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, (humi > 70) ? GPIO_PIN_SET : GPIO_PIN_RESET);

          snprintf(msg, sizeof(msg), "Temp: %d C, Humi: %d %%\r\n", temp, humi);
          CDC_Send_String(msg);
        }
        else
        {
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_SET);
          CDC_Send_String("DHT11 Error\r\n");
        }
      }
    }
    else // MODE_MANUAL
    {
      HAL_Delay(50);
    }
  }
}

/**
  * @brief System Clock Configuration
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) Error_Handler();
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
    HAL_Delay(200);
  }
}

/* USER CODE END 4 */

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add custom report */
}
#endif /* USE_FULL_ASSERT */
