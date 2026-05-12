/**
 * @file    motor_port.h
 * @brief   Primitivas de bajo nivel para A4988 vía TIM2_CH3 (PB10) + GPIO.
 */

#ifndef MOTOR_PORT_H
#define MOTOR_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include "motor_registers.h"

HAL_StatusTypeDef      Motor_port_init(void);
void                   Motor_port_start(void);
void                   Motor_port_stop(void);
void                   Motor_port_set_speed(uint32_t sps);
void                   Motor_port_set_dir(GPIO_PinState level);
void                   Motor_port_enable(void);
void                   Motor_port_disable(void);
TIM_HandleTypeDef     *Motor_port_get_tim(void);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_PORT_H */