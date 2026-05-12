/**
 * @file    hx711_port.h
 * @brief   Primitivas de bajo nivel para HX711 vía GPIO bit-bang.
 * @details
 *  Dos instancias independientes:
 *    HX711_TANK  → PC0 (DOUT) / PC1 (SCK)  — celda 3/5 kg
 *    HX711_PLATE → PC2 (DOUT) / PC3 (SCK)  — celda 1 kg
 */

#ifndef HX711_PORT_H
#define HX711_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "hx711_registers.h"

/* -------------------------------------------------------------------------- */
/* Inicialización de GPIO                                                      */
/* -------------------------------------------------------------------------- */

/**
 * @brief  Configura los 4 pines GPIO (DOUT×2 como input, SCK×2 como output).
 */
void HX711_port_init(void);

/* -------------------------------------------------------------------------- */
/* Primitivas de canal                                                         */
/* -------------------------------------------------------------------------- */

/**
 * @brief  Espera hasta que DOUT = LOW (dato listo) con timeout.
 * @param  ch  Canal (HX711_CH_TANK / HX711_CH_PLATE).
 * @return true si el chip respondió, false si timeout.
 */
bool HX711_port_wait_ready(uint8_t ch);

/**
 * @brief  Lee 24 bits del HX711 y genera el pulso de modo (Canal A Gain 128).
 * @param  ch  Canal.
 * @return Valor crudo de 24 bits (sin conversión a int32).
 */
uint32_t HX711_port_read_raw(uint8_t ch);

/**
 * @brief  Pone SCK en HIGH (>60µs) para entrar en power-down.
 * @param  ch  Canal.
 */
void HX711_port_power_down(uint8_t ch);

/**
 * @brief  Pone SCK en LOW para salir de power-down.
 * @param  ch  Canal.
 */
void HX711_port_power_up(uint8_t ch);

#ifdef __cplusplus
}
#endif

#endif /* HX711_PORT_H */
