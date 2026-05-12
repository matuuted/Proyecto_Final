/**
 * @file    hx711_registers.h
 * @brief   Definiciones de pines, canales y constantes para HX711.
 */

#ifndef HX711_REGISTERS_H
#define HX711_REGISTERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/* Identificadores de canal                                                    */
/* -------------------------------------------------------------------------- */
#define HX711_CH_TANK   0U  /**< Galga tanque  (3/5 kg) → PC0/PC1. */
#define HX711_CH_PLATE  1U  /**< Galga plato   (1 kg)   → PC2/PC3. */
#define HX711_CH_COUNT  2U

/* -------------------------------------------------------------------------- */
/* Pines GPIO                                                                  */
/* -------------------------------------------------------------------------- */

/* Tanque */
#define HX711_TANK_DOUT_PORT    GPIOC
#define HX711_TANK_DOUT_PIN     GPIO_PIN_0
#define HX711_TANK_SCK_PORT     GPIOC
#define HX711_TANK_SCK_PIN      GPIO_PIN_1

/* Plato */
#define HX711_PLATE_DOUT_PORT   GPIOC
#define HX711_PLATE_DOUT_PIN    GPIO_PIN_2
#define HX711_PLATE_SCK_PORT    GPIOC
#define HX711_PLATE_SCK_PIN     GPIO_PIN_3

/* -------------------------------------------------------------------------- */
/* Protocolo HX711                                                             */
/* -------------------------------------------------------------------------- */

/** @brief Pulsos SCK para Canal A Gain 128 (estándar). */
#define HX711_PULSES_CHA_128    25U

/** @brief Timeout esperando DOUT = LOW (ms). */
#define HX711_READY_TIMEOUT_MS  500U

/** @brief Muestras promediadas por lectura (reduce ruido). */
#define HX711_AVG_SAMPLES       8U

/* -------------------------------------------------------------------------- */
/* Calibración — ajustar con pesa conocida                                    */
/* -------------------------------------------------------------------------- */

/** @brief Factor crudo/gramo para celda de tanque (3/5 kg). */
#define HX711_SCALE_TANK_DEFAULT    420.0f

/** @brief Factor crudo/gramo para celda de plato (1 kg). */
#define HX711_SCALE_PLATE_DEFAULT   410.0f

#ifdef __cplusplus
}
#endif

#endif /* HX711_REGISTERS_H */
