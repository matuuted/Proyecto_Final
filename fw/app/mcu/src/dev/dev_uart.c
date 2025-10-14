/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dev_uart.c
  * @brief   This file provides code for the configuration
  *          of the UART.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "dev_uart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
static UART_HandleTypeDef huart4;

/* UART4 init function */
bool uartInit(void)
{
  // Buffer para enviar el mensaje de configuración de la UART4 por la UART.
  uint8_t  msg[UART_TX_MAX_SIZE];

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;

  // Si la inicialización de la UART falla, retorna false.
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    return false;
  }

  // Generando el mensaje con la configuración de la UART4.
  int len = snprintf((char*)msg, sizeof(msg),
                  "\r\nUART4: %lu bps, WL=8, Stop=1, Parity=None, Mode=TX/RX, Flow=None\r\n",
                  (unsigned long)huart4.Init.BaudRate);

  // Enviando el mensaje por la UART4 si el tamaño del mensaje es mayor a 0.
  if (len > 0) uartSendString(msg);

  return true;
}


bool uartDeInit(void)
{
  // Si la desinicialización de la UART falla, retorna false.
	if (HAL_UART_DeInit(&huart4) != HAL_OK)
	{
		return false;
	}
	return true;
}


void uartSendString(uint8_t * pstring)
{
  // Si el puntero es nulo, retorna sin hacer nada.
  if (pstring == NULL) return;

  // Envia el string completo hasta el carácter nulo.
  HAL_UART_Transmit(&huart4, pstring, strlen((char*)pstring), UART_TIMEOUT_MS);
}

void uartSendStringSize(uint8_t * pstring, uint16_t size)
{
  // Si el puntero es nulo o el tamaño es 0, retorna sin hacer nada.
  if (pstring == NULL || size == 0) return;

  // Envia el string con el tamaño especificado en el parametro size.
  HAL_UART_Transmit(&huart4, pstring, size, UART_TIMEOUT_MS);
}

void uartReceiveStringSize(uint8_t * pstring, uint16_t size)
{
  // Si el puntero es nulo o el tamaño es 0, retorna sin hacer nada.
  if (pstring == NULL || size == 0) return;

  // Recibe el string con el tamaño especificado en el parametro size.
  HAL_UART_Receive(&huart4, pstring, size, UART_TIMEOUT_MS);
}
