/**
 * @file    motor_port.c
 * @brief   Primitivas de bajo nivel para A4988 vía TIM2_CH3 (PB10) + GPIO.
 */

#include "motor_port.h"

static TIM_HandleTypeDef s_htim2;

HAL_StatusTypeDef Motor_port_init(void)
{
    /* Este función no se usa directamente — ver Motor_GPIO_Init y Motor_InitOS */
    return HAL_OK;
}

void Motor_port_start(void)
{
    __HAL_TIM_CLEAR_FLAG(&s_htim2, TIM_FLAG_UPDATE);
    __HAL_TIM_SET_COUNTER(&s_htim2, 0);
    __HAL_TIM_ENABLE_IT(&s_htim2, TIM_IT_UPDATE);
    HAL_TIM_PWM_Start(&s_htim2, MOTOR_TIM_CHANNEL);
}

void Motor_port_stop(void)
{
    HAL_TIM_PWM_Stop(&s_htim2, MOTOR_TIM_CHANNEL);
    __HAL_TIM_DISABLE_IT(&s_htim2, TIM_IT_UPDATE);
}

void Motor_port_set_speed(uint32_t sps)
{
    if (sps < MOTOR_SPEED_MIN_SPS) sps = MOTOR_SPEED_MIN_SPS;
    if (sps > MOTOR_SPEED_MAX_SPS) sps = MOTOR_SPEED_MAX_SPS;
    uint32_t arr = (MOTOR_TIM_CLK_EFF_HZ / sps) - 1U;
    s_htim2.Instance->ARR  = arr;
    s_htim2.Instance->CCR3 = (arr + 1U) / 2U;
    s_htim2.Instance->EGR  = TIM_EGR_UG;
}

void Motor_port_set_dir(GPIO_PinState level)
{
    HAL_GPIO_WritePin(MOTOR_DIR_PORT, MOTOR_DIR_PIN, level);
}

void Motor_port_enable(void)
{
    HAL_GPIO_WritePin(MOTOR_EN_PORT, MOTOR_EN_PIN, GPIO_PIN_RESET);
}

void Motor_port_disable(void)
{
    HAL_GPIO_WritePin(MOTOR_EN_PORT, MOTOR_EN_PIN, GPIO_PIN_SET);
}

TIM_HandleTypeDef *Motor_port_get_tim(void)
{
    return &s_htim2;
}