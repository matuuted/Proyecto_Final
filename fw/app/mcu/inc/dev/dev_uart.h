/**
  ******************************************************************************
  * @file    dev_uart.h
  * @brief   UART4 driver — TX bloqueante + RX por interrupción para ESP32 bridge.
  ******************************************************************************
  * @attention
  * Copyright (c) 2025 STMicroelectronics. All rights reserved.
  ******************************************************************************
  */

#ifndef __DEV_UART_H__
#define __DEV_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* Configuración --------------------------------------------------------------*/

/** @brief Timeout para operaciones TX bloqueantes (ms). */
#define UART_TIMEOUT_MS     2000

/** @brief Tamaño máximo del buffer TX. */
#define UART_TX_MAX_SIZE    512

/** @brief Tamaño del buffer circular de RX.
 *  Debe ser mayor que el JSON más largo esperado del ESP32. */
#define UART_RX_BUF_SIZE    512

/* Funciones públicas — TX (sin cambios) -------------------------------------*/

/**
 * @brief  Inicializa UART4 y activa la recepción por interrupción.
 * @param  rxTaskHandle  Handle de la tarea que será notificada cuando
 *                       llegue una línea completa del ESP32. Puede ser
 *                       NULL si aún no se usa RX.
 * @return true si OK, false si hubo error.
 */
bool uartInit(osThreadId_t rxTaskHandle);

/**
 * @brief  Desinicializa UART4.
 * @return true si OK, false si hubo error.
 */
bool uartDeInit(void);

/**
 * @brief  Envía una cadena terminada en '\0' por UART4 (bloqueante).
 * @param  pstring  Puntero a la cadena.
 */
void uartSendString(uint8_t *pstring);

/**
 * @brief  Envía N bytes por UART4 (bloqueante).
 * @param  pstring  Puntero al buffer.
 * @param  size     Cantidad de bytes.
 */
void uartSendStringSize(uint8_t *pstring, uint16_t size);

/**
 * @brief  Recibe N bytes por UART4 (bloqueante). Uso legacy.
 * @param  pstring  Puntero al buffer destino.
 * @param  size     Cantidad de bytes a recibir.
 */
void uartReceiveStringSize(uint8_t *pstring, uint16_t size);

/* Funciones públicas — RX por IRQ -------------------------------------------*/

/**
 * @brief  Copia la última línea completa recibida del ESP32 al buffer destino.
 * @note   Llamar desde ESP32_Task después de recibir la notificación.
 * @param  dest     Buffer destino (debe tener al menos UART_RX_BUF_SIZE bytes).
 * @param  maxLen   Tamaño máximo a copiar.
 * @return Longitud de la línea copiada (0 si no hay línea disponible).
 */
uint16_t uartGetLine(char *dest, uint16_t maxLen);

/**
 * @brief  Llamar desde UART4_IRQHandler en stm32f4xx_it.c.
 * @note   Ya está conectado internamente — no necesitás hacer nada extra
 *         si usás HAL, que llama a HAL_UART_IRQHandler automáticamente.
 */
void UART4_IRQHandler(void);


#ifdef __cplusplus
}
#endif

#endif /* __DEV_UART_H__ */