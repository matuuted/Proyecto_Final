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

/**
 * @brief  Cola circular de líneas recibidas — profundidad UART_RX_QUEUE_DEPTH.
 * @note   Evita perder el comando "tare" cuando llega mientras hay telemetría
 *         pendiente de procesar.
 */
#define UART_RX_QUEUE_DEPTH  4U

typedef struct {
    char     data[UART_RX_BUF_SIZE];
    uint16_t len;
} RxLine_t;

static RxLine_t  s_rx_queue[UART_RX_QUEUE_DEPTH];
static volatile uint8_t s_rx_q_head = 0;  /* IRQ escribe acá */
static volatile uint8_t s_rx_q_tail = 0;  /* Tarea lee acá  */

/** @brief Handle de la tarea a notificar cuando llega una línea completa. */
static osThreadId_t s_rx_task_handle = NULL;

#define UART_RX_FLAG  0x01U

/* =========================================================================== */
/*                          FUNCIONES PÚBLICAS — INIT                          */
/* =========================================================================== */

bool uartInit(osThreadId_t rxTaskHandle)
{
    s_rx_task_handle = rxTaskHandle;
    s_rx_line_idx    = 0;
    s_rx_q_head      = 0;
    s_rx_q_tail      = 0;

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

    HAL_NVIC_SetPriority(UART4_IRQn, 5, 0); // Prioridad para FreeRTOS (5 a 15)
    HAL_NVIC_EnableIRQ(UART4_IRQn);

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
    if (dest == NULL || maxLen == 0) return 0;

    /* Cola vacía */
    if (s_rx_q_tail == s_rx_q_head) return 0;

    RxLine_t *entry = &s_rx_queue[s_rx_q_tail];
    uint16_t len = (entry->len < maxLen - 1) ? entry->len : maxLen - 1;
    memcpy(dest, entry->data, len);
    dest[len] = '\0';

    /* Avanzar tail */
    s_rx_q_tail = (s_rx_q_tail + 1) % UART_RX_QUEUE_DEPTH;

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
        /* Ignorar CR */
    }
    else if (c == '\n') {
        if (s_rx_line_idx > 0) {
            /* Calcular próxima posición del head */
            uint8_t next_head = (s_rx_q_head + 1) % UART_RX_QUEUE_DEPTH;

            if (next_head != s_rx_q_tail) {
                /* Hay espacio en la cola — guardar línea */
                RxLine_t *entry = &s_rx_queue[s_rx_q_head];
                memcpy(entry->data, s_rx_line_buf, s_rx_line_idx);
                entry->len = s_rx_line_idx;
                s_rx_q_head = next_head;

                /* Notificar a CommTask */
                if (s_rx_task_handle != NULL) {
                    osThreadFlagsSet(s_rx_task_handle, UART_RX_FLAG);
                }
            }
            /* Si la cola está llena, la línea se descarta — raro pero seguro */
        }
        s_rx_line_idx = 0;
    }
    else {
        if (s_rx_line_idx < UART_RX_BUF_SIZE - 1) {
            s_rx_line_buf[s_rx_line_idx++] = c;
        } else {
            /* Overflow — descartar línea */
            s_rx_line_idx = 0;
        }
    }

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

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == UART4) {
        // Limpiar error y rearmar la recepción
        HAL_UART_Receive_IT(huart, &s_rx_byte, 1);
    }
}
