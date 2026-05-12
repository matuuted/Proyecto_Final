/**
 * @file    dev_oled.h
 * @brief   Driver para display OLED 128×64 SSD1315 vía I²C1.
 * @details
 *  API para inicializar el display, limpiar, escribir texto y
 *  mostrar las pantallas predefinidas del dispensador.
 *
 *  Pantallas en ST_IDLE (alternan cada OLED_SCREEN_SWITCH_MS):
 *
 *    PESOS:                   HORA:
 *    ┌─────────────────────┐  ┌─────────────────────┐
 *    │   Guardian Pet      │  │   Guardian Pet      │
 *    │─────────────────────│  │─────────────────────│
 *    │ Tanque:             │  │    08:15:30         │
 *    │   4.250 kg          │  │                     │
 *    │ Plato:              │  │  Lun 12/04/25       │
 *    │   15 g              │  │                     │
 *    └─────────────────────┘  └─────────────────────┘
 *
 *    ST_DISPENSING:           ST_ERROR:
 *    ┌─────────────────────┐  ┌─────────────────────┐
 *    │   Guardian Pet      │  │                     │
 *    │─────────────────────│  │      ERROR          │
 *    │   ** DANDO  **      │  │   Reiniciando...    │
 *    │   ** RACION **      │  │                     │
 *    └─────────────────────┘  └─────────────────────┘
 *
 * @author  Matías Durante
 * @version 1.0
 * @date    2025
 */

#ifndef DEV_OLED_H
#define DEV_OLED_H

#include <stdint.h>
#include <stdbool.h>
#include "oled_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* TIPOS                                                                       */
/* -------------------------------------------------------------------------- */

typedef enum {
    OLED_OK    = 0,
    OLED_ERROR = 1,
} OLED_Status;

/**
 * @brief Datos del sistema para mostrar en el display.
 */
typedef struct {
    float   tankKg;     /**< Peso del tanque en kg.        */
    float   plateGrams; /**< Peso del plato en gramos.     */
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint8_t year;       /**< Últimos 2 dígitos (ej. 25).  */
    uint8_t weekday;    /**< 1=Lun ... 7=Dom.              */
} OLED_Data;

/* -------------------------------------------------------------------------- */
/* API PÚBLICA                                                                 */
/* -------------------------------------------------------------------------- */

/**
 * @brief  Inicializa el SSD1315 con la secuencia de comandos estándar.
 * @return OLED_OK si el display respondió correctamente por I²C.
 */
OLED_Status OLED_Init(void);

/**
 * @brief  Limpia el display (todos los píxeles apagados).
 */
void OLED_Clear(void);

/**
 * @brief  Escribe un string en col/page del buffer interno y lo vuelca.
 * @param  col   Columna en píxeles (0–127).
 * @param  page  Página (fila de 8 px, 0–7).
 * @param  str   String terminado en '\0'.
 */
void OLED_PutString(uint8_t col, uint8_t page, const char *str);

/**
 * @brief  Actualiza el display con los datos actuales del sistema.
 * @note   Alterna internamente entre pantalla de pesos y de hora
 *         cada OLED_SCREEN_SWITCH_MS. Llamar desde SM_Task en ST_IDLE.
 * @param  data  Puntero a los datos actuales.
 */
void OLED_Update(const OLED_Data *data);

/**
 * @brief  Muestra la pantalla de dispensando (ST_DISPENSING).
 */
void OLED_ShowDispensing(void);

/**
 * @brief  Muestra la pantalla de error (ST_ERROR).
 */
void OLED_ShowError(void);

#ifdef __cplusplus
}
#endif

#endif /* DEV_OLED_H */
