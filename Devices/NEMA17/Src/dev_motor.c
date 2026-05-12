/**
 * @file    dev_motor.c
 * @brief   Driver para NEMA 17 vía A4988 + TIM2_CH3 PWM (PB10).
 *
 *  Cada Update Event del TIM2 = 1 paso completado.
 *  Al llegar a 0, detiene el PWM y deshabilita el motor.
 */

#include "dev_motor.h"

static volatile uint32_t s_steps_remaining = 0;
static volatile bool     s_done            = true;

/* =========================================================================== */
/*  GPIO Init — solo pines, sin timer. Llamar ANTES de osKernelStart().       */
/* =========================================================================== */

void Motor_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    /* DIR y ENABLE — salidas digitales simples */
    gpio.Pin   = MOTOR_DIR_PIN | MOTOR_EN_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* STEP — función alternativa TIM2_CH3 en PB10 */
    gpio.Pin       = MOTOR_STEP_PIN;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio.Alternate = MOTOR_STEP_AF;
    HAL_GPIO_Init(MOTOR_STEP_PORT, &gpio);

    /* Estado seguro inicial */
    HAL_GPIO_WritePin(MOTOR_EN_PORT,  MOTOR_EN_PIN,  GPIO_PIN_SET);   // deshabilitado
    HAL_GPIO_WritePin(MOTOR_DIR_PORT, MOTOR_DIR_PIN, GPIO_PIN_RESET); // CW
}

/* =========================================================================== */
/*  Timer Init — llamar DESPUÉS de osKernelInitialize().                      */
/* =========================================================================== */

Motor_Status Motor_InitOS(void)
{
    /* Motor_port_init() configura TIM2 (los GPIO ya los hizo Motor_GPIO_Init) */
    /* Llamamos solo la parte del timer, no el GPIO de nuevo */
    __HAL_RCC_TIM2_CLK_ENABLE();

    TIM_HandleTypeDef *htim = Motor_port_get_tim();

    htim->Instance               = MOTOR_TIM;
    htim->Init.Prescaler         = MOTOR_TIM_PSC;
    htim->Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim->Init.Period            = (MOTOR_TIM_CLK_EFF_HZ / MOTOR_DEFAULT_SPEED_SPS) - 1U;
    htim->Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_PWM_Init(htim) != HAL_OK) return MOTOR_ERROR;

    TIM_OC_InitTypeDef oc = {0};
    oc.OCMode     = TIM_OCMODE_PWM1;
    oc.Pulse      = (htim->Init.Period + 1U) / 2U;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(htim, &oc, MOTOR_TIM_CHANNEL) != HAL_OK)
        return MOTOR_ERROR;

    HAL_NVIC_SetPriority(MOTOR_TIM_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(MOTOR_TIM_IRQn);

    return MOTOR_OK;
}

/* =========================================================================== */
/*  API                                                                        */
/* =========================================================================== */

void Motor_Dispense(uint32_t steps, Motor_Dir dir)
{
    if (steps == 0 || !s_done) return;

    Motor_port_set_dir(dir == MOTOR_DIR_CW
                       ? MOTOR_DIR_CW_LEVEL
                       : MOTOR_DIR_CCW_LEVEL);
    Motor_port_enable();

    s_steps_remaining = steps;
    s_done            = false;

    Motor_port_start();
}

void Motor_Stop(void)
{
    Motor_port_stop();
    Motor_port_disable();
    s_steps_remaining = 0;
    s_done            = true;
}

bool Motor_IsDone(void) { return s_done; }

void Motor_SetSpeed(uint32_t sps) { Motor_port_set_speed(sps); }

/* =========================================================================== */
/*  IRQ                                                                        */
/* =========================================================================== */

void Motor_TIM2_IRQHandler(void)
{
    TIM_HandleTypeDef *htim = Motor_port_get_tim();

    if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_UPDATE) &&
        __HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_UPDATE))
    {
        __HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);

        if (s_steps_remaining > 0) {
            s_steps_remaining--;
            if (s_steps_remaining == 0) {
                Motor_port_stop();
                Motor_port_disable();
                s_done = true;
            }
        }
    }
}