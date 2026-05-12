/**
 * @file    motor_registers.h
 * @brief   Pines y constantes para A4988 + NEMA 17.
 */

#ifndef MOTOR_REGISTERS_H
#define MOTOR_REGISTERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* -------------------------------------------------------------------------- */
/* Pines — sin conflicto con LD2 (PA5) ni MOTOR_GPIO viejo (PB0)             */
/* -------------------------------------------------------------------------- */

/** @brief PB10 → TIM2_CH3 → A4988 STEP */
#define MOTOR_STEP_PORT         GPIOB
#define MOTOR_STEP_PIN          GPIO_PIN_10
#define MOTOR_STEP_AF           GPIO_AF1_TIM2
#define MOTOR_TIM_CHANNEL       TIM_CHANNEL_3

/** @brief PB4 → A4988 DIR */
#define MOTOR_DIR_PORT          GPIOA
#define MOTOR_DIR_PIN           GPIO_PIN_6

/** @brief PB5 → A4988 ENABLE (LOW = habilitado) */
#define MOTOR_EN_PORT           GPIOA
#define MOTOR_EN_PIN            GPIO_PIN_7

/* -------------------------------------------------------------------------- */
/* Timer                                                                       */
/* -------------------------------------------------------------------------- */
#define MOTOR_TIM               TIM2
#define MOTOR_TIM_IRQn          TIM2_IRQn
#define MOTOR_TIM_CLK_EFF_HZ    1000000U
#define MOTOR_TIM_PSC           83U

/* -------------------------------------------------------------------------- */
/* Mecánica                                                                    */
/* -------------------------------------------------------------------------- */
#define MOTOR_STEPS_PER_REV     200U
#define MOTOR_DEFAULT_SPEED_SPS 400U
#define MOTOR_DEFAULT_STEPS     400U
#define MOTOR_SPEED_MIN_SPS     50U
#define MOTOR_SPEED_MAX_SPS     2000U

#define MOTOR_DIR_CW_LEVEL      GPIO_PIN_RESET
#define MOTOR_DIR_CCW_LEVEL     GPIO_PIN_SET

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_REGISTERS_H */