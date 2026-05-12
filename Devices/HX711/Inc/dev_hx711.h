/**
 * @file    dev_hx711.h
 * @brief   Driver para dos módulos HX711 — galga tanque (3/5 kg) y plato (1 kg).
 * @details
 *  Ganancia: Canal A 128x — resolución ~0.05g (1 kg) / ~0.15g (3/5 kg).
 *
 *  La tara se dispara desde la App vía BLE → ESP32 → UART → CommTask →
 *  handle_tare() → HX711_TareAll() → Comm_SendTareDone() → App.
 *
 * @author  Matías Durante
 * @version 1.0
 * @date    2025
 */

#ifndef DEV_HX711_H
#define DEV_HX711_H

#include <stdint.h>
#include <stdbool.h>
#include "hx711_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* TIPOS                                                                       */
/* -------------------------------------------------------------------------- */

typedef enum {
    HX711_OK      = 0,
    HX711_ERROR   = 1,
    HX711_TIMEOUT = 2,
} HX711_Status;

/* -------------------------------------------------------------------------- */
/* API PÚBLICA                                                                 */
/* -------------------------------------------------------------------------- */

/**
 * @brief  Inicializa GPIO y despierta ambos HX711.
 * @return HX711_OK si ambas galgas responden.
 */
HX711_Status HX711_Init(void);

/**
 * @brief  Lee el peso en gramos de una galga (promedio de HX711_AVG_SAMPLES).
 * @note   Bloqueante ~100 ms. Llamar desde SM_Task en ST_IDLE o CommTask.
 * @param  ch     Canal (HX711_CH_TANK / HX711_CH_PLATE).
 * @param  grams  Resultado en gramos (salida).
 * @return HX711_OK si OK.
 */
HX711_Status HX711_ReadGrams(uint8_t ch, float *grams);

/**
 * @brief  Tara una galga (promedia en vacío y guarda el offset).
 * @param  ch  Canal a tarar.
 * @return HX711_OK si OK.
 */
HX711_Status HX711_Tare(uint8_t ch);

/**
 * @brief  Tara ambas galgas en secuencia.
 * @return HX711_OK si ambas tararon bien.
 */
HX711_Status HX711_TareAll(void);

/**
 * @brief  Ajusta el factor de escala (crudo/gramo) para calibración.
 * @note   Procedimiento: tarar → colocar peso conocido → leer crudo →
 *         scale = (crudo - offset) / peso_gramos
 * @param  ch     Canal.
 * @param  scale  Factor de escala.
 */
void HX711_SetScale(uint8_t ch, float scale);

/**
 * @brief  Pone ambos HX711 en power-down (bajo consumo).
 */
void HX711_PowerDown(void);

/**
 * @brief  Despierta ambos HX711 del power-down.
 */
void HX711_PowerUp(void);

#ifdef __cplusplus
}
#endif

#endif /* DEV_HX711_H */
