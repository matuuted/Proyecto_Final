/**
  ******************************************************************************
  * @file    dev_uart.c
  * @brief   UART4 driver — TX bloqueante + RX por interrupción para ESP32 bridge.
  *
  * @details
  *  Estrategia de RX:
  *   1. HAL_UART_Receive_IT() arranca la recepción de 1 byte en s_rx_byte.
  *   2. HAL_UART_RxCpltCallback() se ejecuta en contexto de IRQ por cada byte.
  *   3. El byte se acumula en s_rx_line_buf hasta encontrar '\n'.
  *   4. Al encontrar '\n', la línea completa se copia a s_rx_ready_buf y
  *      se notifica a s_rx_task_handle via osThreadFlagsSet().
  *   5. La tarea correspondiente lee la línea con uartGetLine() y la procesa.
  *
  ******************************************************************************
  * @attention
  * Copyright (c) 2025 STMicroelectronics. All rights reserved.
  ******************************************************************************
  */

#include "dev_uart.h"

/* =========================================================================== */
/*                          VARIABLES PRIVADAS                                 */
/* =========================================================================== */

static UART_HandleTypeDef huart4;

/** @brief Buffer de 1 byte usado por HAL_UART_Receive_IT. */
static uint8_t  s_rx_byte = 0;

/** @brief Buffer de acumulación de la línea en curso. */
static char     s_rx_line_buf[UART_RX_BUF_SIZE];
static uint16_t s_rx_line_idx = 0;

/** @brief Buffer con la última línea completa lista para leer. */
static char     s_rx_ready_buf[UART_RX_BUF_SIZE];
static uint16_t s_rx_ready_len = 0;

/** @brief Flag: hay una línea lista para leer (acceso desde IRQ y tarea). */
static volatile bool s_rx_line_ready = false;

/** @brief Handle de la tarea a notificar cuando llega una línea completa. */
static osThreadId_t s_rx_task_handle = NULL;

/** @brief Flag de notificación que la tarea espera con osThreadFlagsWait. */
#define UART_RX_FLAG  0x01U

/* =========================================================================== */
/*                          FUNCIONES PÚBLICAS — INIT                          */
/* =========================================================================== */

bool uartInit(osThreadId_t rxTaskHandle)
{
    s_rx_task_handle = rxTaskHandle;
    s_rx_line_idx    = 0;
    s_rx_line_ready  = false;

    huart4.Instance          = UART4;
    huart4.Init.BaudRate     = 115200;
    huart4.Init.WordLength   = UART_WORDLENGTH_8B;
    huart4.Init.StopBits     = UART_STOPBITS_1;
    huart4.Init.Parity       = UART_PARITY_NONE;
    huart4.Init.Mode         = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart4) != HAL_OK) {
        return false;
    }

    /* Arrancar la primera recepción por IRQ — se re-arma sola en el callback */
    if (HAL_UART_Receive_IT(&huart4, &s_rx_byte, 1) != HAL_OK) {
        return false;
    }

    /* Mensaje de confirmación por UART */
    uint8_t msg[UART_TX_MAX_SIZE];
    int len = snprintf((char *)msg, sizeof(msg),
        "\r\n[UART4] Init OK — 115200 8N1, RX por IRQ activo\r\n");
    if (len > 0) uartSendString(msg);

    return true;
}

bool uartDeInit(void)
{
    HAL_UART_Abort(&huart4);
    return (HAL_UART_DeInit(&huart4) == HAL_OK);
}

/* =========================================================================== */
/*                          FUNCIONES PÚBLICAS — TX                            */
/* =========================================================================== */

void uartSendString(uint8_t *pstring)
{
    if (pstring == NULL) return;
    HAL_UART_Transmit(&huart4, pstring, strlen((char *)pstring), UART_TIMEOUT_MS);
}

void uartSendStringSize(uint8_t *pstring, uint16_t size)
{
    if (pstring == NULL || size == 0) return;
    HAL_UART_Transmit(&huart4, pstring, size, UART_TIMEOUT_MS);
}

/* Legacy bloqueante — mantener por compatibilidad */
void uartReceiveStringSize(uint8_t *pstring, uint16_t size)
{
    if (pstring == NULL || size == 0) return;
    HAL_UART_Receive(&huart4, pstring, size, UART_TIMEOUT_MS);
}

/* =========================================================================== */
/*                          FUNCIONES PÚBLICAS — RX                            */
/* =========================================================================== */

uint16_t uartGetLine(char *dest, uint16_t maxLen)
{
    if (!s_rx_line_ready || dest == NULL || maxLen == 0) return 0;

    uint16_t len = (s_rx_ready_len < maxLen - 1) ? s_rx_ready_len : maxLen - 1;
    memcpy(dest, s_rx_ready_buf, len);
    dest[len] = '\0';

    s_rx_line_ready = false;
    return len;
}

/* =========================================================================== */
/*                     CALLBACK DE HAL — CONTEXTO IRQ                         */
/* =========================================================================== */

/**
 * @brief  Se ejecuta automáticamente por HAL cuando llega 1 byte por UART4.
 * @note   Acumula bytes hasta '\n', luego notifica a la tarea correspondiente.
 *         Se re-arma solo para seguir recibiendo.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != UART4) return;

    char c = (char)s_rx_byte;

    if (c == '\r') {
        /* Ignorar CR — el protocolo usa '\n' como delimitador */
    }
    else if (c == '\n') {
        /* Línea completa — copiar al buffer de salida si no hay una pendiente */
        if (!s_rx_line_ready && s_rx_line_idx > 0) {
            memcpy(s_rx_ready_buf, s_rx_line_buf, s_rx_line_idx);
            s_rx_ready_len  = s_rx_line_idx;
            s_rx_line_ready = true;

            /* Notificar a la tarea correspondiente desde IRQ */
            if (s_rx_task_handle != NULL) {
                osThreadFlagsSet(s_rx_task_handle, UART_RX_FLAG);
            }
        }
        s_rx_line_idx = 0; /* Resetear buffer de acumulación */
    }
    else {
        /* Acumular byte — proteger contra overflow */
        if (s_rx_line_idx < UART_RX_BUF_SIZE - 1) {
            s_rx_line_buf[s_rx_line_idx++] = c;
        } else {
            /* Buffer overflow: descartar línea y empezar de nuevo */
            s_rx_line_idx = 0;
        }
    }

    /* Re-armar la recepción para el siguiente byte */
    HAL_UART_Receive_IT(huart, &s_rx_byte, 1);
}

/* =========================================================================== */
/*                              IRQ HANDLER                                    */
/* =========================================================================== */

/**
 * @brief  Handler de interrupción de UART4.
 */
void UART4_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart4);
}