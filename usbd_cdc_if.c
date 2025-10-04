/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @brief          : USB CDC interface source file
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"
#include "main.h"
#include "gpio.h"
#include "mode.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
extern USBD_HandleTypeDef hUsbDeviceFS;   // handle USB device tu usb_device.c

/* Bien extern tu main.c */
extern uint8_t temp;
extern uint8_t humi;
extern char msg[128];

/* USB handler declaration. */
static uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/* Buffer de luu lenh nhan */
static char rxBuffer[64];
static uint8_t rxIndex = 0;

/* Private function prototypes -----------------------------------------------*/
static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len);

/* USB CDC Interface callback structure */
USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Exported functions --------------------------------------------------------*/

/**
  * @brief  CDC_Init_FS
  * @retval USBD_OK
  */
static int8_t CDC_Init_FS(void)
{
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, NULL, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
}

/**
  * @brief  CDC_DeInit_FS
  * @retval USBD_OK
  */
static int8_t CDC_DeInit_FS(void)
{
  return (USBD_OK);
}

/**
  * @brief  CDC_Control_FS
  * @retval USBD_OK
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:
    case CDC_GET_ENCAPSULATED_RESPONSE:
    case CDC_SET_COMM_FEATURE:
    case CDC_GET_COMM_FEATURE:
    case CDC_CLEAR_COMM_FEATURE:
    case CDC_SET_LINE_CODING:
    case CDC_GET_LINE_CODING:
    case CDC_SET_CONTROL_LINE_STATE:
    case CDC_SEND_BREAK:
    default:
      break;
  }
  return (USBD_OK);
}

/**
  * @brief  CDC_Receive_FS
  *         Callback khi nhan data tu PC qua USB CDC
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  for (uint32_t i = 0; i < *Len; i++)
  {
    char c = (char)Buf[i];

    if (c == '\r' || c == '\n') // ket thuc chuoi lenh
    {
      rxBuffer[rxIndex] = '\0';
      rxIndex = 0;

      if (strcmp(rxBuffer, "AUTO") == 0)
      {
        mode = MODE_AUTO;
        CDC_Transmit_FS((uint8_t*)"Switched to AUTO\r\n", 18);
      }
      else if (strcmp(rxBuffer, "MANUAL") == 0)
      {
        mode = MODE_MANUAL;

        // Khi vào MANUAL: t?t h?t LED ngay
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);

        CDC_Transmit_FS((uint8_t*)"Switched to MANUAL\r\n", 20);
      }
      else if (mode == MODE_MANUAL)  
      {
        if (strcmp(rxBuffer, "LED_GREEN ON") == 0)
        {
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
          CDC_Transmit_FS((uint8_t*)"LED_GREEN ON\r\n", 14);
        }
        else if (strcmp(rxBuffer, "LED_GREEN OFF") == 0)
        {
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
          CDC_Transmit_FS((uint8_t*)"LED_GREEN OFF\r\n", 15);
        }
        else if (strcmp(rxBuffer, "LED_YELLOW ON") == 0)
        {
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
          CDC_Transmit_FS((uint8_t*)"LED_YELLOW ON\r\n", 15);
        }
        else if (strcmp(rxBuffer, "LED_YELLOW OFF") == 0)
        {
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
          CDC_Transmit_FS((uint8_t*)"LED_YELLOW OFF\r\n", 16);
        }
        else if (strcmp(rxBuffer, "LED_RED ON") == 0)
        {
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
          CDC_Transmit_FS((uint8_t*)"LED_RED ON\r\n", 12);
        }
        else if (strcmp(rxBuffer, "LED_RED OFF") == 0)
        {
          HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
          CDC_Transmit_FS((uint8_t*)"LED_RED OFF\r\n", 13);
        }
        else if (strcmp(rxBuffer, "STATUS") == 0)
        {
          snprintf(msg, sizeof(msg), "Temp: %d C, Humi: %d %%\r\n", temp, humi);
          CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
        }
        else
        {
          CDC_Transmit_FS((uint8_t*)"Unknown command\r\n", 17);
        }
      }
    }
    else
    {
      if (rxIndex < sizeof(rxBuffer) - 1)
      {
        rxBuffer[rxIndex++] = c; 
      }
    }
  }

  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
}

/**
  * @brief  CDC_Transmit_FS
  *        
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*) hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0)
  {
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  return result;
}
