/**
 * @file    dev_motor.h
 * @brief   Driver para NEMA 17 vía A4988 + TIM2_CH3 PWM (PB10).
 *
 *  Conexiones:
 *    PB10 → A4988 STEP  (TIM2_CH3 PWM)
 *    PA6  → A4988 DIR   (GPIO output)
 *    PA7  → A4988 ENABLE (LOW = habilitado)
 *    MS1/MS2/MS3 → GND  (paso completo, 200 pasos/rev)
 *    SLEEP/RESET → VCC
 *
 *  Init en main():
 *    devices_Init()  → Motor_GPIO_Init()   ← solo GPIO, antes de osKernelStart
 *    osKernelInit()  → Motor_InitOS()      ← crea TIM2, después de osKernelInit
 */

#ifndef DEV_MOTOR_H
#define DEV_MOTOR_H

#include <stdint.h>
#include <stdbool.h>
#include "motor_port.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { MOTOR_OK = 0, MOTOR_ERROR = 1 } Motor_Status;
typedef enum { MOTOR_DIR_CW = 0, MOTOR_DIR_CCW = 1 } Motor_Dir;

/** @brief Inicializa GPIO solamente. Llamar en devices_Init(), antes de osKernelStart(). */
void         Motor_GPIO_Init(void);

/** @brief Inicializa TIM2 y crea recursos. Llamar después de osKernelInitialize(). */
Motor_Status Motor_InitOS(void);

/** @brief Gira steps pasos en dirección dir. No bloqueante. */
void         Motor_Dispense(uint32_t steps, Motor_Dir dir);

/** @brief Detiene el motor inmediatamente. */
void         Motor_Stop(void);

/** @brief true cuando terminó de dar los pasos pedidos. */
bool         Motor_IsDone(void);

/** @brief Ajusta velocidad en pasos/segundo. */
void         Motor_SetSpeed(uint32_t sps);

/** @brief Llamar desde TIM2_IRQHandler en stm32f4xx_it.c. */
void         Motor_TIM2_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* DEV_MOTOR_H */