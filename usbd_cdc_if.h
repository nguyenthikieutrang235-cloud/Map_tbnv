/**
  ******************************************************************************
  * @file           : usbd_cdc_if.h
  * @brief          : Header for usbd_cdc_if.c file
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CDC_IF_H__
#define __USBD_CDC_IF_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc.h"

/* Exported defines ----------------------------------------------------------*/
#define APP_RX_DATA_SIZE  64
#define APP_TX_DATA_SIZE  64

/* Exported variables --------------------------------------------------------*/
extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;

/* Exported functions ------------------------------------------------------- */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CDC_IF_H__ */
