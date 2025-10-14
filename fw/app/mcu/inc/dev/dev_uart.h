/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    dev_uart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DEV_UART_H__
#define __DEV_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// Tiempo máximo de espera para operaciones UART
#define UART_TIMEOUT_MS 2000

// Tamaño máximo del buffer para enviar datos por UART
#define UART_TX_MAX_SIZE 512


/* Funciones públicas --------------------------------------------------------*/
/**
 * @brief Inicializa la UART.
 * @return true si se inicializó bien, false si hubo algún problema en la configuracion.
 */
bool uartInit();

/**
 * @brief Desinicializa la UART.
 * @return true si se finalizo correctamente, false si hubo algún problema.
 */
bool uartDeInit();

/**
 * @brief Envía una cadena de bytes por UART finalizada en '\0'.
 * @param pstring Puntero a la cadena a enviar.
 * @return void
 */
void uartSendString(uint8_t * pstring);

/**
 * @brief Envía una cantidad específica de bytes por UART.
 * @param pstring Puntero al buffer de datos.
 * @param size Cantidad de bytes a enviar.
 * @return void
 */
void uartSendStringSize(uint8_t * pstring, uint16_t size);

/**
 * @brief Recibe una cantidad de bytes por UART.
 * @param pstring Puntero al buffer donde se guardarán los datos.
 * @param size Cantidad de bytes a recibir.
 * @return void
 */
void uartReceiveStringSize(uint8_t * pstring, uint16_t size);


#ifdef __cplusplus
}
#endif

#endif /* __DEV_UART_H__ */

