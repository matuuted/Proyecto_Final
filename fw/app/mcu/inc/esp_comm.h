/**
 * @file    esp32_comm.h
 * @brief   Puente de comunicación entre STM32 y ESP32 via UART4.
 *
 * @details
 *  Este módulo maneja toda la comunicación JSON entre el STM32 y el ESP32.
 *
 *  Mensajes que RECIBE del ESP32 (comandos de la App):
 *    {"cmd":"dispense","amount":50}
 *    {"cmd":"tare"}
 *    {"cmd":"set_schedules","schedules":[...]}
 *
 *  Mensajes que ENVÍA al ESP32 (telemetría y eventos):
 *    {"type":"telemetry","tank_weight":...,"plate_weight":...,"datetime":{...}}
 *    {"type":"feeding_done","amount":25,"label":"Almuerzo","hour":10,"minute":15}
 *    {"type":"tare_done"}
 *
 *  Tarea RTOS:
 *    CommTask — bloquea en osThreadFlagsWait hasta que UART4 IRQ
 *    notifica que llegó una línea completa, luego la parsea y actúa.
 *
 * @author  Matías Durante
 * @version 1.0
 * @date    2025
 */

#ifndef ESP_COMM_H
#define ESP_COMM_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "app_sm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* CONFIGURACIÓN                                                               */
/* -------------------------------------------------------------------------- */

/** @brief Flag de notificación RTOS que CommTask espera. */
#define COMM_UART_RX_FLAG   0x01U

/** @brief Intervalo de envío de telemetría al ESP32 (ms). */
#define COMM_INTERVAL_MS   1000U

/** @brief Máximo de horarios que puede recibir del ESP32. */
#define COMM_MAX_SCHEDULES  8U

/* -------------------------------------------------------------------------- */
/* TIPOS                                                                       */
/* -------------------------------------------------------------------------- */

/**
 * @brief Horario dinámico recibido desde la App via ESP32.
 */
typedef struct {
    uint8_t hour;
    uint8_t minute;
    char    label[16];
    bool    active;
} Comm_Schedule;

/**
 * @brief Datos de telemetría que el STM32 envía al ESP32.
 * @note  Rellenar antes de llamar a Comm_SendTelemetry().
 */
typedef struct {
    float   tankWeight;   /**< Peso del tanque (kg).   */
    float   plateWeight;  /**< Peso del plato (g).     */
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint8_t year;         /**< Últimos 2 dígitos.      */
    uint8_t weekday;      /**< 1=Lun ... 7=Dom.        */
} Comm_Data;

/* -------------------------------------------------------------------------- */
/* API PÚBLICA                                                                 */
/* -------------------------------------------------------------------------- */

/**
 * @brief  Inicializa el bridge y crea CommTask.
 * @note   Llamar desde main() antes de osKernelStart().
 * @return HAL_OK si la tarea se creó correctamente, HAL_ERROR si no.
 */
HAL_StatusTypeDef Comm_InitOS(void);

/**
 * @brief  Envía telemetría al ESP32.
 * @note   Llamar periódicamente desde CommTask o SMTask.
 * @param  t  Puntero a la estructura con los datos actuales.
 */
void Comm_SendData(const Comm_Data *t);

/**
 * @brief  Notifica al ESP32 que se completó una dispensación.
 * @param  amount  Gramos dispensados.
 * @param  label   Etiqueta ("Desayuno", "Almuerzo", "Manual", etc.).
 * @param  hour    Hora de la dispensación.
 * @param  minute  Minuto de la dispensación.
 */
void Comm_SendFeedingDone(uint8_t amount, const char *label,
                          uint8_t hour, uint8_t minute);

/**
 * @brief  Notifica al ESP32 que la tara se completó.
 */
void Comm_SendTareDone(void);

/**
 * @brief  Obtiene el schedule dinámico recibido desde la App.
 * @param  out    Buffer destino (tamaño mínimo: COMM_MAX_SCHEDULES).
 * @param  count  Cantidad de horarios copiados (salida).
 */
void Comm_GetSchedules(Comm_Schedule *out, uint8_t *count);

/**
 * @brief  Tarea RTOS — no llamar directamente.
 */
void Comm_Task(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* ESP_COMM_H */